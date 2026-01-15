#include <d3d9.h>
#include "Hooks.h"
#include "RenderTargetManager.h"
#include "Globals.h"
#include "Validators.h"

void RenderTargetManager::OnDeviceLost()
{
    SAFE_RELEASE(g_MotionBlurTexA);
    SAFE_RELEASE(g_MotionBlurTexB);
    SAFE_RELEASE(g_CurrentBlurSurface);
    SAFE_RELEASE(g_MotionBlurSurfaceA);
    SAFE_RELEASE(g_MotionBlurSurfaceB);
    SAFE_RELEASE(g_SceneCopySurface);
    SAFE_RELEASE(g_SceneCopyTex);
    SAFE_RELEASE(g_BlurHistoryTexA);
    SAFE_RELEASE(g_BlurHistorySurfA);
    SAFE_RELEASE(g_BlurHistoryTexB);
    SAFE_RELEASE(g_BlurHistorySurfB);
    g_CurrentBlurTex = nullptr;

    // Skip OnLostDevice for tracked effects here; some are stale and
    // cause stack corruption during reset in MW.

    SAFE_RELEASE(g_BlurEffect)
    SAFE_RELEASE(g_CustomBlurEffect)
    SAFE_RELEASE(g_ScreenQuadDecl)
    SAFE_RELEASE(g_GainMapTex)
    SAFE_RELEASE(g_VignetteTex)
    SAFE_RELEASE(g_BloomTex)
    SAFE_RELEASE(g_DofTex)
    SAFE_RELEASE(g_LinearDepthTex)
    SAFE_RELEASE(g_ExposureTex)
    SAFE_RELEASE(g_BloomLUTTex)
    SAFE_RELEASE(g_DepthTex)
    SAFE_RELEASE(g_BackBufferSurface)
    SAFE_RELEASE(g_LastSceneSurface)
    SAFE_RELEASE(g_LastSceneFullSurface)
    SAFE_RELEASE(g_LastSceneFullTex)
    g_MotionBlurHistoryReady = false;
}

bool RenderTargetManager::OnDeviceReset(LPDIRECT3DDEVICE9 device)
{
    if (!device)
        return false;

    IDirect3DSurface9* backBuffer = nullptr;
    if (FAILED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer)) || !backBuffer)
        return false;

    D3DSURFACE_DESC bbDesc{};
    if (FAILED(backBuffer->GetDesc(&bbDesc)))
    {
        backBuffer->Release();
        return false;
    }
    backBuffer->Release();

    g_Width = bbDesc.Width;
    g_Height = bbDesc.Height;

    screenQuadVerts[0] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
    screenQuadVerts[1] = {(float)g_Width, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    screenQuadVerts[2] = {0.0f, (float)g_Height, 0.0f, 1.0f, 0.0f, 1.0f};
    screenQuadVerts[3] = {(float)g_Width, (float)g_Height, 0.0f, 1.0f, 1.0f, 1.0f};

    OnDeviceLost(); // Always clear first

    HRESULT hr1 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTexA, nullptr);
    HRESULT hr2 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTexB, nullptr);
    HRESULT hr3 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_SceneCopyTex, nullptr);
    HRESULT hr4 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_BlurHistoryTexA, nullptr);
    HRESULT hr5 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_BlurHistoryTexB, nullptr);

    if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3) || FAILED(hr4) || FAILED(hr5))
    {
        printf_s("[RenderTargetManager] ❌ Failed to create motion blur textures\n");
        return false;
    }

    g_UseTexA = true;
    g_CurrentBlurTex = g_MotionBlurTexA;
    if (FAILED(g_MotionBlurTexA->GetSurfaceLevel(0, &g_MotionBlurSurfaceA)) ||
        FAILED(g_MotionBlurTexB->GetSurfaceLevel(0, &g_MotionBlurSurfaceB)) ||
        FAILED(g_SceneCopyTex->GetSurfaceLevel(0, &g_SceneCopySurface)) ||
        FAILED(g_BlurHistoryTexA->GetSurfaceLevel(0, &g_BlurHistorySurfA)) ||
        FAILED(g_BlurHistoryTexB->GetSurfaceLevel(0, &g_BlurHistorySurfB)))
    {
        printf_s("[RenderTargetManager] ? Failed to get blur surfaces\n");
        return false;
    }
    g_CurrentBlurSurface = g_MotionBlurSurfaceA;
    g_MotionBlurHistoryReady = false;
    if (!g_CurrentBlurSurface)
    {
        printf_s("[RenderTargetManager] ? Failed to get surface level\n");
        return false;
    }
    SAFE_RELEASE(g_ScreenQuadDecl);
    const D3DVERTEXELEMENT9 elements[] =
    {
        {0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        {0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
        {0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
        {0, 64, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
        {0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},
        {0, 96, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5},
        {0,112, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6},
        {0,128, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7},
        D3DDECL_END()
    };

    if (FAILED(device->CreateVertexDeclaration(elements, &g_ScreenQuadDecl)))
    {
        printf_s("[RenderTargetManager] ? Failed to create screen quad vertex declaration\n");
    }
    printf_s("[RenderTargetManager] ✅ Recreated motion blur targets (%ux%u)\n", g_Width, g_Height);
    return true;
}

void RenderTargetManager::Swap()
{
    g_UseTexA = !g_UseTexA;
    g_CurrentBlurTex = g_UseTexA ? g_MotionBlurTexA : g_MotionBlurTexB;
    g_CurrentBlurSurface = g_UseTexA ? g_MotionBlurSurfaceA : g_MotionBlurSurfaceB;
}

