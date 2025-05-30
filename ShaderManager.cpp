#include "Hooks.h"
#include "ShaderManager.h"
#include <algorithm>
#include <atomic>
#include <windows.h>
#include <d3d9.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include "includes/injector/injector.hpp"
#include <iostream>
#include <mutex>
#include <unordered_set>
#include "FxWrapper.h"
#include "Validators.h"
#include <DbgHelp.h>
#include <set>
#pragma comment(lib, "Dbghelp.lib")

#define printf_s(...) asi_log::Log(__VA_ARGS__)

// Define a cast macro that allows old FxWrapper* to be used with minimal refactoring
// #define FX(x) ((FxWrapper*)(x))

const int ShaderTableSize = 62;
void** g_pVisualTreatment = (void**)0x00982AF0;

static FxWrapper** g_ShaderTable = (FxWrapper**)SHADER_TABLE_ADDRESS; // NFS MW shader g_ShaderTable
std::mutex g_ShaderTableLock = std::mutex();
std::unordered_map<std::string, FxWrapper*> g_DebugLiveEffects;
std::unordered_set<ID3DXEffect*> g_AlreadyInjectedFxThisFrame;

ShaderManager::PresentFn ShaderManager::RealPresent = nullptr;

static bool compiled = false;
bool g_WaitingForReset = false;
bool g_ApplyGraphicsSeenThisFrame = false;
bool g_CallApplyGraphicsManagerNextFrame = false;
int g_ApplyGraphicsTriggerDelay = 0;
bool g_ResumeGameThreadNextPresent = false;
static void* lastPatchedThis = nullptr;

bool g_EnableShaderTableDump = false;
void* g_LiveVisualTreatmentObject = nullptr;

// At top of HookApplyGraphicsSettings (outside function if needed)
static auto lastInvalidLogTime = std::chrono::steady_clock::now();
static void* lastInvalidVtObject = nullptr;

ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr; // ✅ definition
ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal = nullptr; // ✅ definition
IVisualTreatment_ResetFn IVisualTreatment_Reset = (IVisualTreatment_ResetFn)0x0073DE50;
void* g_ApplyGraphicsManagerThis = nullptr;
void* g_ApplyGraphicsSettingsThis = nullptr;
FxWrapper* g_LastReloadedFx = nullptr;

struct QueuedPatch
{
    int slot;
    FxWrapper* fx;
    int framesRemaining = 2; // safe delay
};

std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::vector<QueuedPatch> g_PendingPatches;
FxWrapper* g_SlotRetainedFx[62] = {};

std::mutex g_PatchMutex;
std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_set<std::string> g_FxOverrides;
static std::atomic<int> g_HookCallCount{0};
std::atomic<bool> g_TriggerApplyGraphicsSettings = false;

void* g_ThisCandidates[3] = {};
int g_ThisCount = 0;

std::atomic<bool> g_PausePresent{false};
std::atomic<bool> g_PresentIsWaiting{false};

// -------------------- NFSMW-RenderTarget block --------------------

LPDIRECT3DTEXTURE9 g_MotionBlurTex = nullptr;
LPDIRECT3DSURFACE9 g_MotionBlurSurface = nullptr;

std::unordered_map<std::string, FxWrapper*> g_ActiveEffects;

void* g_LastEView = nullptr;

typedef void (__thiscall*UpdateFunc)(void* thisptr, void* eView);
typedef void (__thiscall*FrameRenderFn)(void* thisptr);
FrameRenderFn ForceFrameRender = (FrameRenderFn)0x006DE300;

UpdateFunc OriginalUpdate = nullptr;

bool CreateMotionBlurResources(IDirect3DDevice9* device)
{
    if (!device)
        return false;

    ReleaseMotionBlurTexture(); // Always release before creating

    HRESULT hr = device->CreateTexture(
        512, 512, 1, D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
        &g_MotionBlurTex, nullptr);

    if (FAILED(hr) || !g_MotionBlurTex)
    {
        printf_s("[MotionBlur] ❌ Failed to create g_MotionBlurTex (0x%08X)\n", hr);
        return false;
    }

    HRESULT surfHr = g_MotionBlurTex->GetSurfaceLevel(0, &g_MotionBlurSurface);
    if (FAILED(surfHr) || !g_MotionBlurSurface)
    {
        printf_s("[MotionBlur] ❌ Failed to get surface from g_MotionBlurTex (0x%08X)\n", surfHr);
        return false;
    }

    printf_s("[MotionBlur] ✅ Created g_MotionBlurTex + Surface (%p)\n", g_MotionBlurSurface);
    return true;
}

#define SAFE_RELEASE(p) if ((p) != nullptr) { (p)->Release(); (p) = nullptr; }

void ReleaseMotionBlurTexture()
{
    // Only release if both are valid
    if (g_MotionBlurSurface)
    {
        printf_s("[ShaderManager] 🔻 Releasing g_MotionBlurSurface\n");
        SAFE_RELEASE(g_MotionBlurSurface);
    }

    if (g_MotionBlurTex)
    {
        printf_s("[ShaderManager] 🔻 Releasing g_MotionBlurTex\n");
        SAFE_RELEASE(g_MotionBlurTex);
    }

    // Notify effects
    for (auto& [name, fx] : g_ActiveEffects)
    {
        if (fx && fx->GetEffect())
        {
            fx->ClearMotionBlurTexture(); // ✨ This is key
            fx->OnLostDevice();
            printf_s("[ShaderManager] 🔁 Cleared motion blur on: %s\n", name.c_str());
        }
    }
}

void OnDeviceReset(LPDIRECT3DDEVICE9 device)
{
    if (!CreateMotionBlurResources(device))
        printf_s("[OnDeviceReset] ⚠️ Motion blur resources not recreated\n");
}

void OnDeviceLost()
{
    ReleaseMotionBlurTexture();

    printf_s("[OnDeviceLost] Released motion blur resources\n");
}

typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
Reset_t oReset = nullptr;

HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params)
{
    printf_s("[hkReset] Called\n");
    OnDeviceLost();

    // This is mostly for D3DPOOL_MANAGED, but calling it after OnDeviceLost():
    device->EvictManagedResources();

    HRESULT hr = oReset(device, params);
    if (SUCCEEDED(hr))
    {
        OnDeviceReset(device);
    }
    return hr;
}

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
void CompileShaderOverride()
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

        ID3DXEffect* rawFx = nullptr;
        FxWrapper* fx = nullptr;
        ID3DXBuffer* errors = nullptr;
        FXIncludeHandler includeHandler;

        HRESULT hr = D3DXCreateEffect(
            GetGameDevice(), data.data(), (UINT)len,
            nullptr, &includeHandler,
            D3DXSHADER_DEBUG, nullptr, &rawFx, &errors);

        if (SUCCEEDED(hr) && rawFx)
        {
            fx = new FxWrapper(rawFx);
            rawFx->Release(); // balance refcount
        }

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

        // ✅ Set g_LastReloadedFx if this is the visual treatment shader
        if (key == "IDI_VISUALTREATMENT_FX" && fx)
        {
            g_LastReloadedFx = fx;
            printf_s("[Init] ✅ Set g_LastReloadedFx = %p (IDI_VISUALTREATMENT_FX)\n", fx);
        }
    }
    while (FindNextFileA(hFind, &findData));

    printf_s("[Debug] g_FxOverrides contains:\n");
    for (const auto& k : g_FxOverrides)
        printf_s("  - %s\n", k.c_str());

    FindClose(hFind);
}

// -------------------- HOOK HANDLER --------------------


void DumpShaderTable()
{
    printf_s("------ Shader Table Dump ------\n");
    for (int i = 0; i < ShaderTableSize; ++i)
    {
        FxWrapper* fx = g_ShaderTable[i];
        if (!fx || !IsValidShaderPointer(fx))
            printf_s("[Check] Slot %02d = INVALID (%p)\n", i, fx);
        else
            printf_s("[Check] Slot %02d = VALID   (%p)\n", i, fx);
    }
}

void ApplyQueuedShaderPatches()
{
    std::lock_guard<std::mutex> lock(g_PatchMutex);
    static FxWrapper** table = (FxWrapper**)SHADER_TABLE_ADDRESS;

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

        FxWrapper* clone = nullptr;
        HRESULT hr = patch.fx->CloneEffect(GetGameDevice(), &clone);

        if (FAILED(hr) || !clone || !IsValidShaderPointer(clone))
        {
            printf_s("[Patch] ❌ CloneEffect failed for fx=%p, slot=%d (hr=0x%08X)\n", patch.fx, patch.slot,
                     (unsigned)hr);
            continue;
        }

        // Defensive: check/ref original for diagnostics
        ULONG refCount = 0;
        if (IsValidShaderPointer(patch.fx))
        {
            refCount = patch.fx->AddRef();
            patch.fx->Release();
        }

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

        // Replace in shader table and retain
        clone->AddRef(); // for g_ShaderTable
        table[patch.slot] = clone;

        // Release any previous retained copy
        if (g_SlotRetainedFx[patch.slot])
        {
            if (IsValidShaderPointer(g_SlotRetainedFx[patch.slot]))
                g_SlotRetainedFx[patch.slot]->Release();
            g_SlotRetainedFx[patch.slot] = nullptr;
        }

        // Store retained copy
        clone->AddRef(); // for g_SlotRetainedFx
        g_SlotRetainedFx[patch.slot] = clone;

        printf_s("[Patch] ✅ Cloned and wrote fx to slot %d (clone=%p, from=%p, refcount~%lu)\n",
                 patch.slot, clone, patch.fx, refCount);
    }

    g_PendingPatches = std::move(stillPending);
}

void ReleaseAllRetainedShaders()
{
    for (int i = 0; i < 62; ++i)
    {
        if (g_SlotRetainedFx[i])
        {
            // Log refcount for diagnostics
            ULONG refCount = g_SlotRetainedFx[i]->AddRef();
            g_SlotRetainedFx[i]->Release(); // Balance AddRef
            printf_s("[ShaderManager] 🔻 Releasing retained fx[%d] = %p (refcount ≈ %lu)\n", i, g_SlotRetainedFx[i],
                     refCount);

            // Remove from g_DebugLiveEffects by value
            for (auto it = g_DebugLiveEffects.begin(); it != g_DebugLiveEffects.end();)
            {
                if (it->second == g_SlotRetainedFx[i])
                    it = g_DebugLiveEffects.erase(it);
                else
                    ++it;
            }

            // Release actual
            g_SlotRetainedFx[i]->Release();
            g_SlotRetainedFx[i] = nullptr;
        }
    }
}

void ReleaseAllActiveEffects()
{
    for (auto& [name, fx] : g_ActiveEffects)
    {
        if (fx && fx->GetEffect())
        {
            void** vtable = *(void***)fx;
            if (!IsBadCodePtr((FARPROC)vtable[0]))
            {
                ULONG refCount = fx->AddRef();
                fx->Release(); // Balance
                printf_s("[ShaderManager] 🔻 Releasing active fx: %s (%p, refcount ≈ %lu)\n", name.c_str(), fx,
                         refCount);

                fx->Release();

                // Remove from g_DebugLiveEffects by value
                for (auto it = g_DebugLiveEffects.begin(); it != g_DebugLiveEffects.end();)
                {
                    if (it->second == fx)
                        it = g_DebugLiveEffects.erase(it);
                    else
                        ++it;
                }
            }
            else
            {
                printf_s("[ShaderManager] ⚠️ Skipped release for invalid fx: %s (%p)\n", name.c_str(), fx);
            }

            fx = nullptr;
        }
    }

    g_ActiveEffects.clear();
}

void TrackLiveEffect(const std::string& name, FxWrapper* fx)
{
    if (fx && fx->GetEffect())
    {
        g_DebugLiveEffects[name] = fx;
    }
}

void DumpLiveEffects()
{
    printf_s("[Debug] 📊 g_DebugLiveEffects has %zu entries\n", g_DebugLiveEffects.size());
    for (auto& [name, fx] : g_DebugLiveEffects)
    {
        printf_s("  - FX: %s @ %p\n", name.c_str(), fx);
    }
}

void ReleaseTrackedEffects()
{
    for (auto& [name, fx] : g_DebugLiveEffects)
    {
        if (fx && fx->GetEffect())
        {
            fx->Release();
        }
    }
    g_DebugLiveEffects.clear();
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
    if (!compiled)
    {
        SetGameDevice(device); // Set global pointer to known-good device

        CompileShaderOverride(); // Compile FX/*.fx → g_FxOverrides
        compiled = true;

        // 🛡️ Hook Present using *unhooked* vtable from freshly passed-in device
        void** vtable = *(void***)device;
        if (vtable && ShaderManager::RealPresent == nullptr)
        {
            ShaderManager::RealPresent = (ShaderManager::PresentFn)vtable[17];

            if (ShaderManager::RealPresent == &HookedPresent)
            {
                printf_s("❌ RealPresent already points to HookedPresent — skipping hook to avoid recursion.\n");
            }
            else
            {
                DWORD oldProtect;
                VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
                vtable[17] = (void*)&HookedPresent;
                VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);

                printf_s("[Init] ✅ Hooked IDirect3DDevice9::Present — RealPresent = %p\n", ShaderManager::RealPresent);
            }
        }
    }

    if (pResource && (strcmp(pResource, "IDI_WORLD_FX") == 0 || strcmp(pResource, "IDI_VISUALTREATMENT_FX") == 0))
    {
        printf_s("[Hook] D3DXCreateEffectFromResourceA called #%d — pResource = %s\n", g_HookCallCount++, pResource);
    }

    // 🔄 Try FX override from compiled shader folder
    if (g_FxOverrides.count(pResource))
    {
        std::string path = std::string(pResource);
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
                printf_s("[Hook] ✅ Loaded compiled FX override for %s\n", pResource);
                return S_OK;
            }

            printf_s("[Hook] ❌ Failed to create effect from compiled override: %s\n", pResource);
        }
        else
        {
            printf_s("[Hook] ❌ Failed to open compiled file: %s\n", path.c_str());
        }
    }

    // 📦 Fallback: Load shader directly from game folder
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
                printf_s("[Hook] ✅ Loaded fallback compiled shader: %s\n", pResource);
                return S_OK;
            }

            printf_s("[Hook] ❌ Failed to load fallback compiled shader: %s\n", pResource);
        }
    }

    // 🧪 Fall back to original unhooked CreateEffect
    return RealCreateFromResource(device, hModule, pResource,
                                  defines, include, flags, pool, outEffect, outErrors);
}

bool SafePatchShaderTable(int slot, FxWrapper* fx)
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
        Sleep(50);
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

const char* TryGetFxNameAnnotation(FxWrapper* fx)
{
    __try
    {
        // Get top-level object (global scope)
        D3DXHANDLE dummy = fx->GetEffect()->GetParameterByName(nullptr, nullptr);
        if (!dummy)
            return nullptr;

        // Get the annotation called "Name"
        D3DXHANDLE nameHandle = fx->GetEffect()->GetAnnotationByName(dummy, "Name");
        if (!nameHandle)
            return nullptr;

        LPCSTR nameStr = nullptr;
        if (SUCCEEDED(fx->GetEffect()->GetString(nameHandle, &nameStr)) && nameStr && !IsBadStringPtrA(nameStr, 128))
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

        FxWrapper** fxSlot = (FxWrapper**)((char*)vt + 0x18C);
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

void ForceReplaceShaderIntoSlots(const std::string& resourceKey, FxWrapper* fx)
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
            IVisualTreatment_Reset(*g_pVisualTreatment);
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

bool RecompileAndReloadAll()
{
    if (!GetGameDevice())
    {
        printf_s("[HotReload] ❌ GetGameDevice() is null, cannot recompile shaders.\n");
        return false;
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
        FxWrapper* fx = nullptr;
        ID3DXEffect* rawFx = nullptr;
        LPD3DXBUFFER errors = nullptr;

        HRESULT hr = D3DXCreateEffect(
            GetGameDevice(), buffer.data(), (UINT)len,
            nullptr, &includeHandler,
            D3DXSHADER_DEBUG, nullptr, &rawFx, &errors);

        // ✅ Wrap first
        if (SUCCEEDED(hr) && rawFx)
        {
            fx = new FxWrapper(rawFx);
            rawFx->Release(); // balance refcount
        }

        // ❌ Only check fx after assigning it
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
            if (fx && fx->GetEffect()) fx->Release(); // Only release if non-null
            continue;
        }

        // Now it's valid — proceed
        fx->AddRef(); // prevent premature destruction
        g_LastReloadedFx = fx;

        g_ActiveEffects["IDI_VISUALTREATMENT_FX"] = g_LastReloadedFx;

        // Track for motion blur injection
        TrackLiveEffect(key, fx);

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

    return true;
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

    if (IsValidShaderPointer((FxWrapper*)sub))
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

    // Optional: Check if +0x30 is likely a valid FxWrapper*
    void* fxCandidate = *(void**)((char*)candidate + 0x30);
    if ((uintptr_t)fxCandidate > 0x10000 && IsValidShaderPointer((FxWrapper*)fxCandidate))
    {
        printf_s("🔍 [IsLikelyApplyGraphicsSettingsObject] Slot +0x30 is valid shader pointer: %p\n", fxCandidate);
    }
    else
    {
        printf_s("🔍 [IsLikelyApplyGraphicsSettingsObject] Slot +0x30 not used or invalid\n");
    }

    return true;
}

bool ReplaceShaderSlot(BYTE* object, int offset, FxWrapper* newFx)
{
    if (!g_LastReloadedFx || !g_LastReloadedFx->GetEffect())
    {
        printf_s("❌ g_LastReloadedFx is invalid\n");
        return false;
    }

    FxWrapper** slot = (FxWrapper**)(object + offset);

    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery(slot, &mbi, sizeof(mbi)) || !(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)))
    {
        printf_s("❌ Cannot access shader slot at +0x%02X (%p)\n", offset, slot);
        return false;
    }

    FxWrapper* oldFx = nullptr;
    __try
    {
        oldFx = *slot;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }

    if (!newFx || !newFx->GetEffect())
    {
        printf_s("[HotReload] ❌ Invalid newFx pointer passed to ReplaceShaderSlot\n");
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
    {
        return g_ApplyGraphicsSettingsThis;
    }

    printf_s("❌ g_ApplyGraphicsSettingsThis is invalid — skipping ApplyGraphicsSettingsOriginal\n");

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
    if (!target || !IsValidThis(target))
        return;

    BYTE* base = (BYTE*)target;

    FxWrapper* currentFx = nullptr;
    FxWrapper** currentFxPtr = (FxWrapper**)CURRENTSHADER_OBJ_ADDR;
    if (IsValidShaderPointer(*currentFxPtr))
        currentFx = *currentFxPtr;

    printf_s("🧪 Scanning shader pointer slots (base = %p)...\n", base);

    for (int offset = 0; offset < 0x100; offset += 4)
    {
        FxWrapper** fxSlot = (FxWrapper**)(base + offset);
        FxWrapper* fx = nullptr;

        __try { fx = *fxSlot; }
        __except (EXCEPTION_EXECUTE_HANDLER) { continue; }

        if (!fx || !IsValidShaderPointer(fx))
            continue;

        ID3DXEffect* effect = nullptr;
        __try { effect = fx->GetEffect(); }
        __except (EXCEPTION_EXECUTE_HANDLER) { continue; }

        if (!effect || (uintptr_t)effect < 0x10000)
            continue;

        D3DXEFFECT_DESC d = {};
        HRESULT hr = E_FAIL;

        __try { hr = effect->GetDesc(&d); }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf_s("⚠️ Exception calling GetDesc at offset +0x%02X (fx=%p, effect=%p)\n", offset, fx, effect);
            continue;
        }

        if (!SUCCEEDED(hr))
        {
            printf_s("⚠️ GetDesc failed at offset +0x%02X (fx=%p)\n", offset, fx);
            continue;
        }

        // 🎯 Classification output
        if (fx == g_LastReloadedFx)
            printf_s("🎯 g_LastReloadedFx is at +0x%02X → %p (%s)\n", offset, fx, d.Creator);
        else if (fx == currentFx)
            printf_s("🌀 Current shader (CURRENTSHADER_OBJ_ADDR) at +0x%02X → %p (%s)\n", offset, fx, d.Creator);
        else if (offset == 0x18C)
            printf_s("🔷 g_pVisualTreatment +0x18C slot → %p (%s)\n", fx, d.Creator);
        else
            printf_s("🔍 Found valid shader at +0x%02X → %p (%s)\n", offset, fx, d.Creator);
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

    // Replace any slots with g_LastReloadedFx or fuzzy match
    if (g_LastReloadedFx && IsValidShaderPointer(g_LastReloadedFx))
    {
        printf_s("🧪 Entering TryApplyGraphicsSettingsSafely() — g_LastReloadedFx=%p\n", g_LastReloadedFx);

        // 📌 Patch known current shader slot first (CURRENTSHADER_OBJ_ADDR)
        FxWrapper** currentShader = (FxWrapper**)CURRENTSHADER_OBJ_ADDR;
        if (IsValidShaderPointer(*currentShader))
        {
            if (*currentShader != g_LastReloadedFx)
            {
                printf_s("🎯 Overwriting CURRENTSHADER_OBJ_ADDR (%p → %p)\n", *currentShader, g_LastReloadedFx);
                *currentShader = g_LastReloadedFx;
            }
            else
            {
                printf_s("✅ g_LastReloadedFx already set in CURRENTSHADER_OBJ_ADDR\n");
            }
        }

        // 📌 Patch known visual treatment slot at +0x18C if applicable
        if (targetThis)
        {
            ReplaceShaderSlot((BYTE*)targetThis, 0x18C, g_LastReloadedFx);
        }

        // 📦 Fallback: scan memory for other matching FxWrapper* instances
        BYTE* obj = (BYTE*)targetThis;

        D3DXEFFECT_DESC newDesc = {};
        if (FAILED(g_LastReloadedFx->GetEffect()->GetDesc(&newDesc)))
        {
            printf_s("❌ Failed to get desc for g_LastReloadedFx\n");
            return false;
        }

        for (int offset = 0; offset < 0x100; offset += 4)
        {
            FxWrapper** fxSlot = (FxWrapper**)(obj + offset);
            FxWrapper* fx = nullptr;

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
                g_LastReloadedFx->ReloadHandles();
                continue;
            }

            // ⚠️ Fuzzy match against previous shader based on D3DXEFFECT_DESC
            ID3DXEffect* fxEffect = nullptr;
            __try
            {
                fxEffect = fx->GetEffect();
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                continue;
            }

            if (!fxEffect)
                continue;

            D3DXEFFECT_DESC desc = {};
            if (SUCCEEDED(fxEffect->GetDesc(&desc)) &&
                !strcmp(desc.Creator, newDesc.Creator) &&
                desc.Parameters == newDesc.Parameters &&
                desc.Techniques == newDesc.Techniques)
            {
                printf_s("🔶 Fuzzy match at +0x%02X (%p) — replacing\n", offset, fx);
                ReplaceShaderSlot(obj, offset, g_LastReloadedFx);
            }
        }
    }

    // Call ApplyGraphicsSettings
    printf_s("🎯 Calling ApplyGraphicsSettingsOriginal(manager=%p, object=%p)\n",
             g_ApplyGraphicsManagerThis, targetThis);

    __try
    {
        // PrintFxAtOffsets(targetThis);
        ApplyGraphicsSettingsOriginal(g_ApplyGraphicsManagerThis, nullptr, targetThis);
        // PrintFxAtOffsets(targetThis);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf_s("❌ Exception during ApplyGraphicsSettingsOriginal call\n");
        return false;
    }

    // PrintFxAtOffsets(targetThis); // again after call

    g_ThisCount = 0;
    g_TriggerApplyGraphicsSettings = false; // ✅ Prevent further log spam
    return true;
}

void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject)
{
    LogApplyGraphicsSettingsCall(manager, vtObject, 1);

    if (g_WaitingForReset)
    {
        // skip scanning or patching — game is in reset
        return;
    }

    if (IsValidThis(manager))
    {
        g_ApplyGraphicsManagerThis = manager;
    }
    else
    {
        printf_s("[Hook] ❌ manager is invalid!\n");
    }

    const bool triggerActive = g_TriggerApplyGraphicsSettings;

    if (IsValidThis(vtObject))
    {
        // Validate the [vtObject + 0x140] chain with try/except
        // void* ptr140 = nullptr;
        // __try
        // {
        //     ptr140 = *(void**)((char*)vtObject + 0x140);
        // }
        // __except (EXCEPTION_EXECUTE_HANDLER)
        // {
        //     ptr140 = nullptr;
        // }

        // printf_s("[Debug] vtObject=%p  ptr140=%p\n", vtObject, ptr140);

        // if (!IsValidThis(ptr140))
        // {
        //     static auto lastLogTime = std::chrono::steady_clock::now();
        //     auto now = std::chrono::steady_clock::now();
        //     if (now - lastLogTime > std::chrono::seconds(1))
        //     {
        //         printf_s("[XNFS-ShaderLoader-MW] ❌ Still invalid: vtObject+0x140 = %p (from %p)\n", ptr140, vtObject);
        //         lastLogTime = now;
        //     }
        //     return;
        // }

        // Only update if new
        if (!g_ApplyGraphicsSettingsThis)
        {
            g_ApplyGraphicsSettingsThis = vtObject;
            printf_s("[Hook] ✅ Captured g_ApplyGraphicsSettingsThis: %p\n", vtObject);
        }

        if (g_ThisCount < 3 && !AlreadyStored(vtObject))
        {
            g_ThisCandidates[g_ThisCount++] = vtObject;
        }

        if (triggerActive && g_LastReloadedFx && vtObject != lastPatchedThis)
        {
            printf_s("[HotReload] 🔁 Applying shader and reset for vtObject = %p\n", vtObject);
            ReplaceShaderSlot((BYTE*)vtObject, 0x18C, g_LastReloadedFx);
            g_LastReloadedFx->ReloadHandles();

            if (g_pVisualTreatment && *g_pVisualTreatment)
            {
                IVisualTreatment_Reset(*g_pVisualTreatment);
                printf_s("[HotReload] 🔁 Called IVisualTreatment::Reset()\n");
            }

            lastPatchedThis = vtObject;
        }

        LogApplyGraphicsSettingsCall(manager, vtObject, 2);
    }

    if (ApplyGraphicsSettingsOriginal)
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
}

// Hook to capture eView*
void __fastcall HookedUpdate(void* thisptr, void*, void* eView)
{
    if (!g_LastEView && eView)
    {
        g_LastEView = eView;
        printf_s("[Capture] g_LastEView = %p\n", eView);
    }

    if (OriginalUpdate)
        OriginalUpdate(thisptr, eView);
    else
        ((UpdateFunc)0x0074C000)(thisptr, eView); // Fallback to original address
}

HRESULT TryFullGraphicsReset()
{
    static bool hookInstalled = false;
    if (!hookInstalled)
    {
        injector::MakeCALL(0x006DE299, HookedUpdate, true);
        hookInstalled = true;
    }

    if (!g_LastEView)
    {
        g_LastEView = *(void**)EViewsBase;
        printf_s("[Fallback] Using eViews[0] from 0x009195E0 → %p\n", g_LastEView);
    }

    if (!g_LastEView || !g_pVisualTreatment || !*g_pVisualTreatment)
    {
        printf_s("⚠️ Cannot reset visuals: g_LastEView or g_pVisualTreatment is null\n");
        return E_POINTER;
    }

    // (Optional) Patch vtable[3] for HookedUpdate if not done
    BYTE** vtable = *(BYTE***)g_pVisualTreatment;
    if (vtable)
    {
        DWORD oldProtect;
        VirtualProtect(&vtable[3], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        OriginalUpdate = (UpdateFunc)vtable[3];
        vtable[3] = (BYTE*)&HookedUpdate;
        VirtualProtect(&vtable[3], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[Init] ✅ Hooked IVisualTreatment::Update(eView*) at vtable[3]\n");
    }

    if (!g_LastReloadedFx || !g_LastReloadedFx->GetEffect())
    {
        printf_s("❌ Refusing to patch slot — g_LastReloadedFx or effect is null\n");
        return E_POINTER;
    }

    if (IsValidThis(g_ApplyGraphicsSettingsThis))
    {
        void* applyThis = ResolveApplyGraphicsThis();
        if (!applyThis)
        {
            printf_s("[HotReload] ❌ Cannot replace shader slot — ApplyGraphicsSettingsThis unresolved\n");
            return E_POINTER;
        }
        ReplaceShaderSlot((BYTE*)applyThis, 0x18C, g_LastReloadedFx);
    }
    else if (g_pVisualTreatment && *g_pVisualTreatment)
    {
        BYTE* vtObj = (BYTE*)(*g_pVisualTreatment);
        printf_s("[Fallback] Patching g_pVisualTreatment at +0x18C (vtObj = %p)\n", vtObj);
        ReplaceShaderSlot(vtObj, 0x18C, g_LastReloadedFx);
    }
    else
    {
        printf_s("[HotReload] ❌ Cannot patch shader slot — no valid this-pointer\n");
    }

    // Optional debug output — only if QueryInterface succeeds
    IDirect3DDevice9Ex* deviceEx = nullptr;
    if (SUCCEEDED(GetGameDevice()->QueryInterface(__uuidof(IDirect3DDevice9Ex), (void**)&deviceEx)) && deviceEx)
    {
        HRESULT hr = deviceEx->CheckDeviceState(nullptr);
        printf_s("[Debug] CheckDeviceState: 0x%08X\n", hr);
        deviceEx->Release();
    }
    else
    {
        printf_s("[Debug] IDirect3DDevice9Ex not available — skipping CheckDeviceState\n");
    }

    IVisualTreatment_Reset(*g_pVisualTreatment);
    printf_s("[HotReload] 🔁 Called IVisualTreatment::Reset()\n");

    // (Optional but recommended)
    GetGameDevice()->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    GetGameDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    GetGameDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    GetGameDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    GetGameDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
    GetGameDevice()->SetRenderState(D3DRS_COLORVERTEX, TRUE);
    GetGameDevice()->SetRenderState(D3DRS_CLIPPING, TRUE);
    GetGameDevice()->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    printf_s("[HotReload] ✅ Z-buffer re-enabled\n");

    // Dummy draw to force internal state update
    IDirect3DSurface9* backbuffer = nullptr;
    if (SUCCEEDED(GetGameDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer)) && backbuffer)
    {
        GetGameDevice()->SetRenderTarget(0, backbuffer);
        GetGameDevice()->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
        GetGameDevice()->BeginScene();
        GetGameDevice()->EndScene();
        backbuffer->Release();
        printf_s("[HotReload] 🧪 Performed dummy draw to rebind device state\n");
    }

    *(BYTE*)0x00982C39 = 1;
    printf_s("[HotReload] ✅ Set LoadedFlagMaybe = 1 (0x00982C39)\n");

    if (ForceFrameRender && g_ApplyGraphicsManagerThis)
    {
        ForceFrameRender(g_ApplyGraphicsManagerThis);
        printf_s("[HotReload] ✅ ForceFrameRender (sub_6DE300) called\n");
    }
    else
    {
        printf_s("[HotReload] ❌ Cannot call ForceFrameRender — missing thisptr\n");
    }

    // Force a manual call to IVisualTreatment::Update(eView*)
    if (g_pVisualTreatment && *g_pVisualTreatment)
    {
        BYTE** vtable = *(BYTE***)g_pVisualTreatment;
        if (vtable)
        {
            DWORD oldProtect;
            VirtualProtect(&vtable[3], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            OriginalUpdate = (UpdateFunc)vtable[3];
            vtable[3] = (BYTE*)&HookedUpdate;
            VirtualProtect(&vtable[3], sizeof(void*), oldProtect, &oldProtect);
            printf_s("[Init] ✅ Hooked IVisualTreatment::Update(eView*) at vtable[3]\n");
        }
    }
    else
    {
        printf_s("❌ g_pVisualTreatment is null — cannot install vtable hook!\n");
    }

    return S_OK;
}

static bool InjectSharedTextures(IDirect3DDevice9* /*unused*/)
{
    static thread_local int callDepth = 0;
    static thread_local bool inProgress = false;

    // Prevent re-entry or stack explosion
    if (inProgress || callDepth > 10)
    {
        if (callDepth > 10)
            printf_s("⚠️ InjectSharedTextures: callDepth=%d — skipping to prevent overflow\n", callDepth);
        return false;
    }

    inProgress = true;
    ++callDepth;

    static int frameCounter = 0;
    ++frameCounter;

    g_AlreadyInjectedFxThisFrame.clear();
    bool result = false;

    do
    {
        IDirect3DDevice9* device = GetGameDevice();
        if (!device) break;

        if (!g_MotionBlurTex && !CreateMotionBlurResources(device))
            break;

        if (!g_LastReloadedFx || !g_LastReloadedFx->GetEffect())
            break;

        // 🧷 Shallow copy to prevent iterator invalidation
        std::vector<std::string> keys;
        try {
            for (const auto& [name, fx] : g_ActiveEffects)
                keys.push_back(name);
        }
        catch (...)
        {
            printf_s("❌ Exception copying g_ActiveEffects keys\n");
            break;
        }

        for (const auto& name : keys)
        {
            auto it = g_ActiveEffects.find(name);
            if (it == g_ActiveEffects.end()) continue;

            FxWrapper* fx = it->second;
            if (!fx || !fx->GetEffect()) {
                printf_s("[ShaderManager] ⚠️ Skipped: null effect for %s\n", name.c_str());
                continue;
            }

            ID3DXEffect* e = fx->GetEffect();
            if (!e || g_AlreadyInjectedFxThisFrame.count(e)) continue;

            fx->ReloadHandles();

            D3DXHANDLE texHandle = e->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
            D3DXHANDLE blurHandle = e->GetParameterByName(nullptr, "BlurParams");

            HRESULT hr1 = texHandle ? e->SetTexture(texHandle, g_MotionBlurTex) : E_FAIL;
            D3DXVECTOR4 blurParams(0.5f, 0.3f, 1.0f, 0.0f);
            HRESULT hr2 = blurHandle ? e->SetVector(blurHandle, &blurParams) : E_FAIL;

            if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
            {
                if (frameCounter % 60 == 0)
                    printf_s("[ShaderManager] ✅ Set motion blur texture in: %s\n", name.c_str());
                g_AlreadyInjectedFxThisFrame.insert(e);
            }
            else
            {
                printf_s("[ShaderManager] ❌ Failed to inject into: %s (hr1=0x%08X, hr2=0x%08X)\n",
                         name.c_str(), hr1, hr2);
            }
        }

        // Handle g_LastReloadedFx separately
        ID3DXEffect* e = g_LastReloadedFx->GetEffect();
        if (e && !g_AlreadyInjectedFxThisFrame.count(e))
        {
            g_LastReloadedFx->ReloadHandles();

            D3DXHANDLE texHandle = e->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
            D3DXHANDLE blurHandle = e->GetParameterByName(nullptr, "BlurParams");

            HRESULT hr1 = texHandle ? e->SetTexture(texHandle, g_MotionBlurTex) : E_FAIL;
            D3DXVECTOR4 blurParams(0.5f, 0.3f, 1.0f, 0.0f);
            HRESULT hr2 = blurHandle ? e->SetVector(blurHandle, &blurParams) : E_FAIL;

            if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
            {
                if (frameCounter % 60 == 0)
                    printf_s("[ShaderManager] ✅ Set motion blur texture in: g_LastReloadedFx\n");
                g_AlreadyInjectedFxThisFrame.insert(e);
            }
            else
            {
                printf_s("[ShaderManager] ❌ Failed to inject into g_LastReloadedFx (hr1=0x%08X, hr2=0x%08X)\n", hr1, hr2);
            }
        }

        result = true;

    } while (false);

    inProgress = false;
    --callDepth;
    return result;
}

bool InjectSharedTextures2(IDirect3DDevice9* device)
{
    if (!device)
        return false;

    if (!g_MotionBlurTex && !CreateMotionBlurResources(device))
        return false;

    // Inject into all active effects
    for (auto& [name, fx] : g_ActiveEffects)
    {
        FxWrapper* effect = fx;
        if (!effect)
            continue;

        fx->ReloadHandles(); // Reload after hot-reload

        // ✅ 3. Validate handles before SetTexture/SetVector
        D3DXHANDLE texHandle = effect->GetEffect()->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
        D3DXHANDLE blurHandle = effect->GetEffect()->GetParameterByName(nullptr, "BlurParams");

        HRESULT hr1 = texHandle ? effect->GetEffect()->SetTexture(texHandle, g_MotionBlurTex) : E_POINTER;
        HRESULT hr2 = blurHandle
                          ? effect->GetEffect()->SetVector(blurHandle, &D3DXVECTOR4(0.5f, 0.3f, 1.0f, 0.0f))
                          : E_POINTER;

        if (FAILED(hr1) || FAILED(hr2))
        {
            printf_s("[ShaderManager] ⚠️ Failed to set one or more params in %s (retrying ReloadHandles)\n",
                     name.c_str());
            fx->ReloadHandles(); // ✅ point 4 — force handle reset
            texHandle = effect->GetEffect()->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
            blurHandle = effect->GetEffect()->GetParameterByName(nullptr, "BlurParams");

            if (!g_MotionBlurTex || !fx->GetEffect())
                continue;

            hr1 = texHandle ? effect->GetEffect()->SetTexture(texHandle, g_MotionBlurTex) : E_POINTER;
            hr2 = blurHandle
                      ? effect->GetEffect()->SetVector(blurHandle, &D3DXVECTOR4(0.5f, 0.3f, 1.0f, 0.0f))
                      : E_POINTER;
        }
    }

    // Inject into hot-reloaded shader directly
    if (g_LastReloadedFx && IsValidShaderPointer(g_LastReloadedFx))
    {
        FxWrapper* fx = g_LastReloadedFx;
        if (fx)
        {
            D3DXHANDLE texHandle = fx->GetEffect()->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
            D3DXHANDLE blurHandle = fx->GetEffect()->GetParameterByName(nullptr, "BlurParams");

            HRESULT hr1 = texHandle ? fx->GetEffect()->SetTexture(texHandle, g_MotionBlurTex) : E_POINTER;
            HRESULT hr2 = blurHandle
                              ? fx->GetEffect()->SetVector(blurHandle, &D3DXVECTOR4(0.5f, 0.3f, 1.0f, 0.0f))
                              : E_POINTER;

            if (FAILED(hr1) || FAILED(hr2))
            {
                printf_s("[ShaderManager] ⚠️ Failed to set params on hot-reloaded effect, reloading handles...\n");
                g_LastReloadedFx->ReloadHandles();
                texHandle = fx->GetEffect()->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
                blurHandle = fx->GetEffect()->GetParameterByName(nullptr, "BlurParams");

                if (fx && g_MotionBlurTex)
                {
                    hr1 = texHandle ? fx->GetEffect()->SetTexture(texHandle, g_MotionBlurTex) : E_POINTER;
                    hr2 = blurHandle
                              ? fx->GetEffect()->SetVector(blurHandle, &D3DXVECTOR4(0.5f, 0.3f, 1.0f, 0.0f))
                              : E_POINTER;
                }
            }
        }
    }

    return true;
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                             const RECT* src, const RECT* dest,
                             HWND hwnd, const RGNDATA* dirty)
{
    static bool ranThisFrame = false;
    if (ranThisFrame) return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
    ranThisFrame = true;
    
    if (g_EnableShaderTableDump)
        DumpShaderTable();

    static int waitFrames = 0;
    static int resetTimeout = 0;

    // Handle shader hot-reload trigger
    if (g_TriggerApplyGraphicsSettings)
    {
        if (g_ApplyGraphicsTriggerDelay > 0)
        {
            g_ApplyGraphicsTriggerDelay--;
        }
        else
        {
            printf_s("✅ ApplyGraphicsSettings hook confirmed — applying shader changes\n");

            g_TriggerApplyGraphicsSettings = false;
            g_WaitingForReset = true;
            waitFrames = 0;

            // ✅ Ensure last reloaded shader is valid before proceeding                   
            if (!g_LastReloadedFx || !g_LastReloadedFx->GetEffect() || !IsValidShaderPointer(g_LastReloadedFx))
            {
                printf_s("[HotReload] ❌ g_LastReloadedFx invalid before reset, aborting\n");
                g_WaitingForReset = false;
                return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
            }

            g_LastReloadedFx->AddRef(); // prevent early release

            // 🧹 Pause and fully clean up before triggering reset
            PauseGameThread();

            {
                std::lock_guard<std::mutex> lock(g_ShaderTableLock); // synchronize access
                ReleaseAllRetainedShaders();
                ReleaseAllActiveEffects();
            }

            DumpLiveEffects();

            // IDirect3DDevice9Ex* deviceEx = nullptr;
            // if (SUCCEEDED(GetGameDevice()->QueryInterface(IID_PPV_ARGS(&deviceEx))) && deviceEx)
            // {
            //     UINT count = 0;
            //     HRESULT hr = deviceEx->CheckDeviceState(nullptr);
            //     printf_s("[Debug] CheckDeviceState: 0x%08X\n", hr);
            //     deviceEx->Release();
            // }

            IDirect3DDevice9Ex* deviceEx = nullptr;
            if (SUCCEEDED(GetGameDevice()->QueryInterface(__uuidof(IDirect3DDevice9Ex), (void**)&deviceEx)) && deviceEx)
            {
                HRESULT hr = deviceEx->CheckDeviceState(nullptr);
                printf_s("[Debug] CheckDeviceState: 0x%08X\n", hr);
                deviceEx->Release();
            }
            else
            {
                printf_s("[Debug] IDirect3DDevice9Ex not available — skipping CheckDeviceState\n");
            }

            // 🌀 Trigger the full reset first
            TryFullGraphicsReset(); // includes IVisualTreatment::Reset

            // 🧪 Inject shared textures after full reset
            InjectSharedTextures(GetGameDevice());

            // 🔁 Trigger ApplyGraphicsManager to finalize settings
            if (IsValidThis(g_ApplyGraphicsManagerThis))
            {
                ApplyGraphicsManagerMainOriginal(g_ApplyGraphicsManagerThis);
                printf_s("[HotReload] ✅ ApplyGraphicsManagerMain called (post-reset)\n");

                void* resolved = ResolveApplyGraphicsThis();
                if (resolved && IsValidThis(resolved))
                {
                    ApplyGraphicsSettingsOriginal(g_ApplyGraphicsManagerThis, nullptr, resolved);
                }
                else
                {
                    printf_s(
                        "[HotReload] ⚠️ Skipping ApplyGraphicsSettingsOriginal — 'this' is stale or unresolved.\n");
                }

                // ⬅️ Move this UP — before ResumeGameThread
                if (!TryApplyGraphicsSettingsSafely())
                {
                    printf_s("[HotReload] ❌ TryApplyGraphicsSettingsSafely failed — final fallback attempt\n");
                }

                ReleaseMotionBlurTexture();
                printf_s("[ShaderManager] 🔻 Released g_MotionBlurTex (pre-reset)\n");
            }
            else
            {
                printf_s("[HotReload] ⚠️ Invalid ApplyGraphicsManagerThis — skipping call\n");
            }

            if (IsValidThis(g_ApplyGraphicsSettingsThis))
            {
                void* applyThis = ResolveApplyGraphicsThis();
                if (!applyThis)
                {
                    printf_s("[HotReload] ❌ Cannot replace shader slot — ApplyGraphicsSettingsThis unresolved\n");
                    return E_POINTER;
                }
                g_LastReloadedFx->GetEffect()->OnLostDevice();

                ReplaceShaderSlot((BYTE*)applyThis, 0x18C, g_LastReloadedFx);
            }
            else if (g_pVisualTreatment && *g_pVisualTreatment)
            {
                BYTE* vtObj = (BYTE*)(*g_pVisualTreatment);
                printf_s("[Fallback] Patching g_pVisualTreatment at +0x18C (vtObj = %p)\n", vtObj);
                g_LastReloadedFx->GetEffect()->OnLostDevice();

                ReplaceShaderSlot(vtObj, 0x18C, g_LastReloadedFx);
            }
            else
            {
                printf_s("[HotReload] ❌ Cannot patch shader slot — no valid this-pointer\n");
            }

            // if (g_LastReloadedFx->IsValid())
            // {
            //     g_LastReloadedFx->Release();
            //     g_LastReloadedFx = nullptr;
            // }

            if (ApplyGraphicsManagerMainOriginal && g_ApplyGraphicsManagerThis)
            {
                ApplyGraphicsManagerMainOriginal(g_ApplyGraphicsManagerThis);
                printf_s("[HotReload] ✅ ApplyGraphicsManagerMain reapplied\n");
            }

            g_TriggerApplyGraphicsSettings = false;
            lastPatchedThis = nullptr; // 🔁 ready for next reload

            ResumeGameThread();
            printf_s("[HotReload] ✅ Game Present thread resumed (immediate)\n");
            g_ResumeGameThreadNextPresent = false;
            g_WaitingForReset = false;

            g_CallApplyGraphicsManagerNextFrame = false;
            printf_s("[HotReload] ✅ Full graphics reapplication triggered\n");

            return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
        }

        // Waiting loop for ApplyGraphicsSettings
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

    // Safety: if reset hangs, auto-resume game thread after timeout
    if (g_WaitingForReset)
    {
        if (++resetTimeout > 360)
        {
            printf_s("[HotReload] ❌ Reset timeout — forcing resume\n");
            ResumeGameThread();
            printf_s("[HotReload] ✅ Game Present thread resumed (forced)\n");
            g_ResumeGameThreadNextPresent = true;
            g_WaitingForReset = false;
            resetTimeout = 0;
        }
    }
    else
    {
        resetTimeout = 0;
    }

    // ✅ Inject shared textures only if not inside reset
    if (!g_WaitingForReset)
        InjectSharedTextures(device);

    ApplyQueuedShaderPatches();

    lastPatchedThis = nullptr; // 🔁 ready for next reload

    if (g_ResumeGameThreadNextPresent)
    {
        ResumeGameThread();
        g_ResumeGameThreadNextPresent = false;
        printf_s("[HotReload] ✅ Game Present thread resumed (delayed)\n");

        // 🛠️ Fallback: force patch using known IVisualTreatment if ApplyGraphicsSettingsThis was never seen
        if (!IsValidThis(g_ApplyGraphicsSettingsThis) && g_pVisualTreatment && *g_pVisualTreatment)
        {
            BYTE* obj = (BYTE*)(*g_pVisualTreatment);
            if (obj && IsValidShaderPointer(g_LastReloadedFx))
            {
                printf_s("[HotReload] 🛠️ Forcing fallback shader patch via g_pVisualTreatment\n");
                ReplaceShaderSlot(obj, 0x18C, g_LastReloadedFx);
                g_LastReloadedFx->ReloadHandles();
            }
        }
    }

    return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
}
