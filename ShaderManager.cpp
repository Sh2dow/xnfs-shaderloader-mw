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
void** g_pVisualTreatment = (void**)0x00982AF0;
static ID3DXEffect** g_ShaderTable = (ID3DXEffect**)SHADER_TABLE_ADDRESS; // NFS MW shader g_ShaderTable

LPDIRECT3DDEVICE9 g_Device = nullptr;
ShaderManager::PresentFn ShaderManager::RealPresent = nullptr;

bool g_ApplyGraphicsSeenThisFrame = false;
int g_ApplyGraphicsTriggerDelay = 0;

bool g_EnableShaderTableDump = false;
void* g_LiveVisualTreatmentObject = nullptr;

ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr; // ✅ definition
ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal = nullptr; // ✅ definition
IVisualTreatment_ResetFn IVisualTreatment_Reset = (IVisualTreatment_ResetFn)0x0073DE50;
void* g_ApplyGraphicsManagerThis = nullptr;
void* g_ApplyGraphicsSettingsThis = nullptr;
ID3DXEffect* g_LastReloadedFx = nullptr;

struct QueuedPatch
{
    int slot;
    ID3DXEffect* fx;
    int framesRemaining = 2; // safe delay
};

std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::vector<QueuedPatch> g_PendingPatches;
ID3DXEffect* g_SlotRetainedFx[64] = {};

std::mutex g_PatchMutex;
std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_set<std::string> g_FxOverrides;
static std::atomic<int> g_HookCallCount{0};
std::atomic<bool> g_TriggerApplyGraphicsSettings = false;

void* g_ThisCandidates[3] = {};
int g_ThisCount = 0;

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

    __try
    {
        void* vtable = *(void**)ptr;
        if (!vtable)
            return false;

        void* fn = *((void**)vtable); // vtable[0]
        return fn != nullptr && !IsBadCodePtr((FARPROC)fn);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsValidShaderPointer_SEH(ID3DXEffect* fx, void*** outVtable, DWORD* outProtect)
{
    __try
    {
        if (IsBadReadPtr(fx, sizeof(void*)))
            return false;

        void** vtable = *(void***)fx;
        if (!vtable || IsBadReadPtr(vtable, sizeof(void*)))
            return false;

        MEMORY_BASIC_INFORMATION mbi = {};
        if (!VirtualQuery(vtable, &mbi, sizeof(mbi)))
            return false;

        *outVtable = vtable;
        *outProtect = mbi.Protect;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx || (uintptr_t)fx < 0x10000)
        return false;

    if (((uintptr_t)fx & 0xFFF00000) == 0x3F000000)
        return false;

    void** vtable = nullptr;
    DWORD prot = 0;
    if (!IsValidShaderPointer_SEH(fx, &vtable, &prot))
        return false;

    bool validVtable =
        (prot & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) &&
        !IsBadCodePtr((FARPROC)vtable[0]);

    static std::unordered_map<uintptr_t, int> invalidVtableHits;
    if (!validVtable)
    {
        uintptr_t vtblAddr = reinterpret_cast<uintptr_t>(vtable);
        int& count = invalidVtableHits[vtblAddr];
        if (count++ < 1)
            printf_s("⚠️ Invalid vtable: fx=%p vtable=%p prot=0x%08X\n", fx, vtable, prot);

        // if (++count == 1 || count % 10 == 0) {
        //     printf_s("⚠️ Invalid vtable: fx=%p vtable=%p prot=0x%08X (seen %d times)\n", fx, vtable, prot, count);
        // }
    }

    return validVtable;
}

void DumpShaderTable()
{
    printf_s("------ Shader Table Dump ------\n");
    for (int i = 0; i < ShaderTableSize; ++i)
    {
        ID3DXEffect* fx = g_ShaderTable[i];
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

        // Clone the shader to avoid shared use-after-free
        ID3DXEffect* clone = nullptr;
        HRESULT hr = patch.fx->CloneEffect(g_Device, &clone);

        if (FAILED(hr) || !clone || !IsValidShaderPointer(clone))
        {
            printf_s("[Patch] ❌ CloneEffect failed for fx=%p, slot=%d (hr=0x%08X)\n", patch.fx, patch.slot,
                     (unsigned)hr);
            continue;
        }

        // Defensive: track refcount of original (optional diagnostic)
        ULONG refCount = patch.fx->AddRef();
        patch.fx->Release();

        // Release old shader in this slot
        if (table[patch.slot])
        {
            if (IsValidShaderPointer(table[patch.slot]))
            {
                printf_s("[Patch] 🔁 Releasing old fx in slot %d (%p)\n", patch.slot, table[patch.slot]);
                table[patch.slot]->Release();
            }
            else
            {
                printf_s("[Patch] ⚠️ Existing fx in slot %d (%p) is invalid — replacing anyway\n", patch.slot,
                         table[patch.slot]);
            }
        }

        // Replace g_ShaderTable[slot] with new clone
        table[patch.slot] = clone;

        // Release any previous retained clone first
        if (g_SlotRetainedFx[patch.slot])
        {
            g_SlotRetainedFx[patch.slot]->Release();
            g_SlotRetainedFx[patch.slot] = nullptr;
        }

        // Assign to g_ShaderTable
        clone->AddRef(); // ref for slot g_ShaderTable
        table[patch.slot] = clone;

        // Retain permanently
        clone->AddRef(); // ref for retention
        g_SlotRetainedFx[patch.slot] = clone;

        printf_s("[Patch] ✅ Cloned and wrote fx to slot %d (clone=%p, from=%p, refcount~%lu)\n",
                 patch.slot, clone, patch.fx, refCount);
    }

    g_PendingPatches = std::move(stillPending);
}

void ReleaseAllRetainedShaders()
{
    for (int i = 0; i < 64; ++i)
    {
        if (g_SlotRetainedFx[i])
        {
            g_SlotRetainedFx[i]->Release();
            g_SlotRetainedFx[i] = nullptr;
        }
    }
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

bool IsSafeReadableString(const char* ptr)
{
    if ((uintptr_t)ptr < 0x10000) return false;

    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(ptr, &mbi, sizeof(mbi))) return false;

    return (mbi.Protect & (
        PAGE_READONLY |
        PAGE_READWRITE |
        PAGE_EXECUTE_READ |
        PAGE_EXECUTE_READWRITE)) != 0;
}

// MUST NOT use STL types in this function!
const char* SafeGetShaderTableEntry(int i)
{
    const char** shaderNameTable = (const char**)WORLDSHADER_TABLE_ADDRESS;

    if (i < 0 || i >= ShaderTableSize)
        return nullptr;

    const char* ptr = shaderNameTable[i];
    if (!IsSafeReadableString(ptr))
        return nullptr;

    return ptr;
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
        if (!entry || !IsSafeReadableString(entry))
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

const char* TryGetFxNameAnnotation(ID3DXEffect* fx)
{
    __try
    {
        // Get top-level object (global scope)
        D3DXHANDLE dummy = fx->GetParameterByName(nullptr, nullptr);
        if (!dummy)
            return nullptr;

        // Get the annotation called "Name"
        D3DXHANDLE nameHandle = fx->GetAnnotationByName(dummy, "Name");
        if (!nameHandle)
            return nullptr;

        LPCSTR nameStr = nullptr;
        if (SUCCEEDED(fx->GetString(nameHandle, &nameStr)) && nameStr && !IsBadStringPtrA(nameStr, 128))
            return nameStr;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return nullptr;
    }

    return nullptr;
}

void VisualTreatment_Reset()
{
    if (g_pVisualTreatment && IsValidThis(*g_pVisualTreatment))
    {
        void* vt = *g_pVisualTreatment;

        ID3DXEffect** fxSlot = (ID3DXEffect**)((char*)vt + 0x18C);
        if (IsValidShaderPointer(*fxSlot))
        {
            printf_s("🔧 Releasing fx at +0x18C (%p)\n", *fxSlot);
            (*fxSlot)->Release();
            *fxSlot = nullptr;
        }

        __try
        {
            IVisualTreatment_Reset(vt);
            printf_s("[HotReload] ✅ Reset() called on IVisualTreatment at %p\n", vt);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf_s("[HotReload] ❌ Exception while calling IVisualTreatment::Reset()\n");
        }
    }
    else
    {
        printf_s("[HotReload] ❌ g_pVisualTreatment invalid or null\n");
    }
}

void ForceReplaceShaderIntoSlots(const std::string& resourceKey, ID3DXEffect* fx)
{
    if (!fx || !IsValidShaderPointer(fx))
    {
        printf_s("[HotReload] ❌ Invalid fx pointer passed to ForceReplaceShaderIntoSlots for %s\n",
                 resourceKey.c_str());
        return;
    }

    // 🌐 Try to get the [Name] annotation safely
    const char* annotation = TryGetFxNameAnnotation(fx);
    if (annotation && !IsBadStringPtrA(annotation, 128))
        printf_s("[HotReload] 🔍 Shader annotation [Name] = %s\n", annotation);
    else
        printf_s("[HotReload] ⚠️ Shader [Name] annotation missing or caused exception\n");

    PauseGameThread();

    bool didPatch = false;

    // 🔍 Lookup slots by resourceKey (IDI_XXX_FX)
    auto slots = LookupShaderSlotsFromResource(resourceKey);
    if (!slots.empty())
    {
        for (int slot : slots)
        {
            printf_s("[HotReload] 🎯 Attempting patch into slot %d for %s\n", slot, resourceKey.c_str());

            if (!SafePatchShaderTable(slot, fx))
            {
                printf_s("[HotReload] ❌ Failed to patch slot %d for %s\n", slot, resourceKey.c_str());
                continue;
            }

            printf_s("[HotReload] ✅ Patched slot %d with new effect %p for %s\n", slot, fx, resourceKey.c_str());
            didPatch = true;
        }

        if (didPatch)
        {
            if (g_pVisualTreatment && *g_pVisualTreatment)
            {
                VisualTreatment_Reset();
                printf_s("[HotReload] 🔄 IVisualTreatment::Reset triggered for %s\n", resourceKey.c_str());
            }

            g_TriggerApplyGraphicsSettings = true;

            // Optional: refresh fallback candidates
            ScanIVisualTreatment();
        }

        if (!didPatch && fx && g_pVisualTreatment && *g_pVisualTreatment)
        {
            printf_s("[HotReload] 🧪 Forcing Reset on IVisualTreatment singleton\n");
            // IVisualTreatment_Reset(*g_pVisualTreatment);
            VisualTreatment_Reset();
        }
    }
    else
    {
        printf_s("[HotReload] ⚠️ No slots found for %s — relying on manual hook or fallback.\n", resourceKey.c_str());

        // Even without patches, trigger ApplyGraphicsSettings
        g_TriggerApplyGraphicsSettings = true;
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
        if (IsValidShaderPointer(fx))
        {
            fx->AddRef(); // prevent premature destruction
            g_LastReloadedFx = fx;
        }
        else
        {
            printf_s("[HotReload] ❌ fx pointer invalid, not assigning to g_LastReloadedFx\n");
        }

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

void ScanIVisualTreatment()
{
    if (!g_pVisualTreatment || IsBadReadPtr(g_pVisualTreatment, sizeof(void*)))
    {
        printf_s("❌ g_pVisualTreatment invalid or not set\n");
        return;
    }

    void* base = *g_pVisualTreatment;
    if (!base || IsBadReadPtr(base, 0x200))
    {
        printf_s("❌ g_pVisualTreatment base object is invalid\n");
        return;
    }

    uintptr_t sub = *(uintptr_t*)((char*)base + 0x18C);
    if (!sub || IsBadReadPtr((void*)sub, 0x60))
    {
        printf_s("❌ sub-object @ +0x18C is invalid\n");
        return;
    }

    printf_s("[Scan] IVisualTreatment @ %p\n", g_pVisualTreatment);
    printf_s("[SubScan @ 0x18C] object = %p\n", (void*)sub);

    if (IsValidShaderPointer((ID3DXEffect*)sub))
    {
        printf_s("✅ SubScan at +0x18C looks valid, storing as vtObject[0]\n");
        g_ThisCandidates[0] = (void*)sub; // ✅ use this in fallback
        g_ThisCount = 1;
    }
    else
    {
        printf_s("❌ SubScan result is not a valid shader-like object\n");
    }
}

void* GetIVisualTreatmentObject()
{
    if (!g_ThisCandidates[0])
        ScanIVisualTreatment();

    return g_ThisCandidates[0];
}

bool IsLikelyApplyGraphicsSettingsObject(void* candidate)
{
    if (!candidate || (uintptr_t)candidate < 0x10000 || IsBadReadPtr(candidate, 0x130))
        return false;

    // Slot +0x12C: should be a vtable pointer
    void* vtblPtr = *(void**)((char*)candidate + 0x12C);
    if (!vtblPtr || (uintptr_t)vtblPtr < 0x10000)
        return false;

    MEMORY_BASIC_INFORMATION mbi = {};
    if (VirtualQuery(vtblPtr, &mbi, sizeof(mbi)) != sizeof(mbi))
        return false;

    // VTable must be readable and executable
    if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
        return false;

    // Sanity check: slot +0x0 to +0x30 should not contain floating point constants
    for (int offset = 0x00; offset <= 0x30; offset += 4)
    {
        DWORD val = *(DWORD*)((char*)candidate + offset);
        if ((val & 0xFFF00000) == 0x3F000000) // Likely float constant (0.5–1.0 range)
            return false;
    }

    // Optional: Check if +0x30 is likely a valid ID3DXEffect*
    void* fxCandidate = *(void**)((char*)candidate + 0x30);
    if ((uintptr_t)fxCandidate > 0x10000 && IsValidShaderPointer((ID3DXEffect*)fxCandidate))
    {
        printf_s("🔍 [IsLikelyApplyGraphicsSettingsObject] Slot +0x30 is valid shader pointer: %p\n", fxCandidate);
    }
    else
    {
        printf_s("🔍 [IsLikelyApplyGraphicsSettingsObject] Slot +0x30 not used or invalid\n");
    }

    return true;
}

bool ReplaceShaderSlot(BYTE* object, int offset, ID3DXEffect* newFx)
{
    ID3DXEffect** slot = (ID3DXEffect**)(object + offset);

    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery(slot, &mbi, sizeof(mbi)) || !(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)))
    {
        printf_s("❌ Cannot access shader slot at +0x%02X (%p)\n", offset, slot);
        return false;
    }

    ID3DXEffect* oldFx = nullptr;
    __try
    {
        oldFx = *slot;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }

    if (!newFx || !IsValidShaderPointer(newFx))
    {
        printf_s("❌ Invalid newFx pointer passed to ReplaceShaderSlot\n");
        return false;
    }

    if (oldFx != newFx)
    {
        if (oldFx && IsValidShaderPointer(oldFx))
        {
            printf_s("🔁 Releasing old fx at +0x%02X = %p\n", offset, oldFx);
            oldFx->Release();
        }

        *slot = newFx;
        newFx->AddRef();
        printf_s("✅ Overwrote slot +0x%02X with fx = %p\n", offset, newFx);
    }
    else
    {
        printf_s("⚠️ Slot +0x%02X already set to fx = %p — skipping\n", offset, newFx);
    }

    return true;
}

void* ResolveApplyGraphicsThis()
{
    if (IsValidThis(g_ApplyGraphicsSettingsThis))
        return g_ApplyGraphicsSettingsThis;

    for (int i = 0; i < g_ThisCount; ++i)
        if (IsValidThis(g_ThisCandidates[i]))
            return g_ThisCandidates[i];

    void* scan = GetIVisualTreatmentObject();
    if (IsValidThis(scan) && IsLikelyApplyGraphicsSettingsObject(scan))
        return scan;

    return nullptr;
}

bool AlreadyStored(void* ptr)
{
    for (int i = 0; i < g_ThisCount; ++i)
    {
        if (g_ThisCandidates[i] == ptr)
            return true;
    }
    return false;
}

void LogLiveShaderMatch(void* targetThis)
{
    if (!targetThis || !IsValidThis(targetThis))
        return;

    BYTE* sub = (BYTE*)targetThis;

    for (int i = 0; i < 0x100; i += 4)
    {
        ID3DXEffect** fxSlot = (ID3DXEffect**)(sub + i);
        ID3DXEffect* fx = nullptr;

        __try
        {
            fx = *fxSlot;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            continue;
        }

        if (fx == g_LastReloadedFx)
        {
            printf_s("🎯 Exact match: g_LastReloadedFx found at +0x%02X (%p)\n", i, fx);
        }
        else if (IsValidShaderPointer(fx))
        {
            D3DXEFFECT_DESC d = {};
            if (SUCCEEDED(fx->GetDesc(&d)))
            {
                printf_s("🔍 Shader at +0x%02X = %p: Creator=%s Params=%d Techs=%d\n",
                         i, fx, d.Creator, d.Parameters, d.Techniques);
            }
        }
    }
}

void LogApplyGraphicsSettingsCall(void* manager, void* object, int objectType)
{
    static std::unordered_map<std::pair<void*, int>, int> objectHits;
    static std::pair<void*, int> lastKey = {nullptr, 0};
    // static int repeatCount = 0;

    std::pair<void*, int> key = std::make_pair(object, objectType);
    int& count = objectHits[key];

    // if (key == lastKey) {
    //     ++repeatCount;
    // } else {
    //     if (repeatCount > 1 && count % 10 == 0)
    //     {
    //         printf_s("  [Hook] (object %p type=%d repeated %d times)\n", key.first, key.second, repeatCount);
    //     }
    //
    //     lastKey = key;
    //     repeatCount = 1;
    // }

    if (count++ < 1)
    {
        switch (objectType)
        {
        case 1:
            printf_s("[Hook] ApplyGraphicsSettings live-call: manager=%p object=%p\n", manager, object);
            break;
        case 2:
            printf_s("[Hook] ➕ Captured live vtObject = %p\n", object);
            break;
        }
    }
}

void PrintFxAtOffsets(void* target)
{
    BYTE* base = (BYTE*)target;
    for (int i = 0; i < 0x100; i += 4)
    {
        ID3DXEffect** fxSlot = (ID3DXEffect**)(base + i);
        ID3DXEffect* fx = nullptr;

        __try
        {
            fx = *fxSlot;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            continue;
        }

        if (fx == g_LastReloadedFx)
        {
            printf_s("🎯 Match: g_LastReloadedFx is actively assigned at offset +0x%02X (%p)\n", i, fx);
        }
        else if (IsValidShaderPointer(fx))
        {
            D3DXEFFECT_DESC d = {};
            if (SUCCEEDED(fx->GetDesc(&d)))
            {
                printf_s("🔍 Found other valid shader at +0x%02X: %p (%s)\n", i, fx, d.Creator);
            }
        }
    }
}

bool TryApplyGraphicsSettingsSafely()
{
    void* targetThis = ResolveApplyGraphicsThis();
    if (!targetThis)
    {
        printf_s("❌ Failed to resolve ApplyGraphicsSettings this-pointer\n");
        return false;
    }

    if (!g_ApplyGraphicsManagerThis)
    {
        printf_s("❌ g_ApplyGraphicsManagerThis is null\n");
        return false;
    }

    PrintFxAtOffsets(targetThis); // just do this once for diagnostics

    // Replace +0x18C if reloaded
    if (g_LastReloadedFx && IsValidShaderPointer(g_LastReloadedFx))
    {
        BYTE* obj = (BYTE*)targetThis;

        D3DXEFFECT_DESC newDesc = {};
        g_LastReloadedFx->GetDesc(&newDesc);

        for (int offset = 0; offset < 0x100; offset += 4)
        {
            ID3DXEffect** fxSlot = (ID3DXEffect**)(obj + offset);
            ID3DXEffect* fx = nullptr;

            __try
            {
                fx = *fxSlot;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                continue;
            }

            if (!fx || !IsValidShaderPointer(fx))
                continue;

            if (fx == g_LastReloadedFx)
            {
                ReplaceShaderSlot(obj, offset, g_LastReloadedFx);
            }
            else
            {
                D3DXEFFECT_DESC desc = {};
                if (SUCCEEDED(fx->GetDesc(&desc)) &&
                    !strcmp(desc.Creator, newDesc.Creator) &&
                    desc.Parameters == newDesc.Parameters &&
                    desc.Techniques == newDesc.Techniques)
                {
                    printf_s("🔶 Fuzzy match at +0x%02X (%p) — replacing\n", offset, fx);
                    ReplaceShaderSlot(obj, offset, g_LastReloadedFx);
                }
            }
        }
    }

    // Call ApplyGraphicsSettings
    printf_s("🎯 Calling ApplyGraphicsSettingsOriginal(manager=%p, object=%p)\n",
             g_ApplyGraphicsManagerThis, targetThis);

    __try
    {
        LogLiveShaderMatch(targetThis);
        ApplyGraphicsSettingsOriginal(g_ApplyGraphicsManagerThis, nullptr, targetThis);
        LogLiveShaderMatch(targetThis);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf_s("❌ Exception during ApplyGraphicsSettingsOriginal call\n");
        return false;
    }

    PrintFxAtOffsets(targetThis); // again after call

    g_ThisCount = 0;
    g_TriggerApplyGraphicsSettings = false; // ✅ Prevent further log spam
    return true;
}

void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject)
{
    // printf_s("[Hook] ApplyGraphicsSettings live-call: manager=%p object=%p\n", manager, vtObject);
    LogApplyGraphicsSettingsCall(manager, vtObject, 1);

    g_ApplyGraphicsManagerThis = manager;

    if (IsValidThis(vtObject))
    {
        if (!g_ApplyGraphicsSettingsThis)
        {
            g_ApplyGraphicsSettingsThis = vtObject;
            printf_s("[Hook] ✅ g_ApplyGraphicsSettingsThis captured for first time: %p\n", vtObject);
        }

        if (g_ThisCount < 3 && !AlreadyStored(vtObject))
        {
            g_ThisCandidates[g_ThisCount++] = vtObject;
        }

        // printf_s("[Hook] ➕ Captured live vtObject = %p\n", vtObject);
        LogApplyGraphicsSettingsCall(manager, vtObject, 2);
    }

    // ✅ Continue execution
    if (ApplyGraphicsSettingsOriginal)
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);

    if (g_TriggerApplyGraphicsSettings)
    {
        printf_s("✅ ApplyGraphicsSettings hook confirmed — clearing trigger flag\n");
        g_TriggerApplyGraphicsSettings = false;
        g_ThisCount = 0;
    }
}

bool SafeReleaseShaderSlot(ID3DXEffect** fxSlot, int offset)
{
    if (IsBadReadPtr(fxSlot, sizeof(void*)))
    {
        printf_s("⚠️ fxSlot @ +0x%02X not readable\n", offset);
        return false;
    }

    ID3DXEffect* fx = *fxSlot;

    if ((uintptr_t)fx < 0x10000)
    {
        printf_s("⚠️ fx @ +0x%02X too low to be valid: %p\n", offset, fx);
        return false;
    }

    if (((uintptr_t)fx & 0xFFF00000) == 0x3F000000)
    {
        printf_s("⚠️ fx @ +0x%02X looks like float constant: 0x%08X\n", offset, (unsigned)(uintptr_t)fx);
        return false;
    }

    if (!IsValidShaderPointer(fx))
    {
        printf_s("⚠️ fx @ +0x%02X is not a valid ID3DXEffect*: %p\n", offset, fx);
        return false;
    }

    printf_s("[HotReload] 🔻 Releasing shader at +0x%02X (%p)\n", offset, fx);
    fx->Release();
    *fxSlot = nullptr;
    return true;
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                             const RECT* src, const RECT* dest,
                             HWND hwnd, const RGNDATA* dirty)
{
    if (g_EnableShaderTableDump)
        DumpShaderTable();

    static int waitFrames = 0;

    if (g_TriggerApplyGraphicsSettings)
    {
        if (g_ApplyGraphicsTriggerDelay > 0)
        {
            g_ApplyGraphicsTriggerDelay--;
            g_ApplyGraphicsSeenThisFrame = false; // Ignore anything during delay
        }
        else if (g_ApplyGraphicsSeenThisFrame)
        {
            printf_s("✅ ApplyGraphicsSettings hook confirmed — clearing trigger flag\n");
            g_TriggerApplyGraphicsSettings = false;
            g_ApplyGraphicsSeenThisFrame = false;
            g_ApplyGraphicsTriggerDelay = 0;
            waitFrames = 0;
        }
        else
        {
            if (++waitFrames >= 180)
            {
                printf_s("⚠️ Timeout waiting for ApplyGraphicsSettings — resetting\n");
                g_TriggerApplyGraphicsSettings = false;
                waitFrames = 0;
            }
            else if (waitFrames == 1 || waitFrames % 30 == 0)
            {
                printf_s("[HotReload] ⏳ Waiting for ApplyGraphicsSettings calls... (%d captured)\n", g_ThisCount);
            }
        }
    }
    else
    {
        g_ApplyGraphicsSeenThisFrame = false;
    }

    ApplyQueuedShaderPatches();

    return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
}
