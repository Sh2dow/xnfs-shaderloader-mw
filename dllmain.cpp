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
std::unordered_set<std::string> g_FxOverrides;

std::string g_TriggerShaderReloadFor;
std::mutex g_TriggerShaderReloadMutex;
std::atomic<bool> g_PendingApplyGraphicsSettings = false;
std::atomic<bool> g_DeferredApplyRequested = false;
std::atomic<int> g_ApplyDelayCountdown = 0;
std::unordered_set<int> g_PendingShaderClearSlots;
std::mutex g_PendingShaderClearMutex;
constexpr int kShaderSlotCount = 64;
constexpr uintptr_t kShaderTableRVA = 0x0054D9C4;
constexpr uintptr_t kApplyGraphicsSettingsRVA = 0x002D6000;

struct ShaderInfo
{
    std::string key;
    ID3DXEffect* effect;
};

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

//─ compute and validate the shader-table pointer at runtime ─────────────
static constexpr uintptr_t g_ShaderTableRVA = 0x00F9B60;
// ← replace 0x00F9B60 with the true RVA you discover in the MW exe
static ID3DXEffect** GetShaderTable()
{
    static ID3DXEffect** table = nullptr;
    if (!table)
    {
        auto hMod = GetModuleHandle(nullptr);
        uintptr_t base = reinterpret_cast<uintptr_t>(hMod);
        table = reinterpret_cast<ID3DXEffect**>(base + g_ShaderTableRVA);

        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
        uintptr_t start = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
        uintptr_t end = start + mi.SizeOfImage;
        uintptr_t tptr = reinterpret_cast<uintptr_t>(table);
        printf("[Init] ShaderTable @ 0x%08X (mod 0x%08X–0x%08X)\n", (unsigned)tptr, (unsigned)start, (unsigned)end);
        if (tptr < start || tptr + sizeof(ID3DXEffect*) * 64 > end)
            printf("[Error] ShaderTable is out of module bounds!\n");
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

bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx)
        return false;

    __try
    {
        // Try reading the first DWORD — should be the vtable
        void* vtable = *(void**)fx;

        // Minimal sanity check: vtable pointer must also be non-null
        if (!vtable)
            return false;

        // Optional: check that vtable address is within a reasonable range
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(vtable, &mbi, sizeof(mbi)) == 0)
            return false;

        if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
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
    for (int i = 0; i < kShaderSlotCount; ++i)
    {
        ID3DXEffect* fx = table[i];
        if (fx && !IsValidShaderPointer(fx))
        {
            printf("[Sanity] Slot %d contains invalid fx pointer (value: %p), clearing\n", i, fx);
            table[i] = nullptr;
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
void (__cdecl* GetApplyGraphicsSettings())()
{
    auto base = (uintptr_t)GetModuleHandle(nullptr);
    return reinterpret_cast<void(__cdecl*)()>(base + kApplyGraphicsSettingsRVA);
}

bool SafePatchShaderTable(int slot, ID3DXEffect* fx)
{
    auto table = GetShaderTable();
    if (!table || slot < 0 || slot >= kShaderSlotCount)
        return false;

    DWORD oldProtect;
    if (!VirtualProtect(&table[slot], sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    ID3DXEffect* existing = table[slot];

    if (fx)
    {
        __try
        {
            fx->AddRef();
            printf("[Patch] AddRef() successful — fx = %p\n", fx);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf("[Patch] ⚠️ AddRef() failed — invalid fx for slot %d\n", slot);
            VirtualProtect(&table[slot], sizeof(ID3DXEffect*), oldProtect, &oldProtect);
            return false;
        }

        if (!IsValidShaderPointer(fx))
        {
            printf("[Patch] ❌ fx is invalid (slot %d), skipping patch\n", slot);
            fx->Release();
            VirtualProtect(&table[slot], sizeof(ID3DXEffect*), oldProtect, &oldProtect);
            return false;
        }
    }

    if (existing && IsValidShaderPointer(existing))
    {
        __try
        {
            existing->Release();
            printf("[Patch] Released old fx = %p from slot %d\n", existing, slot);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf("[Patch] ⚠️ Release() failed — old fx may be invalid in slot %d\n", slot);
        }
    }

    printf("[Patch] Writing shader to slot %d (%p)\n", slot, fx);

    table[slot] = fx;

    VirtualProtect(&table[slot], sizeof(ID3DXEffect*), oldProtect, &oldProtect);
    return fx != nullptr;
}

void PatchAllInvalidMatchingFx(ID3DXEffect* replacementFx)
{
    auto table = GetShaderTable();
    if (!table || !replacementFx)
        return;

    for (int i = 0; i < kShaderSlotCount; ++i)
    {
        ID3DXEffect* fx = table[i];
        if (fx && !IsValidShaderPointer(fx))
        {
            printf("[Reload] ⚠️ Slot %d contains invalid fx (%p), patching with %p\n", i, fx, replacementFx);
            SafePatchShaderTable(i, replacementFx);
        }
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

    if (!g_Device)
    {
        g_Device = device;
        LoadShaderOverrides(); // one-time fill of g_ShaderBuffers
    }

    // Hot-reload path
    {
        std::lock_guard<std::mutex> lock(g_TriggerShaderReloadMutex);
        if (!g_TriggerShaderReloadFor.empty() &&
            strcmp(pResource, g_TriggerShaderReloadFor.c_str()) == 0)
        {
            auto& src = g_ShaderBuffers[g_TriggerShaderReloadFor];
            if (src.empty())
            {
                printf("[Hotkey] ❌ Shader buffer for %s is empty!\n", g_TriggerShaderReloadFor.c_str());
                return E_FAIL;
            }

            FXIncludeHandler incl;
            LPD3DXEFFECT fx = nullptr;

            HRESULT hr = D3DXCreateEffect(device,
                                          src.data(), (UINT)src.size(),
                                          nullptr, &incl,
                                          D3DXSHADER_DEBUG,
                                          nullptr,
                                          &fx,
                                          outErrors);
            if (SUCCEEDED(hr) && fx)
            {
                auto slots = LookupShaderSlotsFromResource(pResource);
                for (int slot : slots)
                {
                    if (SafePatchShaderTable(slot, fx))
                        printf("[Hotkey] ✅ Hot-reloaded %s into slot %d\n", g_TriggerShaderReloadFor.c_str(), slot);
                    else
                        printf("[Hotkey] ❌ Failed to patch slot %d for %s\n", slot, g_TriggerShaderReloadFor.c_str());
                }
                g_TriggerShaderReloadFor.clear();
                *outEffect = fx;
                return S_OK;
            }
            else
            {
                printf("[Hotkey] ❌ Hot-reload failed for %s (hr=0x%08X)\n",
                       g_TriggerShaderReloadFor.c_str(), hr);
                g_TriggerShaderReloadFor.clear(); // avoid infinite loop
            }
        }
    }

    // One-time override path
    if (g_FxOverrides.count(pResource))
    {
        auto& src = g_ShaderBuffers[pResource];
        FXIncludeHandler incl;
        LPD3DXEFFECT fx = nullptr;

        HRESULT hr = D3DXCreateEffect(device,
                                      src.data(), (UINT)src.size(),
                                      defines, &incl,
                                      flags, pool,
                                      &fx, outErrors);

        if (SUCCEEDED(hr) && fx)
        {
            auto slots = LookupShaderSlotsFromResource(pResource);
            for (int slot : slots)
            {
                if (SafePatchShaderTable(slot, fx))
                    printf("[Patch] ✅ Replaced %s in slot %d\n", pResource, slot);
                else
                    printf("[Patch] ❌ Failed to patch slot %d for %s\n", slot, pResource);
            }
            *outEffect = fx;
            return S_OK;
        }
        else
        {
            printf("[Hook] ❌ Override failed for %s (hr=0x%08X)\n", pResource, hr);
        }
    }

    // Fallback to original game behavior
    return RealCreateFromResource(
        device, hModule, pResource,
        defines, include, flags, pool,
        outEffect, outErrors
    );
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
            D3DXSHADER_DEBUG, // Flags
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
        const std::string& key = "IDI_VISUALTREATMENT_FX";
        if (!g_ShaderBuffers.count(key) || g_ShaderBuffers[key].empty())
        {
            printf("[Reload] ❌ No shader buffer available for %s\n", key.c_str());
            return 0;
        }

        auto& src = g_ShaderBuffers[key];
        FXIncludeHandler incl;
        LPD3DXEFFECT fx = nullptr;
        auto slots = LookupShaderSlotsFromResource(key);
        for (int slot : slots)
        {
            LPD3DXEFFECT fx = nullptr;
            HRESULT hr = D3DXCreateEffect(g_Device, src.data(), (UINT)src.size(),
                                          nullptr, &incl, D3DXSHADER_DEBUG, nullptr, &fx, nullptr);

            if (SUCCEEDED(hr) && fx)
            {
                if (IsValidEffect(fx, slot) && SafePatchShaderTable(slot, fx))
                    printf("[Reload] ✅ Patched slot %d for %s\n", slot, key.c_str());
                else
                    printf("[Reload] ❌ Failed to patch slot %d for %s\n", slot, key.c_str());
            }
            else
            {
                printf("[Reload] ❌ Shader compilation failed for slot %d of %s (hr=0x%08X)\n", slot, key.c_str(), hr);
            }
        }

        // 🛡️ Scan all slots for invalid fx and patch them with the new effect
        PatchAllInvalidMatchingFx(fx);

        g_PendingShaderClearSlots.clear();
    }

    Sleep(50); // give the engine time
    SanityCheckShaderTable();
    auto fn = GetApplyGraphicsSettings();
    if (fn)
        fn();

    return 0;
}

DWORD WINAPI HotkeyThread(LPVOID)
{
    printf("[HotkeyThread] Waiting for F10...\n");

    while (true)
    {
        if (GetAsyncKeyState(VK_F10) & 1)
        {
            printf("[HotkeyThread] F10 pressed — recompiling and queueing reload\n");
            LoadShaderOverrides(); // refill g_ShaderBuffers

            // When F10 is pressed:
            {
                std::lock_guard<std::mutex> lk(g_TriggerShaderReloadMutex);
                g_TriggerShaderReloadFor = "IDI_VISUALTREATMENT_FX";
            }

            // Tell the hook we want to defer the clear
            g_PendingApplyGraphicsSettings = true;
            // Kick off the engine’s reload pass
            // And spawn a tiny thread to do the deferred clear + reapply afterwards
            CreateThread(nullptr, 0, TriggerApplyLater, nullptr, 0, nullptr);
            Sleep(50); // Give time for deferred patch
            SanityCheckShaderTable();
            auto fn = GetApplyGraphicsSettings();
            if (fn)
            {
                printf("[HotkeyThread] Calling ApplyGraphicsSettings()\n");
                fn();
            }
            else
            {
                printf("[HotkeyThread] ERROR: ApplyGraphicsSettings not found\n");
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
