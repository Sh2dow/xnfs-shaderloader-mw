#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
// NFSMW Shader Compiler & Loader (Trampoline Style)
#include <cstdio>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <MinHook.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <experimental/filesystem>
#include "stdafx.h"
#include "includes/d3d9hook.cpp"
#include "includes/errorreport.cpp"
#include "includes/injector/injector.hpp"
namespace fs = std::experimental::filesystem;

static std::unordered_map<std::string, LPD3DXEFFECT> g_PrecompiledEffects;
#define SafeRelease(p) { if(p) { (p)->Release(); (p)=nullptr; } }

// Maximum number of shader entries expected
static const unsigned int kMaxShaderEntries = 512;

unsigned int CurrentShaderIndex;

// Last successfully loaded effect for fallback
static LPD3DXEFFECT g_LastValidEffect = nullptr;
static const char* g_ResolvedShaderName = nullptr;
extern "C" const char* g_ResolvedShaderName;
const void* g_ResolvedNameTrial_2C = nullptr;
const void* g_ResolvedNameTrial_28 = nullptr;
const void* g_ResolvedNameTrial_30 = nullptr;
bool g_PreloadTriggered = false;

// Utility for logging
int __cdecl cusprintf(const char* Format, ...)
{
    va_list ArgList;
    int Result;
    va_start(ArgList, Format);
    Result = vprintf(Format, ArgList);
    va_end(ArgList);
    return Result;
}

std::string ToUpper(const std::string& str)
{
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    return out;
}

std::string ToLower(const std::string& str)
{
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

LPDIRECT3DDEVICE9 g_Device = nullptr;

class FXIncludeHandler : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE type, LPCSTR fileName, LPCVOID, LPCVOID* data, UINT* bytes) override
    {
        std::string path = std::string("fx/") + fileName;
        printf("[Include] Trying to open: %s\n", path.c_str());

        FILE* f = fopen(path.c_str(), "rb");
        if (!f)
        {
            printf("[Include] Failed to open: %s\n", path.c_str());
            return E_FAIL;
        }

        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);

        BYTE* buffer = new BYTE[len];
        fread(buffer, 1, len, f);
        fclose(f);

        *data = buffer;
        *bytes = len;

        printf("[Include] Opened: %s\n", path.c_str());
        return S_OK;
    }

    STDMETHOD(Close)(LPCVOID data) override
    {
        delete[] reinterpret_cast<const BYTE*>(data);
        return S_OK;
    }
};

using D3DXCreateEffectFn = HRESULT(WINAPI*)(
    IDirect3DDevice9*,
    HMODULE,
    LPCSTR,
    const D3DXMACRO*,
    LPD3DXINCLUDE,
    DWORD,
    ID3DXEffectPool*,
    ID3DXEffect**,
    ID3DXBuffer**);

D3DXCreateEffectFn OriginalD3DXCreateEffect = nullptr;
bool g_PreloadDone = false;

using D3DXCreateEffectFromResourceFn = HRESULT(WINAPI*)(
    IDirect3DDevice9*, HMODULE, LPCSTR, const D3DXMACRO*, LPD3DXINCLUDE,
    DWORD, ID3DXEffectPool*, ID3DXEffect**, ID3DXBuffer**);


void PreloadShaders()
{
    printf("[Preload] >>> PreloadShaders() called\n");

    if (!g_Device)
    {
        printf("[Error] No valid device to preload shaders.\n");
        return;
    }

    printf("[Init] Preloading shaders from fx/... using live device = %p\n", g_Device);

    for (const auto& entry : fs::directory_iterator("fx"))
    {
        if (!fs::is_regular_file(entry.path()))
            continue;

        std::string path = entry.path().string();
        std::string ext = entry.path().extension().string();

        if (ext != ".fx")
        {
            continue;
        }

        std::string name = entry.path().stem().string();
        printf("[Preload] Compiling %s.fx...\n", name.c_str());

        FILE* f = fopen(entry.path().string().c_str(), "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> data(len);
        fread(data.data(), 1, len, f);
        fclose(f);

        ID3DXBuffer* errors = nullptr;
        ID3DXEffect* fx = nullptr;
        FXIncludeHandler includeHandler;
        std::string localFxName = name + ".fx";
        HRESULT hr = D3DXCreateEffectFromFile(
            g_Device,
            localFxName.c_str(),
            nullptr,
            &includeHandler,
            D3DXSHADER_DEBUG,
            nullptr,
            &fx,
            &errors);


        if (FAILED(hr))
        {
            printf("[Preload] Failed to compile %s.fx\n", name.c_str());
            if (errors)
            {
                printf("[Preload] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        std::string key = "IDI_" + ToUpper(name) + "_FX";
        printf("[Preload] Caching shader with key: %s\n", key.c_str()); // This should print "IDI_VISUALTREATMENT_FX"
        g_PrecompiledEffects[key] = fx;
    }
}

HRESULT WINAPI ShaderWrapper(
    IDirect3DDevice9* device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    ID3DXEffectPool* pool,
    ID3DXEffect** outEffect,
    ID3DXBuffer** outErrors)
{
    g_ResolvedShaderName = pResource;
    printf("[Wrapper] Shader = %s\n", pResource ? pResource : "(null)");

    // ✅ Assign g_Device here (since Hooked_CreateEffectFromResource is not used)
    if (!g_Device && device)
    {
        g_Device = device;
        printf("[Wrapper] g_Device assigned = %p\n", g_Device);

        if (!g_PreloadTriggered)
        {
            printf("[Wrapper] >>> Calling PreloadShaders()\n");
            PreloadShaders();
            g_PreloadTriggered = true;
        }
    }

    // Normalize the key (since cache uses uppercase)
    std::string key = ToUpper(std::string(pResource));
    auto it = g_PrecompiledEffects.find(key);
    if (it != g_PrecompiledEffects.end())
    {
        *outEffect = it->second;
        printf("[Wrapper] Using precompiled cache for %s\n", key.c_str());
        return S_OK;
    }

    printf("[Wrapper] No cache found for key: %s\n", key.c_str());

    if (pResource && strncmp(pResource, "IDI_", 4) == 0)
    {
        std::string name = ToLower(pResource + 4);
        if (name.size() > 3 && name.substr(name.size() - 3) == "_fx")
            name = name.substr(0, name.size() - 3);

        std::string fxPath = "fx/" + name + ".fx";

        FILE* f = fopen(fxPath.c_str(), "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<char> buffer(len);
            fread(buffer.data(), 1, len, f);
            fclose(f);

            cusprintf("[Override] Loading %s instead of resource.\n", fxPath.c_str());

            ID3DXBuffer* errors = nullptr;
            ID3DXEffect* fx = nullptr;
            FXIncludeHandler includeHandler;

            HRESULT hr = D3DXCreateEffect(
                device,
                buffer.data(), (UINT)len,
                nullptr,
                &includeHandler,
                D3DXSHADER_DEBUG,
                pool,
                &fx,
                &errors);

            if (FAILED(hr))
            {
                if (errors)
                {
                    fwrite(errors->GetBufferPointer(), 1, errors->GetBufferSize(), stderr);
                    cusprintf("[Override] Compilation error: %s\n", (char*)errors->GetBufferPointer());
                    errors->Release();
                }
                else
                {
                    cusprintf("[Override] Failed to compile %s.fx, HRESULT: 0x%X\n", fxPath.c_str(), hr);
                }
            }

            *outEffect = fx;
            return S_OK;
        }
    }

    // fallback
    static D3DXCreateEffectFn fallbackThunk = reinterpret_cast<D3DXCreateEffectFn>(0x007E7D76);
    D3DXCreateEffectFn realFn = OriginalD3DXCreateEffect ? OriginalD3DXCreateEffect : fallbackThunk;

    return realFn(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

HRESULT WINAPI Hooked_CreateEffectFromResource(
    IDirect3DDevice9* device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    ID3DXEffectPool* pool,
    ID3DXEffect** outEffect,
    ID3DXBuffer** outErrors)
{
    if (!g_Device)
    {
        g_Device = device;
        printf("[Hook] g_Device initialized = %p\n", g_Device);

        if (!g_PreloadTriggered)
        {
            printf("[Hook] >>> Calling PreloadShaders()\n");
            PreloadShaders();
            g_PreloadTriggered = true;
        }
    }

    return ShaderWrapper(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

int InitShaderHook()
{
    HMODULE d3dx = GetModuleHandleA("d3dx9_26.dll");
    if (!d3dx)
    {
        printf("[Hook] d3dx9_26.dll not loaded!\n");
        return 1;
    }

    printf("[Hook] d3dx9_26.dll handle = %p\n", d3dx);

    void* addr = (void*)GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
    if (addr)
    {
        printf("[Hook] Hooked D3DXCreateEffectFromResourceA at IAT entry = 0x%p\n", addr);
    }
    else
    {
        printf("[Hook] GetProcAddress failed on D3DXCreateEffectFromResourceA\n");
        return 1;
    }

    if (MH_Initialize() != MH_OK) return 1;
    if (MH_CreateHook(addr, &Hooked_CreateEffectFromResource, reinterpret_cast<void**>(&OriginalD3DXCreateEffect)) !=
        MH_OK)
        return 1;
    if (MH_EnableHook(addr) != MH_OK) return 1;

    // Patch game's call site
    injector::MakeCALL(0x006C60D2, ShaderWrapper, true);
    printf("[Hook] Shader call replaced at 0x006C60D2\n");

    return 0;
}

DWORD WINAPI InitThread(LPVOID)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    printf("[Init] Shader Hook Thread Started\n");

    // Wait for d3dx9_26.dll to load (max 5 seconds)
    for (int i = 0; i < 100; ++i)
    {
        if (GetModuleHandleA("d3dx9_26.dll"))
            break;
        Sleep(50);
    }

    InitShaderHook();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
