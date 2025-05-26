#include <algorithm>
#include <atomic>
#include <windows.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include <MinHook.h>
#include <unordered_set>
#include "includes/injector/injector.hpp"
#include "d3d9.h"
#include "d3dx9shader.h" // for ID3DXEffectCompiler

// -------------------- GLOBALS --------------------
#define SHADER_TABLE_PTR reinterpret_cast<ID3DXEffect**>(0x0094D9C4)

LPDIRECT3DDEVICE9 g_Device = nullptr;

std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::unordered_set<std::string> g_FxOverrides;
std::unordered_set<int> g_OwnedShaderSlots;

struct ShaderInfo
{
    std::string key;
    ID3DXEffect* effect;
};

std::unordered_map<std::string, ShaderInfo> g_ActiveEffects;

std::unordered_map<std::string, int> g_ShaderSlotMap = {
    {"IDI_WORLD_FX", 0},
    {"IDI_WORLDREFLECT_FX", 1},
    {"IDI_WORLDBONE_FX", 2},
    {"IDI_WORLDNORMALMAP_FX", 3},
    {"IDI_CAR_FX", 4},
    {"IDI_GLOSSYWINDOW_FX", 5},
    {"IDI_TREE_FX", 6},
    {"IDI_WORLDMIN_FX", 7},
    {"IDI_WORLDNOFOG_FX", 8},
    {"IDI_FE_FX", 9},
    {"IDI_FE_MASK_FX", 10},
    {"IDI_FILTER_FX", 11},
    {"IDI_OVERBRIGHT_FX", 12},
    {"IDI_SCREENFILTER_FX", 13},
    {"IDI_RAIN_DROP_FX", 14},
    {"IDI_RUNWAYLIGHT_FX", 15},
    {"IDI_VISUALTREATMENT_FX", 16},
    {"IDI_WORLDPRELIT_FX", 17},
    {"IDI_PARTICLES_FX", 18},
    {"IDI_SKYBOX_FX", 19},
    {"IDI_SHADOW_MAP_MESH_FX", 20},
    {"IDI_SKYBOX_CG_FX", 21},
    {"IDI_SHADOW_CG_FX", 22},
    {"IDI_CAR_SHADOW_MAP_FX", 23},
    {"IDI_WORLDDEPTH_FX", 24},
    {"IDI_WORLDNORMALMAPDEPTH_FX", 25},
    {"IDI_CARDEPTH_FX", 26},
    {"IDI_GLOSSYWINDOWDEPTH_FX", 27},
    {"IDI_TREEDEPTH_FX", 28},
    {"IDI_SHADOW_MAP_MESH_DEPTH_FX", 29},
    {"IDI_WORLDNORMALMAPNOFOG_FX", 30},
    {"IDI_GLASSREFLECTSHADER_FX", 31}
};

typedef ULONG (STDMETHODCALLTYPE* ReleaseFn)(IDirect3DDevice9*);
ReleaseFn RealRelease = nullptr;

int LookupShaderSlotFromResource(const std::string& key)
{
    auto it = g_ShaderSlotMap.find(key);
    if (it == g_ShaderSlotMap.end())
        printf("[Lookup] WARNING: Shader %s not mapped to a slot!\n", key.c_str());
    return (it != g_ShaderSlotMap.end()) ? it->second : -1;
}

int hkReload = VK_F2; // optionally load from INI too
// -------------------- HELPERS --------------------

std::string ToUpper(const std::string& str)
{
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    return out;
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

ULONG STDMETHODCALLTYPE HookedRelease(IDirect3DDevice9* device)
{
    static bool cleaned = false;
    if (!cleaned)
    {
        printf("[Hook] Cleaning up shader table (no Release calls)...\n");

        for (int i = 0; i < 62; ++i)
        {
            SHADER_TABLE_PTR[i] = nullptr; // prevent use-after-free
        }

        cleaned = true;
    }

    return RealRelease(device); // call the original Release()
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
    // First-time setup
    if (!g_Device)
    {
        g_Device = device;
        LoadShaderOverrides();
    }

    static int g_HookCallCount = 0;
    printf("[Hook] D3DXCreateEffectFromResourceA called #%d — pResource = %s\n", g_HookCallCount++,
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
                bool patched = false;
    
                int index = LookupShaderSlotFromResource(pResource);
                if (index >= 0 && index < 62)
                {
                    (*outEffect)->AddRef();
                    // Avoid injecting effect directly into game memory
                    g_ActiveEffects[pResource] = {pResource, *outEffect};
                    printf("[Patch] ⚠️ Skipped SHADER_TABLE_PTR[%d] assignment to prevent dangling use\n", index);
                    printf("[Patch] ✅ Replaced live shader slot %d for %s\n", index, pResource);
                    patched = true;
                }
    
                if (!patched)
                {
                    g_ActiveEffects[pResource] = {pResource, *outEffect};
                    printf("[Patch] ⚠️ Shader not found in table — stored in internal map instead: %s\n", pResource);
                }
    
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

void ReloadTrackedEffects()
{
    for (auto it = g_ActiveEffects.begin(); it != g_ActiveEffects.end(); ++it)
    {
        const std::string& key = it->first;
        ShaderInfo& shader = it->second;

        printf("[ReloadTrack] Recompiling and reloading: %s\n", key.c_str());

        // Do NOT release; we assume the game owns it now
        shader.effect = nullptr;

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
            int index = LookupShaderSlotFromResource(key);
            if (index >= 0 && index < 62)
            {
                SHADER_TABLE_PTR[index] = fx;
                printf("[ReloadTrack] ✅ Updated live shader slot %d\n", index);
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

DWORD WINAPI HotkeyThread(LPVOID)
{
    printf("[HotkeyThread] Started and waiting for F2...\n");

    while (true)
    {
        if (GetAsyncKeyState(hkReload) & 1)
        {
            printf("[HotkeyThread] F2 pressed — recompiling shaders...\n");
            LoadShaderOverrides();
            ReloadTrackedEffects(); // ← 🔁 call the generic reload
            printf("[HotkeyThread] Shader reload complete.\n");

            *(BYTE*)0x00982C39 = 1;
            printf_s("[HotReload] ✅ Set LoadedFlagMaybe = 1 (0x00982C39)\n");

            while (GetAsyncKeyState(hkReload) & 0x8000)
                Sleep(10);
        }

        Sleep(10);
    }

    return 0;
}

// -------------------- DllMain --------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (MH_Initialize() != MH_OK)
        {
            printf("[MinHook] Failed to initialize!\n");
            return FALSE;
        }

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
            printf("[Init] Hooked D3DXCreateEffectFromResourceA\n");
            // ✅ Correct: Launch F2 hotkey thread *after* setup
            HANDLE hThread = CreateThread(nullptr, 0, HotkeyThread, nullptr, 0, nullptr);
            if (hThread)
                printf("[Init] Hotkey thread started successfully.\n");
            else
                printf("[Init] Failed to start hotkey thread! Error: %lu\n", GetLastError());
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        printf("[Shutdown] Nulling shader table...\n");

        for (int i = 0; i < 62; ++i)
            SHADER_TABLE_PTR[i] = nullptr;

        g_ActiveEffects.clear();
        g_ShaderOverridePaths.clear();
        g_ShaderBuffers.clear();
        g_FxOverrides.clear();
        g_Device = nullptr;
    }

    return TRUE;
}
