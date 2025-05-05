#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <cstdio>
#include <MinHook.h>
#include <shlwapi.h> // for PathFileExistsA
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Shlwapi.lib")

// Globals
typedef HRESULT (WINAPI*EndScene_t)(LPDIRECT3DDEVICE9);
LPD3DXEFFECT g_pEffect = nullptr;
EndScene_t g_OriginalEndScene = nullptr;

// Utility log
void Log(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    std::cout << "[Hook] " << buf << std::endl;
}

// Shader loading stub
void LoadFXFiles(LPDIRECT3DDEVICE9 device)
{
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("FX\\*.fx", &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        std::string path = "FX\\" + std::string(fd.cFileName);
        Log("Found shader: %s", path.c_str());

        ID3DXEffect* pEffect = nullptr;
        ID3DXBuffer* pErrors = nullptr;
        HRESULT hr = D3DXCreateEffectFromFileA(device, path.c_str(), NULL, NULL, D3DXSHADER_DEBUG, NULL, &pEffect, &pErrors);
        Log("D3DXCreateEffectFromFileA returned HRESULT: 0x%08X", hr); // <-- add this line

        if (FAILED(hr))
        {
            Log("Failed to compile: %s", path.c_str());
            if (pErrors)
            {
                Log("Shader error: %s", (char*)pErrors->GetBufferPointer());
                pErrors->Release();
            }
        }
        else
        {
            Log("Successfully loaded: %s", path.c_str());
            pEffect->Release();
        }
    }
    while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

// Hook
static ID3DXEffect* g_VisualTreatment = nullptr;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = nullptr;

struct CUSTOMVERTEX
{
    float x, y, z, rhw;
    float u, v;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_TEX1)

struct ScreenVertex
{
    float x, y, z, rhw; // Position in screen space
    float u, v; // Texture coordinates
};

void DrawFullScreenQuad(LPDIRECT3DDEVICE9 pDevice)
{
    D3DVIEWPORT9 vp;
    pDevice->GetViewport(&vp);

    float w = (float)vp.Width;
    float h = (float)vp.Height;

    ScreenVertex quad[4] = {
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
        {w - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
        {-0.5f, h - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
        {w - 0.5f, h - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f},
    };

    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
    pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(ScreenVertex));
}

typedef HRESULT (WINAPI*D3DXCreateEffectFromFileA_t)(
    LPDIRECT3DDEVICE9 pDevice,
    LPCSTR pSrcFile,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT* ppEffect,
    LPD3DXBUFFER* ppCompilationErrors);

D3DXCreateEffectFromFileA_t OriginalCreateEffectFromFileA = nullptr;

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceA_t)(
    LPDIRECT3DDEVICE9 pDevice,
    HMODULE hSrcModule,
    LPCSTR pResource,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT* ppEffect,
    LPD3DXBUFFER* ppCompilationErrors);

D3DXCreateEffectFromResourceA_t OriginalCreateEffectFromResourceA = nullptr;

HRESULT WINAPI HookedCreateEffectFromResourceA(
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
    Log("[Hook] D3DXCreateEffectFromResourceA called with resource: %s", pResource ? pResource : "nullptr");

    if (pResource && strcmp(pResource, "IDI_VISUALTREATMENT_FX") == 0)
    {
        std::string customPath = "FX\\visualtreatment.fx";
        if (PathFileExistsA(customPath.c_str()))
        {
            Log("[Hook] Loading custom visualtreatment.fx from FX folder\n");
            HRESULT hr = D3DXCreateEffectFromFileA(
                pDevice,
                customPath.c_str(),
                pDefines,
                pInclude,
                Flags,
                pPool,
                ppEffect,
                ppCompilationErrors);

            if (SUCCEEDED(hr) && ppEffect && *ppEffect)
            {
                Log("[Hook] Successfully loaded replacement shader.\n");
                g_VisualTreatment = *ppEffect;
                return hr;
            }

            if (ppCompilationErrors && *ppCompilationErrors)
            {
                Log("[Hook] Shader compile error: %s", (char*)(*ppCompilationErrors)->GetBufferPointer());
            }

            Log("[Hook] Failed to compile replacement shader, falling back to original.\n");
        }
        else
        {
            Log("[Hook] File not found: %s", customPath.c_str());
        }
    }

    return OriginalCreateEffectFromResourceA(
        pDevice, hSrcModule, pResource, pDefines, pInclude,
        Flags, pPool, ppEffect, ppCompilationErrors);
}

HRESULT WINAPI HookedEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    Log("[Hook] EndScene called.");

    static bool loggedTechniques = false;

    if (g_VisualTreatment)
    __try {
        // Set screen size if used by your shader
        D3DXVECTOR4 screenSize(1280.0f, 720.0f, 1.0f / 1280.0f, 1.0f / 720.0f);
        g_VisualTreatment->SetVector("gScreenSize", &screenSize);

        if (!loggedTechniques)
        {
            for (UINT i = 0; ; ++i)
            {
                D3DXHANDLE tech = g_VisualTreatment->GetTechnique(i);
                if (!tech) break;
                D3DXTECHNIQUE_DESC desc;
                if (SUCCEEDED(g_VisualTreatment->GetTechniqueDesc(tech, &desc)))
                {
                    Log("Technique[%u]: %s", i, desc.Name);
                }
            }
            loggedTechniques = true;
        }

        // Select the first technique available
        D3DXHANDLE hTechnique = nullptr;
        D3DXEFFECT_DESC effectDesc;
        if (SUCCEEDED(g_VisualTreatment->GetDesc(&effectDesc)))
        {
            for (UINT i = 0; i < effectDesc.Techniques; ++i)
            {
                hTechnique = g_VisualTreatment->GetTechnique(i);
                if (SUCCEEDED(g_VisualTreatment->ValidateTechnique(hTechnique)))
                {
                    g_VisualTreatment->SetTechnique(hTechnique);
                    Log("Using technique index: %u", i);
                    break;
                }
            }
        }

        UINT passes = 0;
        if (hTechnique && SUCCEEDED(g_VisualTreatment->Begin(&passes, 0)))
        {
            for (UINT i = 0; i < passes; ++i)
            {
                g_VisualTreatment->BeginPass(i);
                DrawFullScreenQuad(pDevice);
                g_VisualTreatment->EndPass();
            }
            g_VisualTreatment->End();
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Log("[Hook] Exception occurred in EndScene effect rendering.");
    }

    if (g_OriginalEndScene)
        return g_OriginalEndScene(pDevice);

    Log("[Hook] g_OriginalEndScene is null!");
    return D3D_OK;
}

HRESULT WINAPI HookedCreateEffectFromFileA(
    LPDIRECT3DDEVICE9 pDevice,
    LPCSTR pSrcFile,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT* ppEffect,
    LPD3DXBUFFER* ppCompilationErrors)
{
    Log("[Hook] D3DXCreateEffectFromFileA called with file: %s", pSrcFile ? pSrcFile : "nullptr");

    return OriginalCreateEffectFromFileA(
        pDevice, pSrcFile, pDefines, pInclude,
        Flags, pPool, ppEffect, ppCompilationErrors);
}

// Entry point
DWORD WINAPI InitThread(LPVOID)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    Log("Initializing hook...");

    // Wait for d3dx9_43.dll to be loaded
    while (!GetModuleHandleA("d3dx9_43.dll")) Sleep(100);

    HMODULE d3dx9 = GetModuleHandleA("d3dx9_43.dll");
    void* effectFunc = GetProcAddress(d3dx9, "D3DXCreateEffectFromResourceA");
    if (!effectFunc)
    {
        Log("[Hook] Failed to locate D3DXCreateEffectFromResourceA");
        return 0;
    }

    void* fxFileFunc = GetProcAddress(d3dx9, "D3DXCreateEffectFromFileA");
    if (fxFileFunc)
    {
        Log("[Hook] Found D3DXCreateEffectFromFileA at %p", fxFileFunc);
        MH_CreateHook(fxFileFunc, HookedCreateEffectFromFileA,
                      reinterpret_cast<void**>(&OriginalCreateEffectFromFileA));
        MH_EnableHook(fxFileFunc);
    }
    else
    {
        Log("[Hook] D3DXCreateEffectFromFileA not found.");
    }

    // Create dummy window
    WNDCLASSEXA wc = {
        sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        "DummyWindowClass", NULL
    };
    RegisterClassExA(&wc);
    HWND hWnd = CreateWindowA("DummyWindowClass", "Dummy", WS_OVERLAPPEDWINDOW,
                              0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    if (!hWnd)
    {
        Log("[Hook] CreateWindowA failed.");
        return 0;
    }

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D)
    {
        Log("[Hook] Direct3DCreate9 failed.");
        return 0;
    }

    D3DPRESENT_PARAMETERS pp = {};
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = hWnd;

    LPDIRECT3DDEVICE9 pDevice = nullptr;
    HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                    hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &pDevice);

    if (FAILED(hr))
    {
        Log("[Hook] HAL device failed. Trying REF...");
        hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF,
                                hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &pDevice);
    }

    if (FAILED(hr) || !pDevice)
    {
        Log("[Hook] REF device also failed. HRESULT: 0x%08X", hr);
        DestroyWindow(hWnd);
        UnregisterClassA("DummyWindowClass", GetModuleHandle(NULL));
        pD3D->Release();
        return 0;
    }

    // void** vtable = *reinterpret_cast<void***>(pDevice);
    // void* endSceneAddr = vtable[42];
    // Log("[Hook] EndScene address: %p", endSceneAddr);

    void* endSceneAddr = (void*)0x71B96420;
    Log("[Hook] Using hardcoded EndScene address: %p", endSceneAddr);


    if (MH_Initialize() != MH_OK)
    {
        Log("[Hook] MH_Initialize failed.");
        return 0;
    }
    if (MH_CreateHook(effectFunc, HookedCreateEffectFromResourceA,
                      reinterpret_cast<void**>(&OriginalCreateEffectFromResourceA)) != MH_OK)
    {
        Log("[Hook] Failed to create hook for D3DXCreateEffectFromResourceA");
    }
    if (MH_CreateHook(endSceneAddr, &HookedEndScene, reinterpret_cast<void**>(&g_OriginalEndScene)) != MH_OK)
    {
        Log("[Hook] Failed to create hook for EndScene");
    }
    if (MH_EnableHook(effectFunc) != MH_OK)
    {
        Log("[Hook] Failed to enable hook for D3DXCreateEffectFromResourceA");
    }
    if (MH_EnableHook(endSceneAddr) != MH_OK)
    {
        Log("[Hook] Failed to enable hook for EndScene");
    }

    Log("[Hook] Hooks installed.");

    pDevice->Release();
    pD3D->Release();
    DestroyWindow(hWnd);
    UnregisterClassA("DummyWindowClass", GetModuleHandle(NULL));

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        InitThread(0);
    }
    return TRUE;
}
