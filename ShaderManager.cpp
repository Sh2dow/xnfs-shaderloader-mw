#include "Hooks.h"
#include "ShaderManager.h"
#include <algorithm>
#include <atomic>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include "includes/injector/injector.hpp"
#include <d3dx9effect.h>
#include <iostream>
#include <MinHook.h>
#include <mutex>
#include <unordered_set>

#define SHADER_TABLE_ADDRESS 0x0093DE78;
#define WORLDSHADER_TABLE_ADDRESS 0x008F9B60;
const int ShaderTableSize = 64;

LPDIRECT3DDEVICE9 g_Device = nullptr;
ShaderManager::PresentFn ShaderManager::RealPresent = nullptr;
bool g_EnableShaderTableDump = false;

ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr;  // ✅ definition
void* g_ApplyGraphicsSettingsThis = nullptr;

struct QueuedPatch
{
    int slot;
    ID3DXEffect* fx;
    int framesRemaining = 2; // safe delay
};

std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::vector<QueuedPatch> g_PendingPatches;
std::mutex g_PatchMutex;
std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_set<std::string> g_FxOverrides;
static std::atomic<int> g_HookCallCount{0};
std::atomic<bool> g_TriggerApplyGraphicsSettings = false;


std::atomic<bool> g_PausePresent{false};
std::atomic<bool> g_PresentIsWaiting{false};

// -------------------- HELPERS --------------------

// Converts string to uppercase
std::string ToUpper(const std::string& str)
{
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i)
        result[i] = (char)toupper(result[i]);
    return result;
}

// -------------------- INCLUDE HANDLER --------------------

class FXIncludeHandler : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE, LPCSTR fileName, LPCVOID, LPCVOID* ppData, UINT* pBytes) override
    {
        std::string fullPath = "fx/" + std::string(fileName);
        printf_s("[Include] Trying to open: %s\n", fullPath.c_str());

        FILE* f = fopen(fullPath.c_str(), "rb");
        if (!f)
        {
            printf_s("[Include] Failed to open: %s\n", fullPath.c_str());
            return E_FAIL;
        }

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);
        BYTE* buf = new BYTE[len];
        fread(buf, 1, len, f);
        fclose(f);

        *ppData = buf;
        *pBytes = (UINT)len;
        printf_s("[Include] Opened: %s\n", fullPath.c_str());
        return S_OK;
    }

    STDMETHOD(Close)(LPCVOID pData) override
    {
        delete[] (BYTE*)pData;
        return S_OK;
    }
};

bool CompileAndDumpShader(const std::string& key, const std::string& fxPath)
{
    printf_s("[Compile] Compiling FX effect: %s => %s\n", fxPath.c_str(), key.c_str());

    ID3DXEffectCompiler* pCompiler = nullptr;
    ID3DXBuffer* pErrors = nullptr;

    HRESULT hr = D3DXCreateEffectCompilerFromFile(
        fxPath.c_str(),
        nullptr, // macros
        nullptr, // include
        0, // flags
        &pCompiler,
        &pErrors
    );

    if (FAILED(hr))
    {
        if (pErrors)
        {
            printf_s("[Compile] Error: %s\n", (char*)pErrors->GetBufferPointer());
            pErrors->Release();
        }
        return false;
    }

    ID3DXBuffer* pCompiledEffect = nullptr;
    hr = pCompiler->CompileEffect(0, &pCompiledEffect, &pErrors);
    pCompiler->Release();

    if (FAILED(hr))
    {
        if (pErrors)
        {
            printf_s("[Compile] Error: %s\n", (char*)pErrors->GetBufferPointer());
            pErrors->Release();
        }
        return false;
    }

    // Write to game root folder IDI_*.FX
    std::string outPath = key;
    FILE* f = fopen(outPath.c_str(), "wb");
    if (!f)
    {
        printf_s("[Compile] Failed to open output: %s\n", outPath.c_str());
        pCompiledEffect->Release();
        return false;
    }

    fwrite(pCompiledEffect->GetBufferPointer(), 1, pCompiledEffect->GetBufferSize(), f);
    fclose(f);
    pCompiledEffect->Release();

    printf_s("[Compile] Written compiled FX: %s\n", outPath.c_str());
    return true;
}

// -------------------- SHADER OVERRIDE LOADER --------------------
void LoadShaderOverrides()
{
    DWORD attrs = GetFileAttributesA("fx");
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        printf_s("[Error] fx/ folder does not exist or is inaccessible.\n");
        return;
    }

    printf_s("[Init] fx/ folder found, scanning for shaders...\n");

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("fx\\*.fx", &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf_s("[Init] No .fx files found in fx/ folder.\n");
        return;
    }

    do
    {
        std::string fileName = findData.cFileName;
        std::string name = fileName.substr(0, fileName.find_last_of('.'));
        std::replace(name.begin(), name.end(), '-', '_'); // normalize dashes
        std::string key = "IDI_" + ToUpper(name) + "_FX";
        std::string fullPath = "fx/" + fileName;
        g_FxOverrides.insert(key); // ✅ REQUIRED for HookedCreateFromResource

        CompileAndDumpShader(key, fullPath);

        printf_s("[Init] Compiling and caching %s as %s\n", fileName.c_str(), key.c_str());

        FILE* f = fopen(fullPath.c_str(), "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> data(len);
        fread(data.data(), 1, len, f);
        fclose(f);

        ID3DXEffect* fx = nullptr;
        ID3DXBuffer* errors = nullptr;
        FXIncludeHandler includeHandler;

        HRESULT hr = D3DXCreateEffect(g_Device, data.data(), (UINT)len, nullptr, &includeHandler,
                                      D3DXSHADER_DEBUG, nullptr, &fx, &errors);

        if (FAILED(hr))
        {
            printf_s("[Init] Failed to compile %s\n", fileName.c_str());
            if (errors)
            {
                printf_s("[Init] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        g_ShaderOverridePaths[key] = fullPath;
        g_ShaderBuffers[key] = data; // stores raw HLSL code
    }
    while (FindNextFileA(hFind, &findData));

    printf_s("[Debug] g_FxOverrides contains:\n");
    for (const auto& k : g_FxOverrides)
        printf_s("  - %s\n", k.c_str());

    FindClose(hFind);
}

// -------------------- HOOK HANDLER --------------------
bool IsValidThis(void* ptr)
{
    if (!ptr)
        return false;

    __try {
        void* vtable = *(void**)ptr;
        if (!vtable)
            return false;

        void* fn = *((void**)vtable); // vtable[0]
        return fn != nullptr && !IsBadCodePtr((FARPROC)fn);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx) return false;

    void** vtable = *(void***)fx;

    MEMORY_BASIC_INFORMATION mbi = {};
    if (!vtable || VirtualQuery(vtable, &mbi, sizeof(mbi)) == 0)
        return false;

    // Must be readable and executable (code segment)
    bool isReadable = (mbi.Protect & PAGE_EXECUTE_READ) ||
        (mbi.Protect & PAGE_EXECUTE_READWRITE) ||
        (mbi.Protect & PAGE_EXECUTE_WRITECOPY);
    if (!isReadable)
        return false;

    return !IsBadCodePtr((FARPROC)vtable[0]);
}

void DumpShaderTable()
{
    static ID3DXEffect** table = (ID3DXEffect**)SHADER_TABLE_ADDRESS; // NFS MW shader table

    printf_s("------ Shader Table Dump ------\n");
    for (int i = 0; i < ShaderTableSize; ++i)
    {
        ID3DXEffect* fx = table[i];
        if (!fx || !IsValidShaderPointer(fx))
            printf_s("[Check] Slot %02d = INVALID (%p)\n", i, fx);
        else
            printf_s("[Check] Slot %02d = VALID   (%p)\n", i, fx);
    }
}

void ApplyQueuedShaderPatches()
{
    std::lock_guard<std::mutex> lock(g_PatchMutex);
    static ID3DXEffect** table = (ID3DXEffect**)SHADER_TABLE_ADDRESS;

    std::vector<QueuedPatch> stillPending;

    for (auto& patch : g_PendingPatches)
    {
        if (--patch.framesRemaining > 0)
        {
            stillPending.push_back(patch);
            continue;
        }

        if (!IsValidShaderPointer(patch.fx))
        {
            printf_s("[Patch] ❌ fx invalid for slot %d, fx=%p — skipping\n", patch.slot, patch.fx);

            if ((uintptr_t)patch.fx == 0xCCCCCCCC)
                printf_s("[Patch] ❌ fx == 0xCCCCCCCC — uninitialized stack pointer\n");

            continue;
        }

        if (table[patch.slot] && IsValidShaderPointer(table[patch.slot]))
        {
            printf_s("[Patch] 🔁 Releasing old fx in slot %d (%p)\n", patch.slot, table[patch.slot]);
            table[patch.slot]->Release();
        }

        table[patch.slot] = patch.fx;

        printf_s("[Patch] ✅ Wrote shader to slot %d (fx=%p)\n", patch.slot, patch.fx);
    }

    g_PendingPatches = std::move(stillPending);
}

HRESULT WINAPI HookedCreateFromResource(
    LPDIRECT3DDEVICE9 device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors)
{
    if (!g_Device)
    {
        g_Device = device;
        LoadShaderOverrides();

        // Hook Present now that g_Device is valid
        void** vtable = *(void***)g_Device;
        if (vtable)
        {
            ShaderManager::RealPresent = (PresentFn)vtable[17];
            DWORD oldProtect;
            VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            vtable[17] = (void*)&HookedPresent;
            VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
            printf_s("[Init] Hooked IDirect3DDevice9::Present\n");
        }
    }

    printf_s("[Hook] D3DXCreateEffectFromResourceA called #%d — pResource = %s\n", g_HookCallCount++,
             pResource ? pResource : "(null)");

    if (g_FxOverrides.count(pResource))
    {
        std::string path = std::string(pResource); // assume key starts with IDI_
        FILE* f = fopen(path.c_str(), "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<char> buffer(len);
            fread(buffer.data(), 1, len, f);
            fclose(f);

            FXIncludeHandler includeHandler;
            HRESULT hr = D3DXCreateEffect(device, buffer.data(), (UINT)len, defines, &includeHandler, flags, pool,
                                          outEffect, outErrors);
            if (SUCCEEDED(hr) && *outEffect)
            {
                printf_s("[Hook] Loaded compiled FX override for %s\n", pResource);
                return S_OK;
            }

            printf_s("[Hook] Failed to create effect from compiled override for %s\n", pResource);
        }
        else
        {
            printf_s("[Hook] Failed to open compiled file: %s\n", path.c_str());
        }
    }

    // Fallback: load compiled shader from game root if it exists and wasn't compiled from .fx
    if (!g_FxOverrides.count(pResource) && strncmp(pResource, "IDI_", 4) == 0)
    {
        FILE* f = fopen(pResource, "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<char> buffer(len);
            fread(buffer.data(), 1, len, f);
            fclose(f);

            FXIncludeHandler includeHandler;
            HRESULT hr = D3DXCreateEffect(device, buffer.data(), (UINT)len,
                                          defines, &includeHandler, flags, pool, outEffect, outErrors);
            if (SUCCEEDED(hr) && *outEffect)
            {
                printf_s("[Hook] Loaded fallback compiled shader: %s\n", pResource);
                return S_OK;
            }

            printf_s("[Hook] Failed to load fallback compiled shader: %s\n", pResource);
        }
    }

    return RealCreateFromResource(device, hModule, pResource,
                                  defines, include, flags, pool, outEffect, outErrors);
}

bool SafePatchShaderTable(int slot, ID3DXEffect* fx)
{
    if (slot < 0 || slot >= ShaderTableSize)
    {
        printf_s("[Patch] ❌ Invalid slot index: %d\n", slot);
        return false;
    }

    if (!fx || !IsValidShaderPointer(fx))
    {
        printf_s("[Patch] ❌ fx is null or invalid for slot %d\n", slot);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(g_PatchMutex);
        fx->AddRef(); // Safe defer
        g_PendingPatches.push_back({slot, fx, 2});
    }

    printf_s("[Patch] ⏳ Deferred shader patch queued: slot %d → fx=%p\n", slot, fx);
    return true;
}

void PauseGameThread()
{
    g_PausePresent = true;

    // Wait up to 250ms for Present to acknowledge pause
    const auto start = std::chrono::steady_clock::now();
    while (!g_PresentIsWaiting &&
        std::chrono::steady_clock::now() - start < std::chrono::milliseconds(250))
    {
        Sleep(1);
    }

    if (g_PresentIsWaiting)
        printf_s("[Pause] Game Present thread paused.\n");
    else
        printf_s("[Pause] ⚠️ Timeout waiting for Present to stall — continuing anyway.\n");
}

void ResumeGameThread()
{
    g_PausePresent = false;
    printf_s("[Pause] Game Present thread resumed.\n");
}

// MUST NOT use STL types in this function!
static const char* SafeGetShaderTableEntry(int index)
{
    const char** table = (const char**)WORLDSHADER_TABLE_ADDRESS;
    const char* result = nullptr;
    __try
    {
        result = table[index];
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return nullptr;
    }
    return result;
}

std::vector<int> LookupShaderSlotsFromResource(const std::string& resourceName)
{
    std::vector<int> results;
    std::string base;
    if (resourceName.find("IDI_") == 0 && resourceName.rfind("_FX") == resourceName.length() - 3)
        base = resourceName.substr(4, resourceName.length() - 7); // Strip IDI_ and _FX
    else
        return results;

    for (int i = 0; i < ShaderTableSize; ++i)
    {
        const char* entry = SafeGetShaderTableEntry(i);
        if (!entry || IsBadStringPtrA(entry, 128))
            continue;

        std::string name(entry);
        size_t dot = name.find('.');
        if (dot != std::string::npos)
            name = name.substr(0, dot);

        std::string a = base, b = name;
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);

        if (a == b)
            results.push_back(i);
    }

    return results;
}

void ForceReplaceShaderIntoSlots(const std::string& resourceKey, ID3DXEffect* fx)
{
    if (!fx || !IsValidShaderPointer(fx))
    {
        printf_s("[HotReload] ❌ Invalid fx pointer passed to ForceReplaceShaderIntoSlots for %s\n",
                 resourceKey.c_str());
        return;
    }

    PauseGameThread();

    auto slots = LookupShaderSlotsFromResource(resourceKey);
    if (!slots.empty())
    {
        for (int slot : slots)
        {
            if (!SafePatchShaderTable(slot, fx))
            {
                printf_s("[HotReload] ❌ Failed to patch slot %d for %s\n", slot, resourceKey.c_str());
                continue;
            }

            printf_s("[HotReload] ✅ Patched slot %d with new effect for %s\n", slot, resourceKey.c_str());
        }
    }
    else
    {
        printf_s("[HotReload] ⚠️ No slots found for %s\n", resourceKey.c_str());
    }

    ResumeGameThread();
}

void RecompileAndReloadAll()
{
    if (!g_Device)
    {
        printf_s("[HotReload] ❌ g_Device is null, cannot recompile shaders.\n");
        return;
    }

    bool patchedAny = false;

    for (const auto& key : g_FxOverrides)
    {
        auto it = g_ShaderOverridePaths.find(key);
        if (it == g_ShaderOverridePaths.end())
        {
            printf_s("[HotReload] ❌ No path found for %s\n", key.c_str());
            continue;
        }

        const std::string& fxPath = it->second;
        if (!CompileAndDumpShader(key, fxPath))
        {
            printf_s("[HotReload] ❌ Failed to recompile %s\n", fxPath.c_str());
            continue;
        }

        FILE* f = fopen(key.c_str(), "rb");
        if (!f)
        {
            printf_s("[HotReload] ❌ Failed to reopen compiled FX: %s\n", key.c_str());
            continue;
        }

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> buffer(len);
        fread(buffer.data(), 1, len, f);
        fclose(f);

        FXIncludeHandler includeHandler;
        LPD3DXEFFECT fx = nullptr;
        LPD3DXBUFFER errors = nullptr;

        HRESULT hr = D3DXCreateEffect(
            g_Device, buffer.data(), (UINT)len,
            nullptr, &includeHandler,
            D3DXSHADER_DEBUG, nullptr, &fx, &errors);

        if (FAILED(hr) || !fx || !IsValidShaderPointer(fx))
        {
            printf_s("[HotReload] ❌ Invalid fx pointer after compile: %p (hr=0x%08X)\n", fx, hr);
            if (errors)
            {
                printf_s("[HotReload] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        printf_s("[HotReload] ✅ Reloaded shader: %s (fx=%p)\n", key.c_str(), fx);

        if (!IsValidShaderPointer(fx))
        {
            printf_s("[HotReload] ❌ fx for %s is invalid, skipping patch\n", key.c_str());
            fx->Release();
            continue;
        }

        ForceReplaceShaderIntoSlots(key, fx);
        printf_s("[HotReload] ✅ Force-replaced effect in all slots for %s\n", key.c_str());

        patchedAny = true;
    }

    if (patchedAny)
    {
        printf_s("[HotReload] Triggering ApplyGraphicsSettings()\n");
        g_TriggerApplyGraphicsSettings = true;
        printf_s("[HotReload] Will apply graphics settings at next Present\n");
    }
    else
    {
        printf_s("[HotReload] ❌ No shader slots were patched — skipping ApplyGraphicsSettings()\n");
    }
}

// Logging hook: store the 'this' pointer used in ApplyGraphicsSettings
void __stdcall LogApplyGraphicsSettingsThis(void* thisptr)
{
    printf_s("[Hook] ApplyGraphicsSettings called with this = %p\n", thisptr);
    if (thisptr)
        g_ApplyGraphicsSettingsThis = thisptr;
}

void __fastcall HookApplyGraphicsSettings(void* manager, void* /*unused*/, void* vtObject)
{
    g_ApplyGraphicsSettingsThis = vtObject;
    printf_s("[Hook] ApplyGraphicsSettings called, this = %p, manager = %p\n", vtObject, manager);

    if (ApplyGraphicsSettingsOriginal)
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
}

void TryApplyGraphicsSettings()
{
    if (!g_ApplyGraphicsSettingsThis)
    {
        printf_s("[HotReload] ❌ this pointer is null — ApplyGraphicsSettings skipped\n");
        return;
    }

    printf_s("[HotReload] ✅ Calling ApplyGraphicsSettings(this = %p)\n", g_ApplyGraphicsSettingsThis);
    ApplyGraphicsSettingsOriginal(nullptr, nullptr, g_ApplyGraphicsSettingsThis);
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                             const RECT* src, const RECT* dest,
                             HWND hwnd, const RGNDATA* dirty)
{
    if (g_EnableShaderTableDump)
        DumpShaderTable(); // before anything

    TryApplyGraphicsSettings();

    ApplyQueuedShaderPatches(); // Safe after ApplyGraphicsSettings
    return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
}