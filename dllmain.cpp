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
#include <mutex>
#include <unordered_set>

// -------------------- GLOBALS --------------------

LPDIRECT3DDEVICE9 g_Device = nullptr;

std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::unordered_set<std::string> g_FxOverrides;
static std::atomic<int> g_HookCallCount{0};
std::atomic<bool> g_TriggerApplyGraphicsSettings = false;

struct QueuedPatch {
    int slot;
    ID3DXEffect* fx;
    int framesRemaining = 2; // safe delay
};

std::vector<QueuedPatch> g_PendingPatches;

std::mutex g_PatchMutex;

std::atomic<bool> g_PausePresent{false};
std::atomic<bool> g_PresentIsWaiting{false};

typedef void (__cdecl* ApplyGraphicsSettingsFn)();
ApplyGraphicsSettingsFn ApplyGraphicsSettings = (ApplyGraphicsSettingsFn)0x004EA0B0;

typedef HRESULT (WINAPI* PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);
PresentFn RealPresent = nullptr;

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*, LPD3DXINCLUDE,
    DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

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
        printf("[Include] Trying to open: %s\n", fullPath.c_str());

        FILE* f = fopen(fullPath.c_str(), "rb");
        if (!f)
        {
            printf("[Include] Failed to open: %s\n", fullPath.c_str());
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
        printf("[Include] Opened: %s\n", fullPath.c_str());
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
    printf("[Compile] Compiling FX effect: %s => %s\n", fxPath.c_str(), key.c_str());

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
            printf("[Compile] Error: %s\n", (char*)pErrors->GetBufferPointer());
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
            printf("[Compile] Error: %s\n", (char*)pErrors->GetBufferPointer());
            pErrors->Release();
        }
        return false;
    }

    // Write to game root folder IDI_*.FX
    std::string outPath = key;
    FILE* f = fopen(outPath.c_str(), "wb");
    if (!f)
    {
        printf("[Compile] Failed to open output: %s\n", outPath.c_str());
        pCompiledEffect->Release();
        return false;
    }

    fwrite(pCompiledEffect->GetBufferPointer(), 1, pCompiledEffect->GetBufferSize(), f);
    fclose(f);
    pCompiledEffect->Release();

    printf("[Compile] Written compiled FX: %s\n", outPath.c_str());
    return true;
}

// -------------------- SHADER OVERRIDE LOADER --------------------
void LoadShaderOverrides()
{
    DWORD attrs = GetFileAttributesA("fx");
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        printf("[Error] fx/ folder does not exist or is inaccessible.\n");
        return;
    }

    printf("[Init] fx/ folder found, scanning for shaders...\n");

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("fx\\*.fx", &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("[Init] No .fx files found in fx/ folder.\n");
        return;
    }

    do
    {
        std::string fileName = findData.cFileName;
        std::string name = fileName.substr(0, fileName.find_last_of('.'));
        std::replace(name.begin(), name.end(), '-', '_');  // normalize dashes
        std::string key = "IDI_" + ToUpper(name) + "_FX";
        std::string fullPath = "fx/" + fileName;
        g_FxOverrides.insert(key); // ✅ REQUIRED for HookedCreateFromResource

        CompileAndDumpShader(key, fullPath);

        printf("[Init] Compiling and caching %s as %s\n", fileName.c_str(), key.c_str());

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
            printf("[Init] Failed to compile %s\n", fileName.c_str());
            if (errors)
            {
                printf("[Init] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        g_ShaderOverridePaths[key] = fullPath;
        g_ShaderBuffers[key] = data; // stores raw HLSL code
    }
    while (FindNextFileA(hFind, &findData));

    printf("[Debug] g_FxOverrides contains:\n");
    for (const auto& k : g_FxOverrides)
        printf("  - %s\n", k.c_str());

    FindClose(hFind);
}

// -------------------- HOOK HANDLER --------------------
bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx) return false;
    void** vtable = *(void***)fx;
    return vtable && !IsBadCodePtr((FARPROC)vtable[0]) && !IsBadCodePtr((FARPROC)vtable[1]);
}

__declspec(noinline)
const char* GetShaderSlotPtr(int i)
{
    const char** shaderTable = (const char**)0x008F9B60;
    __try {
        return shaderTable[i];
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}
int LookupShaderSlotIndex(const std::string& resourceName)
{
    for (int i = 0; i < 64; ++i)
    {
        const char* entry = GetShaderSlotPtr(i);
        if (!entry || IsBadStringPtrA(entry, 128))
            continue;

        std::string name(entry);
        size_t dot = name.find_last_of('.');
        if (dot != std::string::npos)
            name = name.substr(0, dot);

        std::string key = "IDI_" + ToUpper(name) + "_FX";
        if (key == resourceName)
            return i;
    }

    return -1;
}

void DumpShaderTable()
{
    static ID3DXEffect** table = (ID3DXEffect**)0x0093DE78; // NFS MW shader table

    printf("------ Shader Table Dump ------\n");
    for (int i = 0; i < 64; ++i)
    {
        ID3DXEffect* fx = table[i];
        if (!fx || !IsValidShaderPointer(fx))
            printf("[Check] Slot %02d = INVALID (%p)\n", i, fx);
        else
            printf("[Check] Slot %02d = VALID   (%p)\n", i, fx);
    }
}

void ApplyQueuedShaderPatches()
{
    std::lock_guard<std::mutex> lock(g_PatchMutex);
    static ID3DXEffect** table = (ID3DXEffect**)0x0093DE78;

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
            printf("[Patch] ❌ fx invalid for slot %d, skipping\n", patch.slot);
            continue;
        }

        if (table[patch.slot] && IsValidShaderPointer(table[patch.slot]))
        {
            printf("[Patch] 🔁 Released old fx in slot %d (%p)\n", patch.slot, table[patch.slot]);
            table[patch.slot]->Release();
        }

        // patch.fx->AddRef(); // assign to table
        table[patch.slot] = patch.fx;

        printf("[Patch] ✅ Wrote shader to slot %d (fx=%p)\n", patch.slot, patch.fx);
    }

    g_PendingPatches = std::move(stillPending);
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                              const RECT* src, const RECT* dest,
                              HWND hwnd, const RGNDATA* dirty)
{
    ApplyQueuedShaderPatches();  // ✅ Apply deferred effects first
    DumpShaderTable();           // ✅ Inspect current state BEFORE triggering graphics reload

    if (g_TriggerApplyGraphicsSettings)
    {
        g_TriggerApplyGraphicsSettings = false;
        printf("[Hook] Applying graphics settings (safe in Present)\n");
        ApplyGraphicsSettings(); // 💥 Game reinitializes shaders — crash may happen AFTER this
    }

    return RealPresent(device, src, dest, hwnd, dirty);
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
            RealPresent = (PresentFn)vtable[17];
            DWORD oldProtect;
            VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            vtable[17] = (void*)&HookedPresent;
            VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
            printf("[Init] Hooked IDirect3DDevice9::Present\n");
        }
    }

    printf("[Hook] D3DXCreateEffectFromResourceA called #%d — pResource = %s\n", g_HookCallCount++, pResource ? pResource : "(null)");

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
                printf("[Hook] Loaded compiled FX override for %s\n", pResource);
                return S_OK;
            }

            printf("[Hook] Failed to create effect from compiled override for %s\n", pResource);
        }
        else
        {
            printf("[Hook] Failed to open compiled file: %s\n", path.c_str());
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
                printf("[Hook] Loaded fallback compiled shader: %s\n", pResource);
                return S_OK;
            }

            printf("[Hook] Failed to load fallback compiled shader: %s\n", pResource);
        }
    }


    return RealCreateFromResource(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

bool SafePatchShaderTable(int slot, ID3DXEffect* fx)
{
    if (slot < 0 || slot >= 64 || !IsValidShaderPointer(fx))
        return false;

    {
        std::lock_guard<std::mutex> lock(g_PatchMutex);
        fx->AddRef(); // Defer patch safely
        g_PendingPatches.push_back({ slot, fx, 2 });
    }

    printf("[Patch] ⏳ Deferred shader patch queued: slot %d → fx=%p\n", slot, fx);
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
        printf("[Pause] Game Present thread paused.\n");
    else
        printf("[Pause] ⚠️ Timeout waiting for Present to stall — continuing anyway.\n");
}

void ResumeGameThread()
{
    g_PausePresent = false;
    printf("[Pause] Game Present thread resumed.\n");
}

// MUST NOT use STL types in this function!
static const char* SafeGetShaderTableEntry(int index)
{
    const char** table = (const char**)0x008F9B60;
    const char* result = nullptr;
    __try {
        result = table[index];
    } __except (EXCEPTION_EXECUTE_HANDLER) {
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

    for (int i = 0; i < 64; ++i)
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
    PauseGameThread();
    auto slots = LookupShaderSlotsFromResource(resourceKey);
    if (!slots.empty())
    {
        for (int slot : slots)
        {
            fx->AddRef();
            SafePatchShaderTable(slot, fx);
            printf("[HotReload] ✅ Patched slot %d with new effect for %s\n", slot, resourceKey.c_str());
        }
    }
    else
    {
        printf("[HotReload] ⚠️ No slots found for %s\n", resourceKey.c_str());
    }
    ResumeGameThread();
}

void RecompileAndReloadAll()
{
    for (const auto& key : g_FxOverrides)
    {
        auto it = g_ShaderOverridePaths.find(key);
        if (it == g_ShaderOverridePaths.end())
        {
            printf("[HotReload] ❌ No path found for %s\n", key.c_str());
            continue;
        }

        const std::string& fxPath = it->second;
        if (!CompileAndDumpShader(key, fxPath))
        {
            printf("[HotReload] ❌ Failed to recompile %s\n", fxPath.c_str());
            continue;
        }

        FILE* f = fopen(key.c_str(), "rb");
        if (!f)
        {
            printf("[HotReload] ❌ Failed to reopen compiled FX: %s\n", key.c_str());
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
            printf("[HotReload] ❌ Invalid fx pointer after compile: %p (hr=0x%08X)\n", fx, hr);
            if (errors)
            {
                printf("[HotReload] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        printf("[HotReload] ✅ Reloaded shader: %s (fx=%p)\n", key.c_str(), fx);

        if (!IsValidShaderPointer(fx))
        {
            printf("[HotReload] ❌ fx for %s is invalid, skipping patch\n", key.c_str());
            fx->Release(); // Clean up
            continue;
        }

        ForceReplaceShaderIntoSlots(key, fx);
        printf("[HotReload] ✅ Force-replaced effect in all slots for %s\n", key.c_str());
        
        printf("[HotReload] Triggering ApplyGraphicsSettings()\n");
        g_TriggerApplyGraphicsSettings = true;
        printf("[HotReload] Will apply graphics settings at next Present\n");
    }
}

DWORD WINAPI DeferredHookThread(LPVOID)
{
    while (!g_Device)
        Sleep(10);

    void** vtable = *(void***)g_Device;
    if (vtable)
    {
        RealPresent = (PresentFn)vtable[17];
        DWORD oldProtect;
        VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[17] = (void*)&HookedPresent;
        VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
        printf("[Init] Hooked IDirect3DDevice9::Present (deferred)\n");
    }
    return 0;
}

DWORD WINAPI HotkeyThread(LPVOID)
{
    while (true)
    {
        if (GetAsyncKeyState(VK_F10) & 1) // Press F10
        {
            printf("[HotkeyThread] F10 pressed → Recompiling FX overrides...\n");
            RecompileAndReloadAll();
        }
        Sleep(100);
    }
}

// -------------------- DllMain --------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        printf("[Init] Shader override DLL loaded.\n");

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (d3dx)
        {
            void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
            RealCreateFromResource = (D3DXCreateEffectFromResourceAFn)addr;
            injector::MakeCALL(0x006C60D2, HookedCreateFromResource, true);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HotkeyThread, nullptr, 0, nullptr);
            printf("[Init] Hooked D3DXCreateEffectFromResourceA\n");

            CreateThread(nullptr, 0, DeferredHookThread, nullptr, 0, nullptr);
        }
    }
    return TRUE;
}
