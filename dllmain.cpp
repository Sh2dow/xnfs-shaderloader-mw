// dllmain.cpp - entry point for NFSMW-RenderTarget (Motion Blur Injection)
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
LPD3DXEFFECT g_LastEffect = nullptr;

unsigned int CurrentShaderNum;
ID3DXEffectCompiler* pEffectCompiler;
ID3DXBuffer* pBuffer = nullptr, *pEffectBuffer = nullptr;
char FilenameBuf[2048];

using Direct3DCreate9_t = IDirect3D9* (WINAPI*)(UINT);
Direct3DCreate9_t oDirect3DCreate9 = nullptr;

using EndScene_t = HRESULT(WINAPI*)(LPDIRECT3DDEVICE9);
EndScene_t oEndScene = nullptr;

using D3DXCreateEffectFromResourceA_t = HRESULT(WINAPI*)(LPDIRECT3DDEVICE9, HMODULE, LPCSTR, CONST D3DXMACRO*, LPD3DXINCLUDE, DWORD,
                                                        LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
D3DXCreateEffectFromResourceA_t oD3DXCreateEffectFromResourceA = nullptr;

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

bool CheckIfFileExists(const char* FileName)
{
    FILE* fin = fopen(FileName, "rb");
    if (!fin)
    {
        char alt[MAX_PATH];
        strcpy(alt, "fx\\");
        strcat(alt, FileName);
        fin = fopen(alt, "rb");
        if (!fin)
            return false;
        strcpy((char*)FileName, alt);
    }
    fclose(fin);
    return true;
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

            if (g_LastEffect)
            {
                g_LastEffect->SetTexture("MOTIONBLUR_TEXTURE", g_MotionBlurTex);
                Log("Rebound MOTIONBLUR_TEXTURE in EndScene");
            }
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
    Log(std::string("Resource requested: ") + (pResource ? pResource : "NULL"));
    Log("Intercepted D3DXCreateEffectFromResourceA");

    if (pResource && strstr(pResource, ".fxo"))
    {
        strcpy(FilenameBuf, pResource);
        char* ext = strrchr(FilenameBuf, '.');
        if (ext) strcpy(ext, ".fx");

        if (CheckIfFileExists(FilenameBuf))
        {
            HRESULT hr = D3DXCreateEffectCompilerFromFile(FilenameBuf, nullptr, nullptr, 0, &pEffectCompiler, &pBuffer);
            if (SUCCEEDED(hr))
            {
                Log("Compiling: " + std::string(FilenameBuf));
                hr = pEffectCompiler->CompileEffect(0, &pEffectBuffer, &pBuffer);
                if (SUCCEEDED(hr))
                {
                    hr = D3DXCreateEffect(pDevice, *(void**)(pEffectBuffer->GetBufferPointer()),
                                          (UINT)pEffectBuffer->GetBufferSize(), pDefines, pInclude,
                                          Flags, pPool, ppEffect, &pBuffer);

                    if (SUCCEEDED(hr))
                    {
                        Log("Compiled and created effect successfully from FX file");
                        if (g_MotionBlurTex && *ppEffect)
                            (*ppEffect)->SetTexture("MOTIONBLUR_TEXTURE", g_MotionBlurTex);
                        g_LastEffect = *ppEffect;
                        return hr;
                    }
                    else
                    {
                        Log("Failed to create effect from compiled FX");
                    }
                }
                else
                {
                    Log("Compilation failed");
                }
            }
            else
            {
                Log("Compiler init failed");
            }
        }
    }

    // fallback: original resource loading
    auto orig = reinterpret_cast<D3DXCreateEffectFromResourceA_t>(
        GetProcAddress(GetModuleHandleA("d3dx9_43.dll"), "D3DXCreateEffectFromResourceA"));

    HRESULT hr = orig(pDevice, hSrcModule, pResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);

    if (SUCCEEDED(hr) && ppEffect && *ppEffect)
    {
        Log("Effect created successfully");

        if (g_MotionBlurTex)
        {
            (*ppEffect)->SetTexture("MOTIONBLUR_TEXTURE", g_MotionBlurTex);
            Log("Bound MOTIONBLUR_TEXTURE to shader via Resource hook");
        }

        // Optional: Log available techniques WITHOUT Begin()
        for (UINT i = 0;; ++i)
        {
            D3DXHANDLE tech = (*ppEffect)->GetTechnique(i);
            if (!tech)
                break; // No more techniques

            D3DXTECHNIQUE_DESC desc;
            if (SUCCEEDED((*ppEffect)->GetTechniqueDesc(tech, &desc)))
            {
                Log("Found technique: " + std::string(desc.Name));
                if (strcmp(desc.Name, "t0") == 0)
                {
                    // Optional: only if you are sure this effect is safe to override
                    // (*ppEffect)->SetTechnique(tech);
                    Log("Technique 't0' is present");
                }
            }
        }
    }

    // Hook EndScene if not yet done
    if (!oEndScene && pDevice)
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
