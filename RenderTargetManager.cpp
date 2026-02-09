#include <d3d9.h>
#include "Hooks.h"
#include "RenderTargetManager.h"
#include "Globals.h"
#include "Validators.h"
#include <windows.h>
#include <d3dx9.h>

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
    SAFE_RELEASE(g_VTDepthTex)
    SAFE_RELEASE(g_VTGainMapTex)
    SAFE_RELEASE(g_MotionBlurMaskTex)
    SAFE_RELEASE(g_LastGameDepthSurface)
    SAFE_RELEASE(g_IntzDepthSurface)
    SAFE_RELEASE(g_IntzDepthTex)
    SAFE_RELEASE(g_BackBufferSurface)
    SAFE_RELEASE(g_LastSceneSurface)
    SAFE_RELEASE(g_LastSceneFullSurface)
    SAFE_RELEASE(g_LastSceneFullTex)
    SAFE_RELEASE(g_SceneColorSurface)
    SAFE_RELEASE(g_SceneColorTex)
    g_SceneColorFrame = 0;
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
    HRESULT hrScene = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_SceneColorTex, nullptr);
    HRESULT hr4 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_BlurHistoryTexA, nullptr);
    HRESULT hr5 = device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                                        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_BlurHistoryTexB, nullptr);

    // Shader-readable depth (INTZ). Used by X360Stuff VT as HeightMapTexture.
    // If unsupported, depth-based gating stays disabled.
    const D3DFORMAT D3DFMT_INTZ = (D3DFORMAT)MAKEFOURCC('I','N','T','Z');
    HRESULT hrIntz = device->CreateTexture(
        g_Width, g_Height, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_INTZ, D3DPOOL_DEFAULT, &g_IntzDepthTex, nullptr);
    if (SUCCEEDED(hrIntz) && g_IntzDepthTex)
    {
        if (FAILED(g_IntzDepthTex->GetSurfaceLevel(0, &g_IntzDepthSurface)))
        {
            SAFE_RELEASE(g_IntzDepthTex);
            printf_s("[RenderTargetManager] ⚠️ INTZ created but no surface level; disabling\n");
        }
    }
    else
    {
        printf_s("[RenderTargetManager] ⚠️ INTZ depth unsupported (hr=0x%08X)\n", hrIntz);
        g_IntzDepthTex = nullptr;
        g_IntzDepthSurface = nullptr;
    }

    if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3) || FAILED(hrScene) || FAILED(hr4) || FAILED(hr5))
    {
        printf_s("[RenderTargetManager] ❌ Failed to create motion blur textures\n");
        return false;
    }

    g_UseTexA = true;
    // History is produced by our custom blur pass into g_BlurHistoryTexA/B.
    // Do not point g_CurrentBlurTex at g_MotionBlurTexA/B (they may be unused/uninitialized).
    // However, we DO need a non-null history texture for VT rebinding right after Reset/reload.
    // Point at a valid RT now; CustomMotionBlurHook will overwrite/initialize contents on first frame.
    g_CurrentBlurTex = g_BlurHistoryTexA;
    if (FAILED(g_MotionBlurTexA->GetSurfaceLevel(0, &g_MotionBlurSurfaceA)) ||
        FAILED(g_MotionBlurTexB->GetSurfaceLevel(0, &g_MotionBlurSurfaceB)) ||
        FAILED(g_SceneCopyTex->GetSurfaceLevel(0, &g_SceneCopySurface)) ||
        FAILED(g_SceneColorTex->GetSurfaceLevel(0, &g_SceneColorSurface)) ||
        FAILED(g_BlurHistoryTexA->GetSurfaceLevel(0, &g_BlurHistorySurfA)) ||
        FAILED(g_BlurHistoryTexB->GetSurfaceLevel(0, &g_BlurHistorySurfB)))
    {
        printf_s("[RenderTargetManager] ? Failed to get blur surfaces\n");
        return false;
    }
    g_CurrentBlurSurface = g_BlurHistorySurfA;
    g_MotionBlurHistoryReady = false;
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
    // Force VT rebind after any reset/reload so sampler handles are refreshed.
    g_PendingVTRebind = true;

    // Optional blur mask texture. If present, the shader can use it to reduce blur on cars/center.
    // Mirrors MW360Tweaks behavior (loads BlurMask.png and binds it as "MotionBlurMask").
    SAFE_RELEASE(g_MotionBlurMaskTex);
    {
        using CreateTexFromFileFn = HRESULT (WINAPI*)(LPDIRECT3DDEVICE9, LPCSTR, LPDIRECT3DTEXTURE9*);
        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (!d3dx)
            d3dx = LoadLibraryA("d3dx9_43.dll");
        auto createTex = d3dx ? reinterpret_cast<CreateTexFromFileFn>(GetProcAddress(d3dx, "D3DXCreateTextureFromFileA")) : nullptr;
        if (createTex)
        {
            // Accept both working-dir root and fx/ folder.
            if (FAILED(createTex(device, "BlurMask.png", &g_MotionBlurMaskTex)))
                (void)createTex(device, "fx\\BlurMask.png", &g_MotionBlurMaskTex);
        }
    }

    // Always provide a valid mask texture so the shader can sample without branching.
    // Default: generate a radial mask (center sharp, edges blurred) unless a real BlurMask.png is provided.
    if (!g_MotionBlurMaskTex)
    {
        const UINT w = 256, h = 256;
        IDirect3DTexture9* t = nullptr;
        if (SUCCEEDED(device->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &t, nullptr)) && t)
        {
            D3DLOCKED_RECT lr{};
            if (SUCCEEDED(t->LockRect(0, &lr, nullptr, 0)) && lr.pBits && lr.Pitch >= (INT)(w * 4))
            {
                // Approximate MW's UVESVIGNETTE: minimal blur near lower-center, stronger towards edges.
                // This is a safe default when we can't reliably steal the VT vignette texture.
                const float cx = 0.50f;
                const float cy = 0.65f;
                // Tune for "no center haze" by default (blur only near the periphery).
                const float inner = 0.60f; // radius with near-zero blur
                const float outer = 0.99f; // radius where blur reaches full strength
                for (UINT y = 0; y < h; ++y)
                {
                    uint32_t* row = reinterpret_cast<uint32_t*>((uint8_t*)lr.pBits + y * lr.Pitch);
                    for (UINT x = 0; x < w; ++x)
                    {
                        const float u = (x + 0.5f) / (float)w;
                        const float v = (y + 0.5f) / (float)h;
                        const float dx = (u - cx);
                        const float dy = (v - cy);
                        const float dist = std::sqrt(dx * dx + dy * dy);
                        float m = 0.0f;
                        if (dist <= inner) m = 0.0f;
                        else if (dist >= outer) m = 1.0f;
                        else
                        {
                            // smoothstep
                            float t01 = (dist - inner) / (outer - inner);
                            t01 = max(0.0f, min(1.0f, t01));
                            m = t01 * t01 * (3.0f - 2.0f * t01);
                        }
                        const uint8_t a = (uint8_t)(m * 255.0f);
                        const uint32_t argb = (uint32_t(a) << 24) | (uint32_t(a) << 16) | (uint32_t(a) << 8) | uint32_t(a);
                        row[x] = argb;
                    }
                }
                t->UnlockRect(0);
                g_MotionBlurMaskTex = t;
            }
            else
            {
                t->Release();
            }
        }
    }

    return true;
}

void RenderTargetManager::Swap()
{
    g_UseTexA = !g_UseTexA;
    g_CurrentBlurTex = g_UseTexA ? g_BlurHistoryTexA : g_BlurHistoryTexB;
    g_CurrentBlurSurface = g_UseTexA ? g_BlurHistorySurfA : g_BlurHistorySurfB;
}

