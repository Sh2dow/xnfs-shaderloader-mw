#include <atomic>
#include <windows.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include <mutex>
#include <unordered_set>
#include "includes/injector/injector.hpp"
#include "d3d9.h"
#include "d3dx9shader.h" // for ID3DXEffectCompiler
#include <psapi.h>

// -------------------- GLOBALS --------------------
namespace std
{
    class mutex;
}

LPDIRECT3DDEVICE9 g_Device = nullptr;

std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::unordered_map<std::string, ID3DXEffect*> g_CompiledFxThisReload;
std::unordered_set<std::string> g_FxOverrides;

std::string g_TriggerShaderReloadFor;
std::mutex g_TriggerShaderReloadMutex;
std::atomic<bool> g_PendingApplyGraphicsSettings = false;
std::atomic<bool> g_DeferredApplyRequested = false;
std::atomic<int> g_ApplyDelayCountdown = 0;
std::atomic<bool> g_PausePresent{false};
std::atomic<bool> g_PresentIsWaiting{false};
std::unordered_set<int> g_PendingShaderClearSlots;
std::mutex g_PendingShaderClearMutex;
constexpr int kShaderSlotCount = 64;
constexpr uintptr_t kApplyGraphicsSettingsRVA = 0x002D6000;
// Base address of the module (as loaded)
constexpr uintptr_t kImageBase = 0x00400000;
// Virtual Address of the shader table discovered in MW.exe
constexpr uintptr_t kShaderTableVA = 0x00AB1230;
constexpr uintptr_t kShaderTableRVA = 0x0054D9C4;
constexpr uintptr_t kShaderTableOffset = kShaderTableVA - kImageBase;

//─ compute and validate the shader-table pointer at runtime ─────────────
std::vector<std::pair<int, ID3DXEffect*>> g_PendingSlotUpdates;
std::mutex g_PendingSlotUpdateMutex;
static bool hasDoneInitialClear = false;
std::atomic<bool> g_HotReloadInProgress{false};

struct ShaderInfo
{
    std::string key;
    ID3DXEffect* effect;
};

struct QueuedPatch {
    int slot;
    ID3DXEffect* fx;
};
std::vector<QueuedPatch> g_PendingPatches;
std::mutex g_PatchMutex;

std::unordered_map<std::string, ShaderInfo> g_ActiveEffects;

std::unordered_map<std::string, std::vector<int>> g_ShaderSlotMap =
{
    {"IDI_WORLD_FX", {0, 31}},
    {"IDI_WORLDREFLECT_FX", {1, 32}},
    {"IDI_WORLDBONE_FX", {2, 33}},
    {"IDI_WORLDNORMALMAP_FX", {3, 34}},
    {"IDI_CAR_FX", {4, 35}},
    {"IDI_GLOSSYWINDOW_FX", {5, 36}},
    {"IDI_TREE_FX", {6, 37}},
    {"IDI_WORLDMIN_FX", {7, 38}},
    {"IDI_WORLDNOFOG_FX", {8, 39}},
    {"IDI_FE_FX", {9, 40}},
    {"IDI_FE_MASK_FX", {10, 41}},
    {"IDI_FILTER_FX", {11, 42}},
    {"IDI_OVERBRIGHT_FX", {12, 43}},
    {"IDI_SCREENFILTER_FX", {13, 44}},
    {"IDI_RAIN_DROP_FX", {14, 45}},
    {"IDI_RUNWAYLIGHT_FX", {15, 46}},
    {"IDI_VISUALTREATMENT_FX", {16, 47}},
    {"IDI_WORLDPRELIT_FX", {17, 48}},
    {"IDI_PARTICLES_FX", {18, 49}},
    {"IDI_SKYBOX_FX", {19, 50}},
    {"IDI_SHADOW_MAP_MESH_FX", {20, 51}},
    {"IDI_SKYBOX_CG_FX", {21, 52}},
    {"IDI_SHADOW_CG_FX", {22, 53}},
    {"IDI_CAR_SHADOW_MAP_FX", {23, 54}},
    {"IDI_WORLDDEPTH_FX", {24, 55}},
    {"IDI_WORLDNORMALMAPDEPTH_FX", {25, 56}},
    {"IDI_CARDEPTH_FX", {26, 57}},
    {"IDI_GLOSSYWINDOWDEPTH_FX", {27, 58}},
    {"IDI_TREEDEPTH_FX", {28, 59}},
    {"IDI_SHADOW_MAP_MESH_DEPTH_FX", {29, 60}},
    {"IDI_WORLDNORMALMAPNOFOG_FX", {30, 61}}
};

// ← replace 0x00F9B60 with the true RVA you discover in the MW exe
static ID3DXEffect** GetShaderTable()
{
    static ID3DXEffect** table = nullptr;
    if (!table)
    {
        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
        table = reinterpret_cast<ID3DXEffect**>(moduleBase + kShaderTableRVA);

        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(),
                             reinterpret_cast<HMODULE>(moduleBase),
                             &mi, sizeof(mi));
        uintptr_t start = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
        uintptr_t end = start + mi.SizeOfImage;
        printf("[Init] ShaderTable @ %p (module: %p–%p)\n",
               (void*)table, (void*)start, (void*)end);
        if (reinterpret_cast<uintptr_t>(table) < start ||
            reinterpret_cast<uintptr_t>(table) + sizeof(ID3DXEffect*) * kShaderSlotCount > end)
        {
            printf("[Error] ShaderTable is out of module bounds!\n");
        }
    }
    return table;
}

typedef HRESULT (STDMETHODCALLTYPE *PresentFn)(
    IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
PresentFn RealPresent = nullptr;

void ApplyQueuedShaderPatches()
{
    std::lock_guard<std::mutex> lock(g_PatchMutex);
    auto table = GetShaderTable();

    for (const auto& patch : g_PendingPatches)
    {
        table[patch.slot] = patch.fx;
        printf("[Patch] ✅ Applied deferred patch: slot %d → fx=%p\n", patch.slot, patch.fx);
    }

    g_PendingPatches.clear();
}

HRESULT STDMETHODCALLTYPE HookedPresent(
    IDirect3DDevice9* device,
    CONST RECT* src, CONST RECT* dest, HWND hwnd, CONST RGNDATA* dirty)
{
    ApplyQueuedShaderPatches();
    static bool hasLoggedPresent = false;
    if (!hasLoggedPresent)
    {
        printf("[Hook] Present() called.\n");
        hasLoggedPresent = true;
    }

    if (g_PausePresent)
    {
        g_PresentIsWaiting = true;
        while (g_PausePresent)
            Sleep(1);
        g_PresentIsWaiting = false;
    }

    return RealPresent(device, src, dest, hwnd, dirty);
}

void TryHookPresent()
{
    if (!g_Device) return;

    void** vtable = *(void***)g_Device;
    if (!vtable) return;

    DWORD oldProtect;
    VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    RealPresent = (PresentFn)vtable[17];
    vtable[17] = (void*)&HookedPresent;
    VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);

    printf("[Hook] IDirect3DDevice9::Present hooked successfully.\n");
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

std::vector<int> LookupShaderSlotsFromResource(const std::string& key)
{
    auto it = g_ShaderSlotMap.find(key);
    if (it == g_ShaderSlotMap.end())
    {
        printf("[Lookup] WARNING: Shader %s not mapped to any slots!\n", key.c_str());
        return {};
    }

    return it->second;
}

int hkReload = VK_F10; // optionally load from INI too
// -------------------- HELPERS --------------------

std::string ToUpper(const std::string& str)
{
    std::string out = str;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = (char)toupper(out[i]);
    return out;
}

bool IsValidEffect(ID3DXEffect* fx, int slot)
{
    __try
    {
        void* vtable = *(void**)fx;
        return vtable != nullptr;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("[Patch] ⚠️ fx is invalid (slot %d), rejecting\n", slot);
        return false;
    }
}

bool IsLikelyValidPointer(void* ptr)
{
    uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    return (p >= 0x10000 && p <= 0x7FFFFFFF);
}

bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx)
        return false;

    __try
    {
        // Basic check: access vtable and verify memory protection
        void* vtable = *(void**)fx;
        if (!vtable)
            return false;

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(vtable, &mbi, sizeof(mbi)) == 0)
            return false;

        if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
            return false;

        // ACTUAL runtime validity check: call AddRef + Release
        ULONG refCount = fx->AddRef();
        fx->Release();

        // Refcount sanity (optional)
        if (refCount > 1000000)
            return false;

        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

void SanityCheckShaderTable()
{
    auto table = GetShaderTable();
    for (int i = 32; i < kShaderSlotCount; ++i)
    {
        // ← INSERT HERE ↓
        if (i < 0 || i >= kShaderSlotCount)
        {
            printf("[Error] Out-of-bounds slot check: %d\n", i);
            continue;
        }
        ID3DXEffect* fx = table[i];
        void* vtable = fx ? *(void**)fx : nullptr;
        printf("[Debug] Sanity slot %d → fx=%p, vtable=%p\n", i, fx, vtable);
        // — end INSERT —

        DWORD old;
        if (VirtualProtect(&table[i], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &old))
        {
            ID3DXEffect* fx = table[i];
            if (fx && !IsValidShaderPointer(fx))
            {
                // only logging here, but if you ever clear you need protection too
                printf("[Sanity] Slot %d invalid, skipping clear\n", i);
            }
            VirtualProtect(&table[i], sizeof(ID3DXEffect*), old, &old);
        }
    }
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
        BYTE* buf = new(std::nothrow) BYTE[len];
        if (!buf)
        {
            fclose(f);
            return E_OUTOFMEMORY;
        }
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
    // 🔄 Clear existing state so we don't accumulate duplicates on reload
    g_ShaderOverridePaths.clear();
    g_ShaderBuffers.clear();
    g_FxOverrides.clear();

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
                                      0, nullptr, &fx, &errors);

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

    if (g_Device)
    {
        auto table = GetShaderTable();

        // Check if the table lies inside the module's memory range
        MEMORY_BASIC_INFORMATION mi{};
        if (VirtualQuery(table, &mi, sizeof(mi)) == 0)
        {
            printf("[Init] ❌ VirtualQuery failed on shader table\n");
            return;
        }

        uintptr_t start = reinterpret_cast<uintptr_t>(mi.BaseAddress);
        uintptr_t end = start + mi.RegionSize;
        uintptr_t tptr = reinterpret_cast<uintptr_t>(table);

        if (tptr < start || (tptr + sizeof(ID3DXEffect*) * kShaderSlotCount) > end)
        {
            printf("[Init] ❌ Shader table out of bounds — skipping clear\n");
        }
        else
        {
            DWORD oldProtect;
            if (VirtualProtect(table, sizeof(ID3DXEffect*) * kShaderSlotCount, PAGE_EXECUTE_READWRITE, &oldProtect))
            {
                for (int i = 0; i < kShaderSlotCount; ++i)
                {
                    if (!IsValidShaderPointer(table[i]))
                    {
                        table[i] = nullptr;
                        printf("[Init] ⚠️ Slot %d cleared — invalid shader pointer\n", i);
                    }
                    else
                    {
                        printf("[Init] ✅ Slot %d preserved — valid shader %p\n", i, table[i]);
                    }
                }

                VirtualProtect(table, sizeof(ID3DXEffect*) * kShaderSlotCount, oldProtect, &oldProtect);
                printf("[Init] ✅ Cleared shader table slots to prevent use of garbage pointers\n");
            }
            else
            {
                printf("[Init] ❌ VirtualProtect failed, cannot clear shader table safely\n");
            }
        }
    }
}

// -------------------- HOOK HANDLER --------------------
void (__cdecl* GetApplyGraphicsSettings())()
{
    auto base = (uintptr_t)GetModuleHandle(nullptr);
    return reinterpret_cast<void(__cdecl*)()>(base + kApplyGraphicsSettingsRVA);
}

bool SafePatchShaderTable(int slot, ID3DXEffect* fx)
{
    if (slot < 0 || slot >= 64 || !IsValidShaderPointer(fx))
        return false;

    {
        std::lock_guard<std::mutex> lock(g_PatchMutex);
        fx->AddRef();  // 🔒 Retain for next frame
        g_PendingPatches.push_back({ slot, fx });
    }

    printf("[Patch] ⏳ Deferred shader patch queued: slot %d → fx=%p\n", slot, fx);
    return true;
}

void PatchAllInvalidMatchingFx(ID3DXEffect* replacementFx)
{
    auto table = GetShaderTable();
    if (!table || !replacementFx) return;

    for (int i = 0; i < kShaderSlotCount; ++i)
    {
        // ← INSERT HERE ↓
        if (i < 0 || i >= kShaderSlotCount)
        {
            printf("[Error] Out-of-bounds patch slot: %d\n", i);
            continue;
        }
        ID3DXEffect* fx = table[i];
        void* vtable = fx ? *(void**)fx : nullptr;
        printf("[Debug] PatchAll slot %d → fx=%p, vtable=%p\n", i, fx, vtable);
        // — end INSERT —

        if (fx && !IsValidShaderPointer(fx))
            SafePatchShaderTable(i, replacementFx);
    }
}

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*, LPD3DXINCLUDE,
    DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);

D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

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
    if (!outEffect)
    {
        printf("[Hook] ERROR: outEffect is null\n");
        return E_POINTER;
    }

    *outEffect = nullptr;

    // Fast skip path during deferred reload
    if (g_TriggerShaderReloadFor.empty() &&
        g_FxOverrides.count(pResource) == 0 &&
        g_PendingApplyGraphicsSettings)
    {
        return RealCreateFromResource(device, hModule, pResource,
                                      defines, include, flags,
                                      pool, outEffect, outErrors);
    }

    printf("[Hook] Shader: %s | Override: %d | Reload: %d | Deferring: %d\n",
           pResource,
           (int)g_FxOverrides.count(pResource),
           !g_TriggerShaderReloadFor.empty(),
           (int)g_PendingApplyGraphicsSettings);

    // First-time init & delayed clear
    if (!g_Device)
    {
        g_Device = device;
        LoadShaderOverrides();

        auto table = GetShaderTable();
        if (table[16] && IsValidShaderPointer(table[16]))
        {
            hasDoneInitialClear = true;
            for (int i = 0; i < kShaderSlotCount; ++i)
            {
                if (!IsValidShaderPointer(table[i]))
                {
                    DWORD old;
                    VirtualProtect(&table[i], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &old);
                    table[i] = nullptr;
                    VirtualProtect(&table[i], sizeof(ID3DXEffect*), old, &old);
                }
            }
            printf("[Init] ✅ Cleared garbage-only slots after initial load\n");
        }
    }

    // Skip until initial clear complete, but still allow one-time overrides
    if (!hasDoneInitialClear)
    {
        if (g_FxOverrides.count(pResource) == 0)
        {
            return RealCreateFromResource(device, hModule, pResource,
                                          defines, include, flags,
                                          pool, outEffect, outErrors);
        }
    }

    // Reentrancy guard
    thread_local static bool insideHotReload = false;
    if (insideHotReload)
    {
        printf("[Hook] ⚠️ Reentrant call for %s — deferring to RealCreateFromResource\n", pResource);
        return RealCreateFromResource(device, hModule, pResource,
                                      defines, include, flags,
                                      pool, outEffect, outErrors);
    }

    // Hot-reload override path
    {
        std::lock_guard<std::mutex> lock(g_TriggerShaderReloadMutex);
        if (!g_TriggerShaderReloadFor.empty() &&
            strcmp(pResource, g_TriggerShaderReloadFor.c_str()) == 0)
        {
            const std::string shaderToReload = g_TriggerShaderReloadFor;
            g_TriggerShaderReloadFor.clear();
            insideHotReload = true;

            auto& src = g_ShaderBuffers[shaderToReload];
            if (src.empty())
            {
                printf("[Hotkey] ❌ Shader buffer for %s is empty!\n", shaderToReload.c_str());
                insideHotReload = false;
                g_HotReloadInProgress = false;
                return E_FAIL;
            }

            FXIncludeHandler incl;
            LPD3DXEFFECT fx = nullptr;

            // Compile or reuse
            if (g_CompiledFxThisReload.count(shaderToReload))
            {
                fx = g_CompiledFxThisReload[shaderToReload];
                if (!IsValidShaderPointer(fx))
                {
                    printf("[Hotkey] ❌ Reused shader pointer is invalid (%p), skipping\n", fx);
                    insideHotReload = false;
                    g_HotReloadInProgress = false;
                    return E_FAIL;
                }
                fx->AddRef();
                printf("[Hotkey] ✅ Reused compiled shader: %s (fx=%p)\n", shaderToReload.c_str(), fx);
            }
            else
            {
                // ------------------------------------------------------------------
                // Build or fetch the new effect via the SAME API the game uses
                // ------------------------------------------------------------------
                HRESULT hr = RealCreateFromResource(
                                device, hModule, pResource,
                                defines, include, flags,
                                pool, outEffect, outErrors);
                LPD3DXEFFECT fx = *outEffect;               // <-- the real effect pointer

                if (FAILED(hr) || !IsValidShaderPointer(fx))
                {
                    printf("[Hotkey] ❌ Effect load failed for %s (hr=0x%08X, fx=%p)\n",
                           shaderToReload.c_str(), hr, fx);
                    insideHotReload = false;
                    g_HotReloadInProgress = false;
                    return E_FAIL;
                }

                /* ---- cache bookkeeping (unchanged, but uses fx) ---- */
                if (g_ActiveEffects.count(shaderToReload))
                {
                    ID3DXEffect* old = g_ActiveEffects[shaderToReload].effect;
                    if (IsValidShaderPointer(old)) old->Release();
                }
                fx->AddRef();                                    // cache ref
                g_CompiledFxThisReload[shaderToReload] = fx;
                g_ActiveEffects[shaderToReload]     = {shaderToReload, fx};

                /* ---- patch every slot with the SAME pointer ---- */
                for (int slot : LookupShaderSlotsFromResource(shaderToReload))
                {
                    fx->AddRef();                                // one ref per slot
                    SafePatchShaderTable(slot, fx);
                }

                /* extra ref for the caller is already held in *outEffect */
                insideHotReload = false;
                g_HotReloadInProgress = false;
                return S_OK;
            }

            // Patch shader table directly with fx
            auto slots = LookupShaderSlotsFromResource(shaderToReload);
            for (int slot : slots)
            {
                fx->AddRef(); // for each slot
                if (SafePatchShaderTable(slot, fx))
                    printf("[Hotkey] ✅ Patched slot %d with fx=%p\n", slot, fx);
                else
                    printf("[Hotkey] ⚠️ Failed patch for slot %d\n", slot);
            }

            *outEffect = fx; // give caller the same shader
            insideHotReload = false;
            g_HotReloadInProgress = false;
            return S_OK;
        }
    }

    // One-time override path
    if (g_FxOverrides.count(pResource))
    {
        FXIncludeHandler incl;
        HRESULT hr = RealCreateFromResource(
                device, hModule, pResource,
                defines, include, flags,
                pool, outEffect, outErrors);
        LPD3DXEFFECT fx = *outEffect;                 // <-- real effect

        if (SUCCEEDED(hr) && IsValidShaderPointer(fx))
        {
            PauseGameThread();

            for (int slot : LookupShaderSlotsFromResource(pResource))
            {
                fx->AddRef();                          // one ref per slot
                SafePatchShaderTable(slot, fx);
            }

            /* caller already owns the *outEffect ref */
            ResumeGameThread();
            return S_OK;
        }
        else
        {
            printf("[Hook] ❌ Override failed for %s (hr=0x%08X, fx=%p)\n",
                   pResource, hr, fx);
            if (fx) fx->Release();
        }
    }

    // Fallback
    return RealCreateFromResource(device, hModule, pResource,
                                  defines, include, flags,
                                  pool, outEffect, outErrors);
}

void ReloadTrackedEffects()
{
    for (auto it = g_ActiveEffects.begin(); it != g_ActiveEffects.end(); ++it)
    {
        const std::string& key = it->first;
        ShaderInfo& shader = it->second;

        printf("[ReloadTrack] Recompiling and reloading: %s\n", key.c_str());

        if (shader.effect)
        {
            shader.effect->Release();
            shader.effect = nullptr;
        }

        LPD3DXEFFECT fx = nullptr;
        LPD3DXBUFFER err = nullptr;

        FXIncludeHandler includeHandler;
        HRESULT hr = D3DXCreateEffectFromResourceA(
            g_Device,
            GetModuleHandle(nullptr),
            key.c_str(),
            nullptr, // D3DXMACRO*
            &includeHandler, // ID3DXInclude*
            0, // Flags
            nullptr, // LPD3DXEFFECTPOOL
            &fx, // Output effect
            &err // Output error buffer
        );

        if (SUCCEEDED(hr))
        {
            shader.effect = fx;
            auto slots = LookupShaderSlotsFromResource(key);
            for (int index : slots)
            {
                if (index >= 0 && index < 64)
                {
                    GetShaderTable()[index] = fx;
                    printf("[ReloadTrack] ✅ Updated live shader slot %d\n", index);
                }
            }
            printf("[ReloadTrack] Reloaded effect for: %s\n", key.c_str());
        }
        else
        {
            printf("[ReloadTrack] Failed to reload: %s\n", key.c_str());
            if (err)
            {
                printf("[ReloadTrack] Error: %s\n", (char*)err->GetBufferPointer());
                err->Release();
            }
        }
    }
}

DWORD WINAPI TriggerApplyLater(LPVOID)
{
    {
        std::lock_guard<std::mutex> lk(g_PendingShaderClearMutex);
        const std::string key = "IDI_VISUALTREATMENT_FX";

        if (!g_ShaderBuffers.count(key) || g_ShaderBuffers[key].empty())
        {
            printf("[Reload] ❌ No shader buffer available for %s\n", key.c_str());
            return 0;
        }

        // Signal that the next game call to CreateEffectFromResource should trigger recompilation
        {
            std::lock_guard<std::mutex> lk2(g_TriggerShaderReloadMutex);
            g_TriggerShaderReloadFor = key;
        }

        // This will force the game to reinit shaders and hit the hook naturally
        Sleep(50);
        auto fn = GetApplyGraphicsSettings();
        if (fn)
        {
            printf("[Reload] 🔁 Calling ApplyGraphicsSettings to trigger reload...\n");
            fn();
        }
    }

    return 0;
}

DWORD WINAPI HotkeyThread(LPVOID)
{
    printf("[HotkeyThread] Waiting for F10...\n");

    while (true)
    {
        if (GetAsyncKeyState(VK_F10) & 1)
        {
            printf("[HotkeyThread] F10 pressed — hot-reloading visualtreatment.fx\n");

            LoadShaderOverrides();
            {
                std::lock_guard<std::mutex> lk(g_TriggerShaderReloadMutex);
                g_TriggerShaderReloadFor = "IDI_VISUALTREATMENT_FX";
            }

            // Signal start
            g_HotReloadInProgress = true;

            // Trigger the engine to recreate shaders
            if (auto fn = GetApplyGraphicsSettings())
                fn();

            // Now block here until the hook clears the flag
            const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
            while (g_HotReloadInProgress && std::chrono::steady_clock::now() < timeout)
                Sleep(1);

            if (g_HotReloadInProgress)
                printf("[HotkeyThread] ⚠️ Hot-reload timed out without patching!\n");
            else
                printf("[HotkeyThread] ✅ Hot-reload complete.\n");
        }

        Sleep(50);
    }

    return 0;
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

            // Rebase the hook address under ASLR
            uintptr_t moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
            constexpr uintptr_t hookRVA = 0x006C60D2 - 0x00400000; // VA - image base
            uintptr_t hookAddr = moduleBase + hookRVA;
            injector::MakeCALL(hookAddr, HookedCreateFromResource, true);
            printf("[Init] Hooked D3DXCreateEffectFromResourceA at 0x%p\n", (void*)hookAddr);
        }
        else
        {
            printf("[Error] d3dx9_43.dll not loaded!\n");
        }

        // ✅ Launch hotkey thread
        HANDLE hThread = CreateThread(nullptr, 0, HotkeyThread, nullptr, 0, nullptr);
        if (hThread)
            printf("[Init] Hotkey thread started successfully.\n");
        else
            printf("[Init] Failed to start hotkey thread! Error: %lu\n", GetLastError());
    }
    return TRUE;
}
