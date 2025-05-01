// dllmain.cpp - entry point for XNFS-ShaderLoader-MW (Motion Blur Injection)
#include <windows.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include <d3d9.h>
#include <d3dx9effect.h>
#include "includes/injector/injector.hpp"

HMODULE g_hModule = nullptr;
std::wstring g_FxOverridePath;

IDirect3DTexture9* g_MotionBlurTex = nullptr;
IDirect3DSurface9* g_MotionBlurSurface = nullptr;

using Direct3DCreate9_t = IDirect3D9* (WINAPI*)(UINT);
Direct3DCreate9_t oDirect3DCreate9 = nullptr;

using EndScene_t = HRESULT(WINAPI*)(LPDIRECT3DDEVICE9);
EndScene_t oEndScene = nullptr;

using D3DXCreateEffectFromFileW_t = HRESULT(WINAPI*)(LPDIRECT3DDEVICE9, LPCWSTR, CONST D3DXMACRO*, LPD3DXINCLUDE, DWORD,
                                                     LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
D3DXCreateEffectFromFileW_t oD3DXCreateEffectFromFileW = nullptr;

void Log(const std::string& msg)
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(g_hModule, path, MAX_PATH);
    std::filesystem::path base = path;
    base = base.remove_filename();
    std::filesystem::path logFile = base / L"motionblur_log.txt";

    std::ofstream log(logFile, std::ios::app);
    log << msg << std::endl;
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    std::cout << msg << std::endl;
}

IDirect3D9* WINAPI hkDirect3DCreate9(UINT SDKVersion)
{
    Log("hkDirect3DCreate9 called");
    if (!oDirect3DCreate9)
    {
        Log("oDirect3DCreate9 is NULL!");
        return nullptr;
    }

    return oDirect3DCreate9(SDKVersion);
}

uintptr_t GetOriginalCallTarget(uintptr_t callSite)
{
    // call instruction is E8 <rel32>
    int32_t relOffset = *reinterpret_cast<int32_t*>(callSite + 1);
    return callSite + 5 + relOffset;
}

HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 device)
{
    static bool initialized = false;
    if (!initialized)
    {
        D3DVIEWPORT9 vp;
        if (SUCCEEDED(device->GetViewport(&vp)))
        {
            if (SUCCEEDED(device->CreateTexture(vp.Width, vp.Height, 1, D3DUSAGE_RENDERTARGET,
                D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTex, nullptr)))
            {
                g_MotionBlurTex->GetSurfaceLevel(0, &g_MotionBlurSurface);
                Log("Created MOTIONBLUR_TEXTURE");
            }
            else
            {
                Log("FAILED to create MOTIONBLUR_TEXTURE");
            }
        }
        initialized = true;
    }

    if (g_MotionBlurTex && g_MotionBlurSurface)
    {
        IDirect3DSurface9* backBuffer = nullptr;
        if (SUCCEEDED(device->GetRenderTarget(0, &backBuffer)))
        {
            device->StretchRect(backBuffer, nullptr, g_MotionBlurSurface, nullptr, D3DTEXF_LINEAR);
            Log("Updated MOTIONBLUR_TEXTURE from backbuffer");
            backBuffer->Release();
        }
    }

    return oEndScene(device); // Call original
}

HRESULT WINAPI hkD3DXCreateEffectFromResourceA(
    LPDIRECT3DDEVICE9 pDevice,
    HMODULE hSrcModule,
    LPCSTR pResource,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT* ppEffect,
    LPD3DXBUFFER* ppCompilationErrors)
{
    Log("Intercepted D3DXCreateEffectFromResourceA");

    // Create the effect with the original call
    auto orig = reinterpret_cast<decltype(&D3DXCreateEffectFromResourceA)>(
        GetProcAddress(GetModuleHandleA("d3dx9_26.dll"), "D3DXCreateEffectFromResourceA"));

    HRESULT hr = orig(pDevice, hSrcModule, pResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);

    // Hook EndScene if not already
    if (SUCCEEDED(hr) && ppEffect && *ppEffect && g_MotionBlurTex)
    {
        (*ppEffect)->SetTexture("MOTIONBLUR_TEXTURE", g_MotionBlurTex);
        Log("Bound MOTIONBLUR_TEXTURE to shader via Resource hook");
    }

    if (oEndScene == nullptr && pDevice)
    {
        void** vtable = *reinterpret_cast<void***>(pDevice);
        oEndScene = reinterpret_cast<EndScene_t>(vtable[42]); // Index 42 = EndScene
        injector::WriteMemory<uintptr_t>(&vtable[42], (uintptr_t)hkEndScene, true);
        Log("Hooked EndScene");
    }

    return hr;
}

void InitFXOverride()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(g_hModule, path, MAX_PATH);
    std::filesystem::path base = path;
    base = base.remove_filename();
    Log("Init base path: " + base.string());

    // Save the original
    uintptr_t d3dCreate9CallSite = 0x006E6A40;
    oDirect3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(GetOriginalCallTarget(d3dCreate9CallSite));

    // Patch the call
    injector::MakeCALL(d3dCreate9CallSite, (uintptr_t)&hkDirect3DCreate9, true);
    injector::MakeCALL(0x006C60D2, (uintptr_t)&hkD3DXCreateEffectFromResourceA, true);

    Log("Patched Direct3DCreate9 call site and stored original");
    Log("Patched Direct3DCreate9 call site");
}

void InitializeMWShaderLoader()
{
    InitFXOverride();
    Log(">>> InitializeMWShaderLoader complete");
}

void CleanupMWShaderLoader()
{
    Log("Shutdown complete");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        Log(">>> DllMain start");
        InitializeMWShaderLoader();
        break;
    case DLL_PROCESS_DETACH:
        CleanupMWShaderLoader();
        break;
    }
    return TRUE;
}
