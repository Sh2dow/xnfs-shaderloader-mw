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
#include "Modules/minhook/include/MinHook.h"
#include "d3d9.h"
#include "d3dx9shader.h" // for ID3DXEffectCompiler
#include <psapi.h>

// -------------------- GLOBALS --------------------
namespace std
{
    class mutex;
}

int hkReload = VK_F10; // optionally load from INI too

LPDIRECT3DDEVICE9 g_Device = nullptr;

struct ShaderInfo
{
    std::string key;
    ID3DXEffect* effect;
};

std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_map<std::string, std::vector<uint8_t>> g_ShaderBuffers;
std::unordered_map<std::string, ID3DXEffect*> g_CompiledFxThisReload;
std::unordered_map<std::string, ShaderInfo> g_FxOverrides;
std::vector<LPD3DXEFFECT> g_TrackedFxClones;

std::string g_TriggerShaderReloadFor;
std::mutex g_TriggerShaderReloadMutex;
static std::atomic<bool> g_HotReloadInProgress{false};
static bool hasDoneInitialClear = false;

constexpr int kShaderSlotCount = 64;
// Base address of the module (as loaded)
constexpr uintptr_t kImageBase = 0x00400000;
// Virtual Address of the shader table discovered in MW.exe
constexpr uintptr_t kShaderTableVA = 0x00AB1230;
// Offset from image base to shader table
constexpr uintptr_t kShaderTableOffset = kShaderTableVA - kImageBase;

// ApplyGraphicsSettings RVA from module base
constexpr uintptr_t kApplyGraphicsSettingsRVA = 0x002D6000;

std::atomic<bool> g_PendingApplyGraphicsSettings = false;
std::atomic<bool> g_DeferredApplyRequested = false;
std::atomic<int> g_ApplyDelayCountdown = 0;
std::unordered_set<int> g_PendingShaderClearSlots;
std::mutex g_PendingShaderClearMutex;
constexpr uintptr_t kShaderTableRVA = 0x0054D9C4;
//─ compute and validate the shader-table pointer at runtime ─────────────
std::unordered_map<int, LPD3DXEFFECT> g_PendingFxTablePatches;
std::mutex g_PendingSlotUpdateMutex;
// ====== Global Flag ======
bool g_TriggerReloadNextFrame = false;

// ====== Original BeginScene Pointer ======
typedef HRESULT (WINAPI*BeginSceneFn)(IDirect3DDevice9*);
BeginSceneFn g_OriginalBeginScene = nullptr;

std::unordered_map<std::string, ShaderInfo> g_ActiveEffects;

std::unordered_map<std::string, std::vector<int>> g_ShaderSlotMap = {
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
        // compute the delta from the PE’s image base
        uintptr_t offset = kShaderTableRVA - kImageBase;
        table = reinterpret_cast<ID3DXEffect**>(moduleBase + offset);

        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(),
                             reinterpret_cast<HMODULE>(moduleBase),
                             &mi, sizeof(mi));
        uintptr_t start = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
        uintptr_t end = start + mi.SizeOfImage;
        printf("[Init] ShaderTable @ %p (module: %p–%p)\n",
               (void*)table, (void*)start, (void*)end);
        if (table < reinterpret_cast<ID3DXEffect**>(start) ||
            reinterpret_cast<uintptr_t>(table)
            + sizeof(ID3DXEffect*) * kShaderSlotCount > end)
        {
            printf("[Error] ShaderTable is out of module bounds!\n");
        }
    }
    return table;
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
        void* vtable = *(void**)fx;
        if (!vtable)
            return false;

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(vtable, &mbi, sizeof(mbi)) == 0)
            return false;

        DWORD protect = mbi.Protect;
        if (!(protect & PAGE_EXECUTE_READ) && !(protect & PAGE_EXECUTE_READWRITE) && !(protect & PAGE_EXECUTE))
            return false;

        // Skip AddRef / Release — too dangerous early
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
        if (i < 0 || i >= kShaderSlotCount)
        {
            printf("[Error] Out-of-bounds slot check: %d\n", i);
            continue;
        }

        ID3DXEffect* fx = table[i];
        DWORD old;
        if (VirtualProtect(&table[i], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &old))
        {
            if (fx)
            {
                if (!IsValidShaderPointer(fx))
                {
                    printf("[Sanity] Slot %d invalid, fx=%p — skipping clear\n", i, fx);
                }
                else
                {
                    void* vtable = *(void**)fx;
                    printf("[Debug] Sanity slot %d → fx=%p, vtable=%p\n", i, fx, vtable);
                }
            }
            else
            {
                printf("[Debug] Sanity slot %d is null\n", i);
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
void FinalizeInitialClearIfNeeded()
{
    if (!hasDoneInitialClear)
    {
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
                    printf("[Sanity] Cleared slot %d (invalid pointer)\n", i);
                }
            }
            printf("[Init] ✅ Cleared garbage-only slots after initial load\n");
        }
    }
}

void LoadShaderOverrides(bool skipClear = false)
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
        g_FxOverrides.insert({key, ShaderInfo{key, nullptr}}); // ✅ REQUIRED for HookedCreateFromResource

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
        g_ShaderBuffers[key] = std::vector<uint8_t>(data.begin(), data.end());
        // stores raw HLSL code
    }
    while (FindNextFileA(hFind, &findData));

    printf("[Debug] g_FxOverrides contains:\n");
    for (const auto& pair : g_FxOverrides)
        printf("  - %s\n", pair.first.c_str());

    FindClose(hFind);

    if (g_Device)
    {
        if (!skipClear && g_Device)
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
                    for (auto it = g_ShaderSlotMap.begin(); it != g_ShaderSlotMap.end(); ++it)
                    {
                        const std::string& key = it->first;
                        const std::vector<int>& slots = it->second;

                        for (int slot : slots)
                        {
                            if (!IsValidShaderPointer(table[slot]))
                            {
                                DWORD old;
                                VirtualProtect(&table[slot], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &old);
                                table[slot] = nullptr;
                                VirtualProtect(&table[slot], sizeof(ID3DXEffect*), old, &old);
                                printf("[Init] ⚠️ Slot %d cleared — invalid shader pointer\n", slot);
                            }
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

    FinalizeInitialClearIfNeeded();
    SanityCheckShaderTable();
}

// -------------------- HOOK HANDLER --------------------
void (__cdecl* GetApplyGraphicsSettings())()
{
    auto base = (uintptr_t)GetModuleHandle(nullptr);
    return reinterpret_cast<void(__cdecl*)()>(base + kApplyGraphicsSettingsRVA);
}

bool SafePatchShaderTable(int slot, ID3DXEffect* fx)
{
    if (!fx)
    {
        printf("[Patch] ❌ Null fx pointer for slot %d\n", slot);
        return false;
    }

    if (!IsValidShaderPointer(fx))
    {
        printf("[Patch] ❌ Invalid fx pointer for slot %d: %p\n", slot, fx);
        return false;
    }

    auto table = GetShaderTable();
    if (!table || slot < 0 || slot >= kShaderSlotCount)
    {
        printf("[Patch] ❌ Invalid slot index: %d\n", slot);
        return false;
    }

    DWORD oldProtect;
    if (!VirtualProtect(&table[slot], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf("[Patch] ❌ VirtualProtect failed for slot %d\n", slot);
        return false;
    }

    ID3DXEffect* existing = table[slot];

    if (existing)
    {
        if (existing == fx)
        {
            printf("[Patch] ℹ️ Slot %d already holds fx=%p\n", slot, fx);
        }
        else if (IsValidShaderPointer(existing))
        {
            printf("[Patch] 🔁 Releasing old fx in slot %d (%p)\n", slot, existing);
            existing->Release(); // Release only if valid
        }
        else
        {
            printf("[Patch] ⚠️ Existing fx in slot %d is garbage (%p), clearing slot\n", slot, existing);
            table[slot] = nullptr; // ⬅️ Critical fallback to prevent game from using garbage
        }
    }
    else
    {
        printf("[Patch] ℹ️ Slot %d was null, inserting new fx=%p\n", slot, fx);
    }

    // ✅ AddRef before storing to keep alive
    if (fx)
    {
        DWORD old;
        VirtualProtect(&table[slot], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &old);
        table[slot] = nullptr;
        VirtualProtect(&table[slot], sizeof(ID3DXEffect*), old, &old);
        table[slot] = fx;
        fx->AddRef();
    }

    DWORD dummyProtect;
    VirtualProtect(&table[slot], sizeof(ID3DXEffect*), oldProtect, &dummyProtect);

    printf("[Patch] ✅ Wrote shader to slot %d (%p)\n", slot, fx);
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

bool WriteShaderSlotSafe(int slot, ID3DXEffect* fx)
{
    ID3DXEffect** table = GetShaderTable();
    DWORD oldProtect;
    if (!VirtualProtect(&table[slot], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf("[Patch] ❌ VirtualProtect failed for slot %d\n", slot);
        return false;
    }

    table[slot] = fx;

    DWORD temp;
    VirtualProtect(&table[slot], sizeof(ID3DXEffect*), oldProtect, &temp);
    return true;
}

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*, LPD3DXINCLUDE,
    DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);

D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

// Helper function to safely patch a shader into one or more slots
bool PatchShaderIntoSlots(const std::string& shaderName,
                          const std::vector<uint8_t>& src,
                          const D3DXMACRO* defines,
                          LPD3DXINCLUDE incl,
                          DWORD flags,
                          LPD3DXEFFECTPOOL pool,
                          LPD3DXBUFFER* outErrors,
                          LPDIRECT3DDEVICE9 device,
                          LPD3DXEFFECT* optionalOut)
{
    auto slots = LookupShaderSlotsFromResource(shaderName);
    if (slots.empty())
    {
        printf("[Patch] ⚠️ No shader slots found for %s\n", shaderName.c_str());
        return false;
    }

    ID3DXEffect** table = GetShaderTable();
    if (!table)
    {
        printf("[Patch] ❌ Shader table is null!");
        return false;
    }

    for (int slot : slots)
    {
        if (slot < 0 || slot >= kShaderSlotCount)
        {
            printf("[Patch] ❌ Slot index %d out of bounds for %s\n", slot, shaderName.c_str());
            continue;
        }

        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(&table[slot], &mbi, sizeof(mbi)) || !(mbi.Protect & (PAGE_READWRITE |
            PAGE_EXECUTE_READWRITE)))
        {
            printf("[Patch] ❌ Slot %d points to non-writable memory for %s\n", slot, shaderName.c_str());
            continue;
        }

        LPD3DXEFFECT fxClone = nullptr;
        HRESULT hr = D3DXCreateEffect(device,
                                      src.data(), (UINT)src.size(),
                                      defines, incl,
                                      flags, pool,
                                      &fxClone, outErrors);

        if (!(SUCCEEDED(hr) && fxClone && IsValidShaderPointer(fxClone)))
        {
            printf("[Patch] ❌ Failed to compile FX for slot %d of %s\n", slot, shaderName.c_str());
            continue;
        }

        fxClone->AddRef(); // Track
        g_TrackedFxClones.push_back(fxClone);

        ID3DXEffect* oldFx = table[slot];

        // AddRef first if the shader will be used by the game
        fxClone->AddRef(); // for game use

        if (g_HotReloadInProgress)
        {
            fxClone->AddRef(); // AddRef BEFORE queuing, so map always owns a valid ref

            {
                std::lock_guard<std::mutex> lock(g_PendingSlotUpdateMutex);
                g_PendingFxTablePatches[slot] = fxClone;
            }

            printf("[Patch] ⏳ Queued patch of %s into slot %d (%p)\n", shaderName.c_str(), slot, fxClone);
        }
        else
        {
            if (!WriteShaderSlotSafe(slot, fxClone))
            {
                fxClone->Release(); // release game ref
                continue;
            }
        }

        // Do NOT call fxClone->SetTechnique(nullptr) during hot reloads

        if (optionalOut && *optionalOut == oldFx)
        {
            (*optionalOut)->Release();
            *optionalOut = fxClone;
            fxClone->AddRef();
        }

        printf("[Patch] ✅ Patched %s into slot %d (%p)\n", shaderName.c_str(), slot, fxClone);
    }

    return true;
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
    if (!outEffect)
    {
        printf("[Hook] ERROR: outEffect is null\n");
        return E_POINTER;
    }

    *outEffect = nullptr;

    // Trigger override on first visualtreatment.fx load
    if (g_PendingApplyGraphicsSettings &&
        strcmp(pResource, "IDI_VISUALTREATMENT_FX") == 0)
    {
        printf("[Init] 🟢 First call to visualtreatment.fx, applying override now\n");
        g_PendingApplyGraphicsSettings = false;
        g_TriggerShaderReloadFor = pResource;
        g_HotReloadInProgress = true; // So BeginScene does the safe patch
    }

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
                    printf("[Sanity] Cleared slot %d (invalid pointer)\n", i);
                }
            }
            printf("[Init] ✅ Cleared garbage-only slots after initial load\n");
        }
    }

    if (!hasDoneInitialClear && g_FxOverrides.count(pResource) == 0)
    {
        return RealCreateFromResource(device, hModule, pResource,
                                      defines, include, flags,
                                      pool, outEffect, outErrors);
    }

    thread_local static bool insideHotReload = false;
    if (insideHotReload)
    {
        printf("[Hook] ⚠️ Reentrant call for %s — deferring to RealCreateFromResource\n", pResource);
        return RealCreateFromResource(device, hModule, pResource,
                                      defines, include, flags,
                                      pool, outEffect, outErrors);
    }

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

            HRESULT hr = D3DXCreateEffect(g_Device,
                                          src.data(), (UINT)src.size(),
                                          defines, &incl,
                                          flags, pool,
                                          &fx, outErrors);

            std::vector<int> patchedSlots;

            if (SUCCEEDED(hr) && fx && IsValidShaderPointer(fx))
            {
                auto& src = g_ShaderBuffers[shaderToReload];
                if (!PatchShaderIntoSlots(shaderToReload, src, defines, &incl, flags, pool, outErrors, g_Device,
                                          outEffect))
                {
                    insideHotReload = false;
                    g_HotReloadInProgress = false;
                    return E_FAIL;
                }
            }
        }
    }

    if (g_FxOverrides.count(pResource))
    {
        auto iter = g_FxOverrides.find(pResource);
        if (iter != g_FxOverrides.end())
        {
            auto& fxOverride = iter->second;
            if (IsValidShaderPointer(fxOverride.effect))
            {
                printf("[Hook] ✅ Using cached compiled effect for %s: %p\n", pResource, fxOverride.effect);
                *outEffect = fxOverride.effect;
                (*outEffect)->AddRef();
                return S_OK;
            }
            else
            {
                printf("[Hook] ⚠️ Cached shader for %s was invalid (%p), recompiling...\n", pResource,
                       fxOverride.effect);
                fxOverride.effect = nullptr; // Safety: discard bad pointer
            }

            FXIncludeHandler incl;
            auto& src = g_ShaderBuffers[pResource];
            PatchShaderIntoSlots(pResource, src, defines, &incl, flags, pool, outErrors, device, outEffect);
        }
    }

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
            if (IsValidShaderPointer(shader.effect))
                shader.effect->Release();
            shader.effect = nullptr;
        }

        LPD3DXEFFECT fx = nullptr;
        LPD3DXBUFFER err = nullptr;

        FXIncludeHandler includeHandler;
        if (!g_Device)
        {
            printf("[ReloadTrack] ❌ g_Device is null — aborting reload.\n");
            return;
        }
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
            shader.effect = nullptr;
            if (IsValidShaderPointer(fx))
            {
                shader.effect = fx;
                fx->AddRef();
            }

            auto slots = LookupShaderSlotsFromResource(key);
            for (int index : slots)
            {
                if (index < 0 || index >= 64)
                    continue;

                ID3DXEffect** table = GetShaderTable();
                ID3DXEffect* existing = table[index];

                if (existing && IsValidShaderPointer(existing))
                {
                    void* vtable = *(void**)existing;
                    if (vtable && !IsBadCodePtr((FARPROC)vtable))
                    {
                        printf("[ReloadTrack] 🔁 Releasing old fx in slot %d (%p)\n", index, existing);
                        existing->Release();
                    }
                    else
                    {
                        printf("[ReloadTrack] ⚠️ Slot %d had garbage vtable (%p), skipping Release()\n", index, vtable);
                    }
                }

                table[index] = nullptr;

                if (!IsValidShaderPointer(fx))
                {
                    printf("[ReloadTrack] ❌ fx pointer invalid for %s\n", key.c_str());
                    continue;
                }

                // AddRef and defer slot update
                {
                    std::lock_guard<std::mutex> lock(g_PendingSlotUpdateMutex);
                    g_PendingFxTablePatches.emplace(index, fx);
                    fx->AddRef(); // Keep alive
                }

                printf("[ReloadTrack] ✅ Queued update for shader slot %d with fx=%p\n", index, fx);
            }

            printf("[ReloadTrack] Reloaded effect for: %s\n", key.c_str());
        }
    }
}

// ====== BeginScene Hook ======
void Hooked_BeginScene(IDirect3DDevice9* device)
{
    std::lock_guard<std::mutex> lock(g_PendingSlotUpdateMutex);
    for (auto& pair : g_PendingFxTablePatches)
    {
        int slot = pair.first;
        ID3DXEffect* fx = pair.second;
        if (fx && IsValidShaderPointer(fx))
        {
            if (WriteShaderSlotSafe(slot, fx))
            {
                ID3DXEffect** table = GetShaderTable();
                if (table)
                {
                    DWORD old;
                    VirtualProtect(&table[slot], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &old);
                    table[slot] = nullptr;
                    VirtualProtect(&table[slot], sizeof(ID3DXEffect*), old, &old);

                    fx->AddRef(); // for game's use
                    table[slot] = fx;
                }

                fx->OnResetDevice(); // safe to call now
                printf("[BeginScene] ✅ Patched slot %d with fx %p\n", slot, fx);
            }
            else
            {
                printf("[BeginScene] ❌ Failed to patch slot %d\n", slot);
            }

            fx->Release(); // balance AddRef done before queuing
        }
    }
    g_PendingFxTablePatches.clear();
    g_HotReloadInProgress = false;
}

// ====== Hook Installation ======
void HookBeginScene(IDirect3DDevice9* device)
{
    void** vtable = *reinterpret_cast<void***>(device);
    void* target = vtable[41]; // Index for BeginScene

    if (MH_CreateHook(target, &Hooked_BeginScene, reinterpret_cast<void**>(&g_OriginalBeginScene)) == MH_OK)
    {
        MH_EnableHook(target);
        printf("[Init] Hooked BeginScene\n");
    }
    else
    {
        printf("[Init] Failed to hook BeginScene\n");
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

            LoadShaderOverrides(true);

            {
                std::lock_guard<std::mutex> lk(g_TriggerShaderReloadMutex);
                g_TriggerShaderReloadFor = "IDI_VISUALTREATMENT_FX";
            }

            // Let the BeginScene hook pick this up safely
            g_HotReloadInProgress = true;
            auto fn = GetApplyGraphicsSettings();
            if (fn)
            {
                printf("[HotkeyThread] 🔁 Calling ApplyGraphicsSettings to trigger reload...\n");
                fn();
            }
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
            uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
            // VA of hook site discovered
            constexpr uintptr_t hookVA = 0x006C60D2;
            constexpr uintptr_t hookOffset = hookVA - kImageBase;
            injector::MakeCALL(base + hookOffset, HookedCreateFromResource, true);
            printf("[Init] Hooked D3DXCreateEffectFromResourceA at 0x%p\n", (void*)(base + hookOffset));

            // ✅ Defer shader patching until first ApplyGraphicsSettings
            g_PendingApplyGraphicsSettings = true;
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
