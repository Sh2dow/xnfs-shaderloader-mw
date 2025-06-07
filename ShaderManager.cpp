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
#include <set>

// Define a cast macro that allows old std::shared_ptr<FxWrapper> to be used with minimal refactoring
// #define FX(x) ((std::shared_ptr<FxWrapper>)(x))
const int ShaderTableSize = 62;

static std::shared_ptr<FxWrapper>* g_ShaderTable = (std::shared_ptr<FxWrapper>*)SHADER_TABLE_ADDRESS;
// NFS MW shader g_ShaderTable
std::mutex g_ShaderTableLock = std::mutex();
std::unordered_map<std::string, std::shared_ptr<FxWrapper>> g_DebugLiveEffects;
std::unordered_set<ID3DXEffect*> g_AlreadyInjectedFxThisFrame;

ShaderManager::PresentFn ShaderManager::RealPresent = nullptr;


bool g_WaitingForReset = false;
bool g_ApplyGraphicsSeenThisFrame = false;
bool g_CallApplyGraphicsManagerNextFrame = false;
int g_ApplyGraphicsTriggerDelay = 0;
bool g_ResumeGameThreadNextPresent = false;

bool g_EnableShaderTableDump = false;
void* g_LiveVisualTreatmentObject = nullptr;
// Map from shader resource name to fallback slot
std::unordered_map<std::string, int> g_DynamicFallbackSlots;

bool g_PendingVisualReset = false;

ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr; // ✅ definition
ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal = nullptr; // ✅ definition
IVisualTreatment_ResetFn IVisualTreatment_Reset = (IVisualTreatment_ResetFn)0x0073DE50;
void* g_ApplyGraphicsManagerThis = nullptr;
void* g_ApplyGraphicsSettingsThis = nullptr;

struct QueuedPatch
{
    int slot = -1;
    std::shared_ptr<FxWrapper> fx;
    int framesRemaining = 2; // safe delay
    std::string resourceName;
    
};

std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::vector<QueuedPatch> g_PendingPatches;
std::shared_ptr<FxWrapper> g_SlotRetainedFx[64] = {};

std::mutex g_PatchMutex;
std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_set<std::string> g_FxOverrides;

static std::atomic<int> g_HookCallCount{0};
std::atomic<bool> g_TriggerApplyGraphicsSettings = false;

std::atomic<bool> g_PausePresent{false};
std::atomic<bool> g_PresentIsWaiting{false};

// -------------------- NFSMW-RenderTarget block --------------------

LPDIRECT3DTEXTURE9 g_MotionBlurTex = nullptr;
LPDIRECT3DSURFACE9 g_MotionBlurSurface = nullptr;

std::unordered_map<std::string, std::shared_ptr<FxWrapper>> g_ActiveEffects;

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
        std::replace(name.begin(), name.end(), '-', '_');
        std::string key = "IDI_" + ToUpper(name) + "_FX";
        std::string fullPath = "fx/" + fileName;

        // ✅ COMPILE IT!
        if (!CompileAndDumpShader(key, fullPath))
        {
            printf("[Init] ❌ Failed to compile %s\n", fileName.c_str());
            continue;
        }

        g_FxOverrides.insert(key);

        printf("[Init] ✅ Compiled and registered %s as %s\n", fileName.c_str(), key.c_str());
    }
    while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
}

static void TrackLiveEffect(const std::string& name, std::shared_ptr<FxWrapper> fx)
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

std::shared_ptr<FxWrapper> CompileFxFromFile(const std::string& key, const std::string& fxPath)
{
    FILE* f = fopen(fxPath.c_str(), "rb");
    if (!f)
    {
        printf_s("[HotReload] ❌ Failed to open %s\n", fxPath.c_str());
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::vector<char> buffer(len);
    fread(buffer.data(), 1, len, f);
    fclose(f);

    FXIncludeHandler includeHandler;
    ID3DXEffect* rawFx = nullptr;
    LPD3DXBUFFER errors = nullptr;

    HRESULT hr = D3DXCreateEffect(
        GetGameDevice(), buffer.data(), (UINT)len,
        nullptr, &includeHandler,
        D3DXSHADER_DEBUG, nullptr, &rawFx, &errors);

    if (FAILED(hr) || !rawFx)
    {
        printf_s("[HotReload] ❌ Failed to compile effect for %s (hr=0x%08X)\n", key.c_str(), hr);
        if (errors)
        {
            printf_s("[HotReload] Error: %s\n", (char*)errors->GetBufferPointer());
            errors->Release();
        }
        return nullptr;
    }

    std::shared_ptr<FxWrapper> wrapper = std::make_shared<FxWrapper>(rawFx);
    rawFx->Release(); // balance refcount

    if (!IsValidShaderPointer(wrapper))
    {
        printf_s("[HotReload] ❌ Invalid FxWrapper for %s\n", key.c_str());
        return nullptr;
    }

    return wrapper;
}

bool CompileShaderOverrides()
{
    bool anyCompiled = false;

    for (const auto& [key, path] : g_ShaderOverridePaths)
    {
        if (CompileAndDumpShader(key, path))
        {
            g_FxOverrides.insert(key);
            anyCompiled = true;
        }
        else
        {
            printf_s("[HotReload] ❌ Failed to compile %s → %s\n", path.c_str(), key.c_str());
        }
    }

    return anyCompiled;
}

bool RecompileAndReloadAll()
{
    CompileShaderOverrides(); // ✅ Compile all shaders before reloading

    bool patchedAny = false;

    for (const auto& key : g_FxOverrides)
    {
        std::string path = key; // ✅ FIXED PATH

        FILE* f = fopen(path.c_str(), "rb");
        if (!f)
        {
            printf_s("[HotReload] ❌ Failed to open compiled FX: %s\n", path.c_str());
            continue;
        }

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (len == 0)
        {
            fclose(f);
            continue;
        }

        std::vector<char> buffer(len);
        size_t bytesRead = fread(buffer.data(), 1, len, f);
        fclose(f);

        if (bytesRead != len)
        {
            printf_s("[HotReload] ⚠️ fread mismatch for %s (expected %zu, got %zu)\n", path.c_str(), len, bytesRead);
            continue;
        }

        FXIncludeHandler includeHandler;
        ID3DXEffect* rawFx = nullptr;
        LPD3DXBUFFER errors = nullptr;

        HRESULT hr = D3DXCreateEffect(
            GetGameDevice(), buffer.data(), (UINT)len,
            nullptr, &includeHandler,
            D3DXSHADER_DEBUG, nullptr, &rawFx, &errors);

        std::shared_ptr<FxWrapper> fx = nullptr;
        if (SUCCEEDED(hr) && rawFx)
        {
            fx = std::make_shared<FxWrapper>(rawFx);
            rawFx->Release();
        }

        if (FAILED(hr) || !fx || !IsValidShaderPointer(fx))
        {
            printf_s("[HotReload] ❌ Invalid fx pointer after load (hr=0x%08X)\n", hr);
            if (errors)
            {
                printf_s("[HotReload] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        fx->AddRef();
        g_ActiveEffects[key] = fx;

        if (key == "IDI_VISUALTREATMENT_FX")
        {
            g_LastReloadedFx = fx;
            printf_s("[HotReload] ✅ Set g_LastReloadedFx = %p (%s)\n", fx.get(), key.c_str());
        }

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

    return patchedAny;
}

// -------------------- HOOK HANDLER --------------------


void DumpShaderTable()
{
    printf_s("------ Shader Table Dump ------\n");
    for (int i = 0; i < ShaderTableSize; ++i)
    {
        std::shared_ptr<FxWrapper> fx = g_ShaderTable[i];
        if (!fx || !IsValidShaderPointer(fx))
            printf_s("[Check] Slot %02d = INVALID (%p)\n", i, fx);
        else
            printf_s("[Check] Slot %02d = VALID   (%p)\n", i, fx);
    }
}

void ApplyQueuedShaderPatches()
{
    std::lock_guard<std::mutex> lock(g_PatchMutex);
    static std::shared_ptr<FxWrapper>* table = (std::shared_ptr<FxWrapper>*)SHADER_TABLE_ADDRESS;

    std::vector<QueuedPatch> stillPending;

    for (auto& patch : g_PendingPatches)
    {
        if (--patch.framesRemaining > 0)
        {
            stillPending.push_back(patch);
            continue;
        }

        // If the patch.slot is invalid or default, try to recover it from our map
        if ((patch.slot < 0 || patch.slot >= ARRAYSIZE(g_SlotRetainedFx)) && !patch.resourceName.empty())
        {
            auto it = g_DynamicFallbackSlots.find(patch.resourceName);
            if (it != g_DynamicFallbackSlots.end())
            {
                patch.slot = it->second;
                printf_s("[Patch] 🔁 Using dynamic fallback slot %d for %s\n", patch.slot, patch.resourceName.c_str());
            }
            else
            {
                printf_s("[Patch] ❌ No fallback slot registered for %s — skipping\n", patch.resourceName.c_str());
                continue;
            }
        }
        
        if (!IsValidShaderPointer(patch.fx))
        {
            printf_s("[Patch] ❌ fx invalid for slot %d, fx=%p — skipping\n", patch.slot, patch.fx.get());

            if (!patch.fx || patch.fx.get() == reinterpret_cast<FxWrapper*>(0xCCCCCCCC))
                printf_s("[Patch] ❌ fx == 0xCCCCCCCC — uninitialized stack pointer\n");

            continue;
        }

        std::shared_ptr<FxWrapper> clone = nullptr;
        HRESULT hr = patch.fx->CloneEffect(GetGameDevice(), &clone);

        if (FAILED(hr) || !clone || !IsValidShaderPointer(clone))
        {
            printf_s("[Patch] ❌ CloneEffect failed for fx=%p, slot=%d (hr=0x%08X)\n", patch.fx.get(), patch.slot,
                     (unsigned)hr);
            continue;
        }

        // Diagnostic: get original refcount
        ULONG refCount = 0;
        if (IsValidShaderPointer(patch.fx))
        {
            refCount = patch.fx->AddRef();
            patch.fx->Release(); // Net 0
        }

        // Retain COM pointer for engine (manual)
        if (ID3DXEffect* effect = clone->GetEffect())
            effect->AddRef();

        // Store in shader table (shared_ptr handles FxWrapper refcount)
        table[patch.slot] = clone;

        // Clean up retained previous copy
        // Overwrite only if you're assigning a new shader
        if (g_SlotRetainedFx[patch.slot] != patch.fx)
        {
            if (g_SlotRetainedFx[patch.slot])
                g_SlotRetainedFx[patch.slot].reset(); // only clear if it's different

            g_SlotRetainedFx[patch.slot] = patch.fx;
            patch.fx->AddRef(); // if needed
        }

        g_SlotRetainedFx[patch.slot] = clone; // copy shared_ptr

        clone->OnResetDevice(); // rebind

        printf_s("[Patch] ✅ Cloned and wrote fx to slot %d (clone=%p, from=%p, refcount~%lu)\n",
                 patch.slot, clone.get(), patch.fx.get(), refCount);
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
            void** vtable = *(void***)fx->GetEffect();
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

// Hooked D3DXCreateEffectFromResourceA
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
    SetGameDevice(device); // ✅ Set global pointer

    static bool loadedOnce = false;
    if (!loadedOnce)
    {
        printf_s("[ShaderLoader] 🔍 Calling LoadShaderOverrides()\n");
        LoadShaderOverrides(); // ✅ This scans fx/*.fx and compiles them to IDI_*.FX
        CompileShaderOverrides(); // ✅ This will now work correctly
        compiled = RecompileAndReloadAll();
        loadedOnce = true;
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
                std::shared_ptr<FxWrapper> wrapper = std::make_shared<FxWrapper>(*outEffect);
                TrackLiveEffect(pResource, wrapper); // make sure pResource is IDI_XXX_FX

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
            HRESULT hr = D3DXCreateEffect(device, buffer.data(), (UINT)len, defines, &includeHandler, flags, pool,
                                          outEffect, outErrors);
            if (SUCCEEDED(hr) && *outEffect)
            {
                printf_s("[Hook] ✅ Loaded fallback compiled shader: %s\n", pResource);
                return S_OK;
            }

            printf_s("[Hook] ❌ Failed to load fallback compiled shader: %s\n", pResource);
        }
    }

    // 🧪 Fall back to original
    return RealCreateFromResource(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

bool SafePatchShaderTable(int slot, std::shared_ptr<FxWrapper> fx, const std::string& resourceName = "<unknown>")
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
        g_PendingPatches.push_back({slot, fx, 2, resourceName});
    }

    printf_s("[Patch] ⏳ Deferred shader patch queued: slot %d → fx=%p (%s)\n",
             slot, fx.get(), resourceName.c_str());
    return true;
}

bool __stdcall TryReloadFxRaw(FxWrapper* fx)
{
    if (!fx) return false;

    __try
    {
        fx->OnResetDevice();
        fx->ReloadHandles();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }

    return true;
}

bool SafeReloadFx(const std::shared_ptr<FxWrapper>& fx, const char* context)
{
    if (!fx || !fx->GetEffect())
    {
        printf_s("[HotReload:%s] ❌ Fx or effect is null\n", context);
        return false;
    }

    if (!TryReloadFxRaw(fx.get()))
    {
        printf_s("[HotReload:%s] ❌ Exception during ReloadHandles\n", context);
        return false;
    }

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

    std::string baseLower = base;
    std::transform(baseLower.begin(), baseLower.end(), baseLower.begin(), ::tolower);
    printf_s("[Lookup] 🔍 Searching for shader slots matching: %s (from %s)\n", baseLower.c_str(),
             resourceName.c_str());

    for (int i = 0; i < ShaderTableSize; ++i)
    {
        const char* entry = SafeGetShaderTableEntry(i);
        if (!entry || !IsSafeReadableString(entry))
            continue;

        std::string entryName(entry);
        size_t dot = entryName.find('.');
        if (dot != std::string::npos)
            entryName = entryName.substr(0, dot); // strip .fx or .hlsl extension if present

        std::string entryLower = entryName;
        std::transform(entryLower.begin(), entryLower.end(), entryLower.begin(), ::tolower);

        if (entryLower == baseLower)
        {
            printf_s("[Lookup] ✅ Match found at slot %d: %s\n", i, entry);
            results.push_back(i);
        }
        else
        {
            printf_s("[Lookup] — Skipping non-match at slot %d: %s\n", i, entry);
        }
    }

    if (results.empty())
        printf_s("[Lookup] ⚠️ No matching shader slots found for %s\n", resourceName.c_str());

    return results;
}

const char* TryGetFxNameAnnotation(FxWrapper* fx)
{
    __try
    {
        if (!fx || !fx->GetEffect()) return nullptr;

        D3DXHANDLE dummy = fx->GetEffect()->GetParameterByName(nullptr, nullptr);
        if (!dummy) return nullptr;

        D3DXHANDLE nameHandle = fx->GetEffect()->GetAnnotationByName(dummy, "vt");
        if (!nameHandle) return nullptr;

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

void __declspec(noinline) __stdcall SafeResetVisualTreatment(void* vt)
{
    __try
    {
        IVisualTreatment_Reset(vt);
        printf_s("[HotReload] ✅ SafeResetVisualTreatment: Reset() succeeded\n");
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf_s("[HotReload] ❌ SafeResetVisualTreatment: Reset() threw an exception\n");
    }
}

void VisualTreatment_Reset()
{
    if (g_pVisualTreatment && IsValidThis(*g_pVisualTreatment))
    {
        void* vt = *g_pVisualTreatment;

        // ⚠️ force invalidate ptr+0x140 to trigger rebuild
        void** fx140 = (void**)((char*)vt + 0x140);
        *fx140 = nullptr;

        std::shared_ptr<FxWrapper>* fxSlot = (std::shared_ptr<FxWrapper>*)((char*)vt + 0x18C);
        if (IsValidShaderPointer(*fxSlot))
        {
            printf_s("🔧 Releasing fx at +0x18C (%p)\n", *fxSlot);
            (*fxSlot)->Release();
            *fxSlot = nullptr;
        }

        // 🧪 Optional: Add null check if unsure about vt
        if (vt)
        {
            IVisualTreatment_Reset(vt);
            printf_s("[HotReload] ✅ Reset() called on IVisualTreatment at %p\n", vt);
        }
        else
        {
            printf_s("[HotReload] ❌ vt was null in VisualTreatment_Reset()\n");
        }
    }
    else
    {
        printf_s("[HotReload] ❌ g_pVisualTreatment invalid or null\n");
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

    FxWrapper* raw = reinterpret_cast<FxWrapper*>(sub); // ✅ safe raw ptr
    std::shared_ptr<FxWrapper> tempShared(raw); // risky if raw is already managed!
    if (IsValidShaderPointer(tempShared))
    {
        printf_s("✅ SubScan at +0x18C looks valid, storing as vtObject[0]\n");
        g_ThisCandidates[0] = raw; // ✅ use this in fallback
        g_ThisCount = 1;
    }
    else
    {
        printf_s("❌ SubScan result is not a valid shader-like object\n");
    }
}

void* GetFirstIVisualTreatmentObject()
{
    if (!g_ThisCandidates[0])
        ScanIVisualTreatment();

    return g_ThisCandidates[0];
}

void* GetIVisualTreatmentObject(void* vtObject)
{
    if (!vtObject || IsBadReadPtr(vtObject, sizeof(void*)))
        return nullptr;

    return *(void**)vtObject;
}

int FindAvailableFxSlot()
{
    constexpr int totalSlots = ARRAYSIZE(g_SlotRetainedFx);
    for (int i = 0; i < totalSlots; ++i)
    {
        if (!g_SlotRetainedFx[i]) // unused slot
            return i;
    }

    // If no free slots, return -1 (signal error)
    return -1;
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

    // Optional: Check if +0x30 is likely a valid std::shared_ptr<FxWrapper>
    void* fxCandidate = *(void**)((char*)candidate + 0x30);

    FxWrapper* raw = reinterpret_cast<FxWrapper*>(fxCandidate); // ✅ safe raw ptr
    std::shared_ptr<FxWrapper> tempShared(raw); // risky if raw is already managed!
    if ((uintptr_t)fxCandidate > 0x10000 && IsValidShaderPointer(tempShared))
    {
        printf_s("🔍 [IsLikelyApplyGraphicsSettingsObject] Slot +0x30 is valid shader pointer: %p\n", fxCandidate);
    }
    else
    {
        printf_s("🔍 [IsLikelyApplyGraphicsSettingsObject] Slot +0x30 not used or invalid\n");
    }

    return true;
}

bool ReplaceShaderSlot_RawEffect(
    BYTE* baseObject,
    int offset,
    ID3DXEffect* newRawFx,
    int slotIndex)
{
    void* slotPtr = (void*)(baseObject + offset);
    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery(slotPtr, &mbi, sizeof(mbi)) ||
        !(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)) ||
        mbi.State != MEM_COMMIT)
    {
        printf_s("[ReplaceShaderSlot] ❌ Memory at %p (+0x%X) not writable or committed\n",
                 slotPtr, offset);
        return false;
    }

    if (slotIndex >= 0 && slotIndex < ARRAYSIZE(g_SlotRetainedFx))
    {
        if (g_SlotRetainedFx[slotIndex] &&
            g_SlotRetainedFx[slotIndex].get()->GetEffect() != newRawFx)
        {
            g_SlotRetainedFx[slotIndex].reset();  // release old
        }
    }
    else
    {
        printf_s("[ReplaceShaderSlot] ⚠️ slotIndex %d out of range\n", slotIndex);
        return false;
    }

    if (newRawFx) newRawFx->AddRef();  // COM ref
    *reinterpret_cast<ID3DXEffect**>(slotPtr) = newRawFx;

    g_SlotRetainedFx[slotIndex] = g_LastReloadedFx;  // shared_ptr takes ownership

    printf_s("[ReplaceShaderSlot] ✅ Wrote new ID3DXEffect* (0x%p) into offset +0x%X (slot %d)\n",
             newRawFx, offset, slotIndex);
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

    void* scan = GetFirstIVisualTreatmentObject();
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

void ForceReplaceShaderIntoSlots(const std::string& resourceName, std::shared_ptr<FxWrapper> fxWrapper)
{
    g_LastReloadedFx = fxWrapper;

    ID3DXEffect* rawFx = fxWrapper->GetEffect(); // ✅ Not const
    const auto slotIndices = LookupShaderSlotsFromResource(resourceName);

    if (!slotIndices.empty())
    {
        for (int index : slotIndices)
        {
            BYTE* base = reinterpret_cast<BYTE*>((uintptr_t)SHADER_TABLE_ADDRESS);

            ReplaceShaderSlot_RawEffect(
                base,
                index * sizeof(ID3DXEffect*),
                rawFx,
                index);

            g_SlotRetainedFx[index] = fxWrapper;
            fxWrapper->AddRef();
        }
    }
    else
    {
        printf_s("[HotReload] ⚠️ No shader table slot for %s — falling back\n", resourceName.c_str());

        for (void* vtObj : g_ThisCandidates)
        {
            if (!IsLikelyApplyGraphicsSettingsObject(vtObj))
                continue;

            BYTE* base = reinterpret_cast<BYTE*>(GetIVisualTreatmentObject(vtObj));
            if (!base)
                continue;

            int fallbackIndex = FindAvailableFxSlot();
            if (fallbackIndex < 0)
            {
                printf_s("[Fallback] ❌ No available slot for vtObject = %p\n", vtObj);
                continue;
            }

            g_DynamicFallbackSlots[resourceName] = fallbackIndex;

            ReplaceShaderSlot_RawEffect(
                base,
                0x18C,
                rawFx,
                fallbackIndex);

            g_SlotRetainedFx[fallbackIndex] = fxWrapper;
            fxWrapper->AddRef();

            printf_s("[Fallback] ✅ Patched vtObject %p at +0x18C using slotIndex = %d\n",
                     vtObj, fallbackIndex);

            // ⏺ Queue patch for ApplyQueuedShaderPatches
            g_PendingPatches.push_back({
                fallbackIndex,
                fxWrapper,
                2,
                resourceName
            });
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
    if (!target || !IsValidThis(target))
        return;

    BYTE* base = static_cast<BYTE*>(target);

    std::shared_ptr<FxWrapper> currentFx = nullptr;
    std::shared_ptr<FxWrapper>* currentFxPtr = reinterpret_cast<std::shared_ptr<FxWrapper>*>(CURRENTSHADER_OBJ_ADDR);
    if (!IsBadReadPtr(currentFxPtr, sizeof(*currentFxPtr)) && IsValidShaderPointer(*currentFxPtr))
        currentFx = *currentFxPtr;

    printf_s("🧪 Scanning shader pointer slots (base = %p)...\n", base);

    for (int offset = 0; offset < 0x100; offset += 4)
    {
        auto fxSlot = reinterpret_cast<std::shared_ptr<FxWrapper>*>(base + offset);
        if (IsBadReadPtr(fxSlot, sizeof(*fxSlot)))
            continue;

        std::shared_ptr<FxWrapper> fx = *fxSlot;
        if (!fx || !IsValidShaderPointer(fx))
            continue;

        ID3DXEffect* effect = fx->GetEffect();
        if (!effect || reinterpret_cast<uintptr_t>(effect) < 0x10000)
            continue;

        D3DXEFFECT_DESC d = {};
        HRESULT hr = effect->GetDesc(&d);
        if (FAILED(hr))
        {
            printf_s("⚠️ GetDesc failed at offset +0x%02X (fx=%p)\n", offset, fx.get());
            continue;
        }

        if (fx == g_LastReloadedFx)
        {
            printf_s("🎯 g_LastReloadedFx is at +0x%02X → %p (%s)\n", offset, fx.get(), d.Creator);
        }
        else if (fx == currentFx)
        {
            printf_s("🌀 Current shader (CURRENTSHADER_OBJ_ADDR) at +0x%02X → %p (%s)\n", offset, fx.get(), d.Creator);
        }
        else if (offset == 0x18C)
        {
            printf_s("🔷 g_pVisualTreatment +0x18C slot → %p (%s)\n", fx.get(), d.Creator);
        }
        else
        {
            printf_s("🔍 Found valid shader at +0x%02X → %p (%s)\n", offset, fx.get(), d.Creator);
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

    PrintFxAtOffsets(targetThis); // diagnostics

    if (g_LastReloadedFx && IsValidShaderPointer(g_LastReloadedFx))
    {
        printf_s("🧪 Entering TryApplyGraphicsSettingsSafely() — g_LastReloadedFx=%p\n", g_LastReloadedFx.get());

        // 📌 Patch known shader pointer slot
        auto currentShader = reinterpret_cast<std::shared_ptr<FxWrapper>*>(CURRENTSHADER_OBJ_ADDR);
        if (IsBadReadPtr(currentShader, sizeof(std::shared_ptr<FxWrapper>)))
        {
            printf_s("⚠️ Cannot access CURRENTSHADER_OBJ_ADDR\n");
        }
        else if (*currentShader != g_LastReloadedFx)
        {
            printf_s("🎯 Overwriting CURRENTSHADER_OBJ_ADDR (%p → %p)\n", currentShader->get(), g_LastReloadedFx.get());
            *currentShader = g_LastReloadedFx;
        }
        else
        {
            printf_s("✅ g_LastReloadedFx already set in CURRENTSHADER_OBJ_ADDR\n");
        }

        // 📌 Patch known visual treatment slot at +0x18C
        if (!IsBadWritePtr((BYTE*)targetThis + 0x18C, sizeof(FxWrapper*)) &&
            g_LastReloadedFx && g_LastReloadedFx->IsValid())
        {
            SafePatchShaderTable(0x18C / 4, g_LastReloadedFx);
        }
        else
        {
            printf_s("❌ Cannot patch slot +0x18C — pointer invalid or g_LastReloadedFx is not valid\n");
        }

        // 📦 Fallback: scan memory for matching shared_ptr<FxWrapper> instances
        BYTE* obj = reinterpret_cast<BYTE*>(targetThis);

        D3DXEFFECT_DESC newDesc = {};
        if (FAILED(g_LastReloadedFx->GetEffect()->GetDesc(&newDesc)))
        {
            printf_s("❌ Failed to get desc for g_LastReloadedFx\n");
            return false;
        }

        for (int offset = 0; offset < 0x100; offset += 4)
        {
            auto fxSlot = reinterpret_cast<std::shared_ptr<FxWrapper>*>(obj + offset);

            if (IsBadReadPtr(fxSlot, sizeof(std::shared_ptr<FxWrapper>)))
                continue;

            std::shared_ptr<FxWrapper> fx = *fxSlot;
            if (!fx || !IsValidShaderPointer(fx))
                continue;

            if (fx == g_LastReloadedFx)
            {
                if (!IsBadWritePtr((BYTE*)obj + offset, sizeof(FxWrapper*)))
                {
                    SafePatchShaderTable(offset / 4, g_LastReloadedFx);
                    if (!g_LastReloadedFx || !g_LastReloadedFx->IsValid())
                    {
                        printf_s("[Error] ❌ g_LastReloadedFx is invalid before ReloadHandles\n");
                        continue;
                    }
                    g_LastReloadedFx->ReloadHandles();
                }
                else
                {
                    printf_s("❌ Cannot write to +0x%02X — skipping ReplaceShaderSlot\n", offset);
                }
                continue;
            }

            ID3DXEffect* fxEffect = fx->GetEffect();
            if (!fxEffect)
                continue;

            D3DXEFFECT_DESC desc = {};
            if (SUCCEEDED(fxEffect->GetDesc(&desc)) &&
                !strcmp(desc.Creator, newDesc.Creator) &&
                desc.Parameters == newDesc.Parameters &&
                desc.Techniques == newDesc.Techniques)
            {
                printf_s("🔶 Fuzzy match at +0x%02X (%p) — replacing\n", offset, fx.get());

                void* slotPtr = (BYTE*)obj + offset;
                if (!IsBadWritePtr(slotPtr, sizeof(FxWrapper*)))
                {
                    SafePatchShaderTable(offset / 4, g_LastReloadedFx);
                }
                else
                {
                    printf_s("❌ Slot at +0x%02X not writable — skipping patch\n", offset);
                }
            }
        }
    }

    // ✅ Call ApplyGraphicsSettingsOriginal
    printf_s("🎯 Calling ApplyGraphicsSettingsOriginal(manager=%p, object=%p)\n",
             g_ApplyGraphicsManagerThis, targetThis);

    if (IsValidThis(targetThis))
    {
        ApplyGraphicsSettingsOriginal(g_ApplyGraphicsManagerThis, nullptr, targetThis);
    }
    else
    {
        printf_s("❌ Invalid targetThis during ApplyGraphicsSettingsOriginal call\n");
        return false;
    }

    g_ThisCount = 0;
    g_TriggerApplyGraphicsSettings = false;
    return true;
}

bool TryPatchSlotIfWritable(void* obj, size_t offset, std::shared_ptr<FxWrapper> fx)
{
    if (!obj || !fx || !IsValidShaderPointer(fx)) return false;

    if (IsBadWritePtr(reinterpret_cast<BYTE*>(obj) + offset, sizeof(FxWrapper*)))
    {
        printf_s("[Patch] ❌ Cannot write to offset +0x%X at %p — skipping\n", (unsigned)offset, obj);
        return false;
    }

    // ReplaceShaderSlot(reinterpret_cast<BYTE*>(obj), offset, fx);
    ReplaceShaderSlot_RawEffect(
        reinterpret_cast<BYTE*>(*g_pVisualTreatment),
        0x18C,
        fx->GetEffect(),   // raw ID3DXEffect*
        62                 // store/release the wrapper at g_SlotRetainedFx[62]
    );

    if (offset == 0x18C)
    {
        g_SlotRetainedFx[62] = fx;
        g_SlotRetainedFx[62]->AddRef(); // 🔒 Retain
        printf_s("[Patch] ✅ Retained new FxWrapper in g_SlotRetainedFx[62]\n");
    }

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

            if (!g_LastReloadedFx->GetEffect() || reinterpret_cast<uintptr_t>(g_LastReloadedFx.get()) < 0x10000)
            {
                printf_s("[HotReload] ❌ g_LastReloadedFx is invalid or corrupted — skipping\n");
                return;
            }

            // ✅ Force-slot into g_SlotRetainedFx[62] before any use
            g_SlotRetainedFx[62] = g_LastReloadedFx;
            g_SlotRetainedFx[62]->AddRef(); // Retain explicitly

            printf_s("[Patch] ✅ g_SlotRetainedFx[62] initialized with g_LastReloadedFx = %p\n", g_LastReloadedFx.get());

            // 🔄 Attempt to patch vtObject +0x18C using the freshly retained shader
            TryPatchSlotIfWritable(vtObject, 0x18C, g_LastReloadedFx);

            // ✅ Reload handles safely
            if (SafeReloadFx(g_LastReloadedFx, "ApplyGraphicsSettings"))
            {
                if (g_pVisualTreatment && *g_pVisualTreatment)
                {
                    IVisualTreatment_Reset(*g_pVisualTreatment);
                    printf_s("[HotReload] 🔁 Called IVisualTreatment::Reset()\n");
                }
            }
            else
            {
                printf_s("[HotReload] ❌ ReloadHandles failed — skipping Reset\n");
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

        void* slotPtr = (BYTE*)applyThis + 0x18C;
        if (!IsBadWritePtr(slotPtr, sizeof(FxWrapper*)))
        {
            SafePatchShaderTable(62, g_LastReloadedFx); // 0x18C / 4
        }
        else
        {
            printf_s("[HotReload] ❌ Cannot write to applyThis + 0x18C — skipping patch\n");
        }
    }
    else if (g_pVisualTreatment && *g_pVisualTreatment)
    {
        BYTE* vtObj = (BYTE*)(*g_pVisualTreatment);
        printf_s("[Fallback] Patching g_pVisualTreatment at +0x18C (vtObj = %p)\n", vtObj);

        void* slotPtr = vtObj + 0x18C;
        if (!IsBadWritePtr(slotPtr, sizeof(FxWrapper*)))
        {
            SafePatchShaderTable(62, g_LastReloadedFx);
        }
        else
        {
            printf_s("[Fallback] ❌ Cannot write to g_pVisualTreatment + 0x18C — skipping patch\n");
        }
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
        try
        {
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

            std::shared_ptr<FxWrapper> fx = it->second;
            if (!fx || !fx->GetEffect())
            {
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
            if (!g_LastReloadedFx || !g_LastReloadedFx->IsValid())
            {
                printf_s("[Error] ❌ g_LastReloadedFx is invalid before ReloadHandles\n");
                return result;
            }
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
                printf_s("[ShaderManager] ❌ Failed to inject into g_LastReloadedFx (hr1=0x%08X, hr2=0x%08X)\n", hr1,
                         hr2);
            }
        }

        result = true;
    }
    while (false);

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
        std::shared_ptr<FxWrapper> effect = fx;
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
        std::shared_ptr<FxWrapper> fx = g_LastReloadedFx;
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
                if (!g_LastReloadedFx || !g_LastReloadedFx->IsValid())
                {
                    printf_s("[Error] ❌ g_LastReloadedFx is invalid before ReloadHandles\n");
                    return false;
                }
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

void ApplyShaderAndResetAll()
{
    ID3DXEffect* newFx = g_LastReloadedFx ? g_LastReloadedFx->GetEffect() : nullptr;
    if (!newFx) return;

    for (int i = 0; i < g_ThisCount; ++i)
    {
        void* obj = g_ThisCandidates[i];
        if (!IsValidThis(obj)) continue;

        ID3DXEffect** fxSlot = reinterpret_cast<ID3DXEffect**>(reinterpret_cast<BYTE*>(obj) + 0x18C);
        if (IsBadReadPtr(fxSlot, sizeof(ID3DXEffect*))) continue;

        ID3DXEffect* currentFx = *fxSlot;
        if (currentFx == newFx)
        {
            printf_s("[HotReload] ⚠️ Slot +0x18C already set to fx = %p — skipping Reset\n", newFx);
            continue;
        }
        printf_s("[HotReload] 🔁 Applying shader and reset for vtObject = %p\n", obj);

        // Validate the pointer before writing
        void* slotPtr = (BYTE*)obj + 0x18C;
        if (!IsBadWritePtr(slotPtr, sizeof(FxWrapper*)))
        {
            SafePatchShaderTable(62, g_LastReloadedFx); // 0x18C / 4 = 62
            if (!g_LastReloadedFx || !g_LastReloadedFx->IsValid())
            {
                printf_s("[Error] ❌ g_LastReloadedFx is invalid before ReloadHandles\n");
                return;
            }
            g_LastReloadedFx->ReloadHandles();
        }
        else
        {
            printf_s("[HotReload] ❌ Cannot write to obj + 0x18C — skipping patch\n");
        }

        if (IsValidThis(obj))
        {
            IVisualTreatment_Reset(obj);
            printf_s("[HotReload] 🔁 Called IVisualTreatment::Reset()\n");
        }
        else
        {
            printf_s("[HotReload] ❌ Invalid IVisualTreatment object for Reset(%p)\n", obj);
        }
    }
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

                ApplyShaderAndResetAll();

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

                if (!IsBadWritePtr((BYTE*)applyThis + 0x18C, sizeof(FxWrapper*)))
                {
                    g_LastReloadedFx->GetEffect()->OnLostDevice(); // Notify effect
                    SafePatchShaderTable(62, g_LastReloadedFx); // 0x18C / 4 = 62
                }
                else
                {
                    printf_s("[HotReload] ❌ Cannot write to applyThis + 0x18C — skipping patch\n");
                }
            }
            else if (g_pVisualTreatment && *g_pVisualTreatment)
            {
                BYTE* vtObj = (BYTE*)(*g_pVisualTreatment);
                void* slotPtr = vtObj + 0x18C;

                printf_s("[Fallback] Patching g_pVisualTreatment at +0x18C (vtObj = %p)\n", vtObj);

                if (!IsBadWritePtr(slotPtr, sizeof(FxWrapper*)))
                {
                    g_LastReloadedFx->GetEffect()->OnLostDevice();
                    SafePatchShaderTable(62, g_LastReloadedFx);
                }
                else
                {
                    printf_s("[Fallback] ❌ Cannot write to g_pVisualTreatment + 0x18C — skipping patch\n");
                }
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

    if (g_PendingVisualReset && g_pVisualTreatment && *g_pVisualTreatment)
    {
        void* vt = *g_pVisualTreatment;

        void** fx140 = (void**)((char*)vt + 0x140);
        *fx140 = nullptr;

        printf_s("[HotReload] 🔁 Deferred IVisualTreatment::Reset() triggered\n");
        SafeResetVisualTreatment(vt);

        g_PendingVisualReset = false;
    }

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

                // Convert to slot index: 0x18C / 4 = 62
                int slot = 62;

                void* slotPtr = obj + 0x18C;

                if (!IsBadWritePtr(slotPtr, sizeof(FxWrapper*)))
                {
                    SafePatchShaderTable(slot, g_LastReloadedFx);
                    if (!g_LastReloadedFx || !g_LastReloadedFx->IsValid())
                    {
                        printf_s("[Error] ❌ g_LastReloadedFx is invalid before ReloadHandles\n");
                        return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
                    }
                    g_LastReloadedFx->ReloadHandles();
                }
                else
                {
                    printf_s("[HotReload] ❌ Cannot write to obj + 0x18C — skipping SafePatchShaderTable\n");
                }
            }
        }
    }

    return ShaderManager::RealPresent(device, src, dest, hwnd, dirty);
}
