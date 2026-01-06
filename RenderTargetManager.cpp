#include <d3d9.h>
#include "Hooks.h"
#include "RenderTargetManager.h"

void RenderTargetManager::OnDeviceLost()
{
    g_DeviceResetInProgress = true;

    SAFE_RELEASE(g_MotionBlurTexA);
    SAFE_RELEASE(g_MotionBlurTexB);
    SAFE_RELEASE(g_CurrentBlurSurface);
    g_CurrentBlurTex = nullptr;

    for (auto& [name, fx] : g_ActiveEffects)
    {
        if (fx)
            fx->OnLostDevice(); // Important for safety
    }

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
}

bool RenderTargetManager::OnDeviceReset(LPDIRECT3DDEVICE9 device)
{
    if (!device)
        return false;

    D3DVIEWPORT9 vp;
    if (FAILED(device->GetViewport(&vp)))
        return false;

    g_Width = vp.Width;
    g_Height = vp.Height;

    screenQuadVerts[0] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
    screenQuadVerts[1] = {(float)g_Width, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    screenQuadVerts[2] = {0.0f, (float)g_Height, 0.0f, 1.0f, 0.0f, 1.0f};
    screenQuadVerts[3] = {(float)g_Width, (float)g_Height, 0.0f, 1.0f, 1.0f, 1.0f};

    OnDeviceLost(); // Always clear first

    HRESULT hr1 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTexA, nullptr);
    HRESULT hr2 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTexB, nullptr);

    if (FAILED(hr1) || FAILED(hr2))
    {
        printf_s("[RenderTargetManager] ❌ Failed to create motion blur textures\n");
        return false;
    }

    g_UseTexA = true;
    g_CurrentBlurTex = g_MotionBlurTexA;

    if (FAILED(g_CurrentBlurTex->GetSurfaceLevel(0, &g_CurrentBlurSurface)))
    {
        printf_s("[RenderTargetManager] ❌ Failed to get surface level\n");
        return false;
    }

    printf_s("[RenderTargetManager] ✅ Recreated motion blur targets (%ux%u)\n", g_Width, g_Height);
    return true;
}

void RenderTargetManager::Swap()
{
    g_UseTexA = !g_UseTexA;
    g_CurrentBlurTex = g_UseTexA ? g_MotionBlurTexA : g_MotionBlurTexB;

    SAFE_RELEASE(g_CurrentBlurSurface);
    if (g_CurrentBlurTex)
        g_CurrentBlurTex->GetSurfaceLevel(0, &g_CurrentBlurSurface);
}
