#include <string>
#include <cmath>
#include "Globals.h"
#include "Hooks.h"
#include "EffectManager.h"
#include "MotionBlurPass.h"
#include "RenderTargetManager.h"
#include "ScopedStateBlock.h"

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name);

void __cdecl MotionBlurPass::CustomMotionBlurHook()
{
    const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    if (gameFlow < 3 || g_RenderTargetManager.g_DeviceResetInProgress)
        return;

    if (g_CustomMotionBlurRanThisFrame)
        return;

    IDirect3DDevice9* device = g_Device;
    if (!device)
        return;

    // Source for blur must be the full scene color (pre-VT), not the post-VT copy.
    // Post-VT can contain UI/front-end overlays and causes "garbage" blur in tunnels.
    IDirect3DTexture9* src = g_RenderTargetManager.g_LastSceneFullTex;
    if (!src)
        src = g_RenderTargetManager.g_SceneCopyTex;
    if (!src || !g_RenderTargetManager.g_LastSceneFullTex)
        return;

    // Pick ping-pong destination (history)
    IDirect3DTexture9*  dstTex  = nullptr;
    IDirect3DSurface9*  dstSurf = nullptr;

    if (g_RenderTargetManager.g_UseTexA)
    {
        dstTex  = g_RenderTargetManager.g_BlurHistoryTexA;
        dstSurf = g_RenderTargetManager.g_BlurHistorySurfA;
    }
    else
    {
        dstTex  = g_RenderTargetManager.g_BlurHistoryTexB;
        dstSurf = g_RenderTargetManager.g_BlurHistorySurfB;
    }

    if (!dstTex || !dstSurf)
        return;

    // State safety (good for DXVK too)
    ScopedStateBlock stateBlock(device);

    // Don’t run mid-graph: only when RT0 is backbuffer
    IDirect3DSurface9* rt0 = nullptr;
    if (SUCCEEDED(device->GetRenderTarget(0, &rt0)) && rt0)
    {
        const bool ok = (rt0 == g_RenderTargetManager.g_BackBufferSurface);
        rt0->Release();
        if (!ok)
            return;
    }

    // Always keep history initialized to a valid scene copy. This prevents undefined RT sampling
    // and allows VT to bind PREV/MOTIONBLUR even when blur amount is zero.
    if (!g_MotionBlurHistoryReady || g_MotionBlurAmount <= 0.0f)
    {
        IDirect3DSurface9* srcSurf = nullptr;
        if (SUCCEEDED(src->GetSurfaceLevel(0, &srcSurf)) && srcSurf)
        {
            device->StretchRect(srcSurf, nullptr, dstSurf, nullptr, D3DTEXF_NONE);
            srcSurf->Release();
        }
        g_RenderTargetManager.g_CurrentBlurTex     = dstTex;
        g_RenderTargetManager.g_CurrentBlurSurface = dstSurf;
        g_MotionBlurHistoryReady = true;
        g_RenderTargetManager.g_UseTexA = !g_RenderTargetManager.g_UseTexA;
        g_RenderTargetManager.g_PendingVTRebind = true;
        g_CustomMotionBlurRanThisFrame = true;
        return;
    }

    // Run blur: src(pre-VT full scene) -> dst(history)
    RenderBlurPass(device, src, dstTex, dstSurf);

    // Publish history for VT sampling (next frame / same frame if timing allows)
    g_RenderTargetManager.g_CurrentBlurTex     = dstTex;
    g_RenderTargetManager.g_CurrentBlurSurface = dstSurf;
    g_MotionBlurHistoryReady = true;

    // flip for next frame
    g_RenderTargetManager.g_UseTexA = !g_RenderTargetManager.g_UseTexA;

    g_RenderTargetManager.g_PendingVTRebind = true;

    g_CustomMotionBlurRanThisFrame = true;
}

void __cdecl MotionBlurPass::RenderBlurPass(
    IDirect3DDevice9* device,
    IDirect3DTexture9* srcTex,
    IDirect3DTexture9* dstTex,
    IDirect3DSurface9* dstSurface)
{
    if (!device) return;

    if (!EffectManager::EnsureCustomBlurEffect(device))
    {
        printf_s("[Blur] EnsureCustomBlurEffect failed\n");
        return;
    }
    ID3DXEffect* fx = g_RenderTargetManager.g_CustomBlurEffect;

    D3DXEFFECT_DESC ed{};
    fx->GetDesc(&ed);
    printf("[Blur] CustomBlurEffect techniques=%u\n", ed.Techniques);
    for (UINT i=0;i<ed.Techniques;i++){
        D3DXHANDLE th = fx->GetTechnique(i);
        D3DXTECHNIQUE_DESC td{};
        if (th && SUCCEEDED(fx->GetTechniqueDesc(th,&td)) && td.Name)
            printf("[Blur] tech[%u]=%s\n", i, td.Name);
    }

    
    IDirect3DSurface9* oldRT = nullptr;
    if (FAILED(device->GetRenderTarget(0, &oldRT)) || !oldRT) return;

    D3DVIEWPORT9 savedVP{};
    device->GetViewport(&savedVP);

    if (!srcTex || !dstTex || !dstSurface)
    {
        SAFE_RELEASE(oldRT);
        return;
    }

    // 🚫 Critical: never sample and render to the same texture (feedback → dark smear)
    if (srcTex == dstTex)
    {
        printf_s("[Blur] ❌ srcTex == dstTex (alias). Skipping blur this frame.\n");
        device->SetRenderTarget(0, oldRT);
        SAFE_RELEASE(oldRT);
        return;
    }

    // If you want even stronger protection, also guard the surface identity:
    IDirect3DSurface9* srcSurf = nullptr;
    if (SUCCEEDED(srcTex->GetSurfaceLevel(0, &srcSurf)) && srcSurf)
    {
        if (srcSurf == dstSurface)
        {
            printf_s("[Blur] ❌ srcSurf == dstSurface (alias). Skipping blur.\n");
            SAFE_RELEASE(srcSurf);
            device->SetRenderTarget(0, oldRT);
            SAFE_RELEASE(oldRT);
            return;
        }
    }
    SAFE_RELEASE(srcSurf);

    if (FAILED(device->TestCooperativeLevel()))
    {
        SAFE_RELEASE(oldRT);
        return;
    }
    if (IsBadReadPtr(dstSurface, sizeof(void*)) || IsBadReadPtr(*(void**)dstSurface, sizeof(void*)))
    {
        printf_s("[Blur] dstSurface invalid, skipping SetRenderTarget\n");
        SAFE_RELEASE(oldRT);
        return;
    }
    const HRESULT setRtHr = device->SetRenderTarget(0, dstSurface);
    if (FAILED(setRtHr))
    {
        printf_s("[Blur] SetRenderTarget failed: 0x%08X\n", setRtHr);
        SAFE_RELEASE(oldRT);
        return;
    }

    D3DSURFACE_DESC sd{};
    dstSurface->GetDesc(&sd);
    D3DVIEWPORT9 vp{};
    vp.X = 0;
    vp.Y = 0;
    vp.Width = sd.Width;
    vp.Height = sd.Height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    device->SetViewport(&vp);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);

    D3DXHANDLE hDiffuse = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    D3DXHANDLE hDepth = fx->GetParameterByName(nullptr, "DEPTHBUFFER_TEXTURE");
    D3DXHANDLE hTexel = fx->GetParameterByName(nullptr, "BlurTexelSize");
    D3DXHANDLE hMotionVec = fx->GetParameterByName(nullptr, "MotionVec");
    D3DXHANDLE hMotionScale = fx->GetParameterByName(nullptr, "MotionBlurScale");
    D3DXHANDLE oldTech = fx->GetCurrentTechnique();

    IDirect3DBaseTexture9* oldDiffuse = nullptr;
    if (hDiffuse)
        fx->GetTexture(hDiffuse, &oldDiffuse);
    // Quiet by default (logging here tanks FPS).

    if (hDiffuse)
        fx->SetTexture(hDiffuse, srcTex);
    if (hDepth)
        fx->SetTexture(hDepth, g_RenderTargetManager.g_VTDepthTex ? g_RenderTargetManager.g_VTDepthTex
                                                                  : g_RenderTargetManager.g_DepthTex);
    if (hMotionVec)
    {
        const float mv[2] = {g_MotionVec[0], g_MotionVec[1]};
        fx->SetFloatArray(hMotionVec, mv, 2);
    }
    if (hMotionScale)
    {
        const float mvx = g_MotionVec[0];
        const float mvy = g_MotionVec[1];
        float scale = std::sqrt(mvx * mvx + mvy * mvy) * 0.5f;
        fx->SetFloat(hMotionScale, scale);
    }
    if (hTexel)
    {
        const float blurScale = 2.0f;
        fx->SetVector(hTexel, &D3DXVECTOR4(blurScale / (float)sd.Width,
                                           blurScale / (float)sd.Height, 0.0f, 0.0f));
    }
    D3DXHANDLE hBlend = fx->GetParameterByName(nullptr, "MotionBlurBlend");
    // MotionBlurScale was previously used to multiplex debug modes inside a single ps_2_0 shader,
    // but that easily exceeds ps_2_0 temp registers. We now select dedicated debug techniques instead.
    if (hBlend)
        fx->SetFloat(hBlend, g_MotionBlurAmount);

    {
        // Only use the explicit technique name. If this fails, we want a hard failure
        // (so "disabling motionblur in the .fx" actually shows up during testing).
        const char* const techs[] = {"motionblur"};
        if (!EffectManager::TrySetTechnique(fx, techs, sizeof(techs) / sizeof(techs[0])))
        {
            printf_s("[Blur] ❌ Technique 'motionblur' missing/invalid in custom FX\n");
            goto cleanup;
        }
    }

    IDirect3DVertexDeclaration9* oldDecl = nullptr;
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->GetVertexDeclaration(&oldDecl);
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->SetVertexDeclaration(g_RenderTargetManager.g_ScreenQuadDecl);

    UINT passes = 0;
    if (FAILED(fx->Begin(&passes, 0)))
    {
        printf_s("[Blur] Begin failed\n");
    }
    else
    {
        // Quiet by default (logging here tanks FPS).
        for (UINT i = 0; i < passes; ++i)
        {
            if (SUCCEEDED(fx->BeginPass(i)))
            {
                static RenderTargetManager::QuadVertex8 vertices[4];
                const float x[4] = {-1.0f, 1.0f, -1.0f, 1.0f};
                const float y[4] = {1.0f, 1.0f, -1.0f, -1.0f};
                const float uBase[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                const float vBase[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                const float blurScales[7] = {0.01f, 0.02f, 0.03f, 0.04f, 0.05f, 0.06f, 0.07f};
                const float motionU = g_MotionVec[0];
                const float motionV = g_MotionVec[1];
                const float motionZ = g_MotionVec[2];
                for (int vi = 0; vi < 4; ++vi)
                {
                    vertices[vi].x = x[vi];
                    vertices[vi].y = y[vi];
                    vertices[vi].z = 0.0f;
                    vertices[vi].rhw = 1.0f;
                    float u0 = uBase[vi];
                    float v0 = vBase[vi];
                    const float uv0[4] = {u0, v0, 0.0f, 0.0f};
                    float uvs[7][4]{};
                    float baseU = u0 - 0.5f;
                    float baseV = v0 - 0.5f;
                    for (int si = 0; si < 7; ++si)
                    {
                        const float scale = blurScales[si];
                        const float blurFactor = 1.0f - motionZ * scale;
                        const float offsetU = baseU * blurFactor + motionU * scale + 0.5f;
                        const float offsetV = baseV * blurFactor + motionV * scale + 0.5f;
                        uvs[si][0] = offsetU;
                        uvs[si][1] = offsetV;
                        uvs[si][2] = 0.0f;
                        uvs[si][3] = 0.0f;
                    }
                    memcpy(vertices[vi].t0, uv0, sizeof(uv0));
                    memcpy(vertices[vi].t1, uvs[0], sizeof(uvs[0]));
                    memcpy(vertices[vi].t2, uvs[1], sizeof(uvs[1]));
                    memcpy(vertices[vi].t3, uvs[2], sizeof(uvs[2]));
                    memcpy(vertices[vi].t4, uvs[3], sizeof(uvs[3]));
                    memcpy(vertices[vi].t5, uvs[4], sizeof(uvs[4]));
                    memcpy(vertices[vi].t6, uvs[5], sizeof(uvs[5]));
                    memcpy(vertices[vi].t7, uvs[6], sizeof(uvs[6]));
                }
                device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
                                        vertices, sizeof(RenderTargetManager::QuadVertex8));
                fx->EndPass();
            }
        }
        fx->End();
    }
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->SetVertexDeclaration(oldDecl);
    SAFE_RELEASE(oldDecl);

cleanup:
    if (hDiffuse)
        fx->SetTexture(hDiffuse, oldDiffuse);
    if (oldDiffuse)
        oldDiffuse->Release();
    if (oldTech)
        fx->SetTechnique(oldTech);

    device->SetRenderTarget(0, oldRT);
    device->SetViewport(&savedVP);

    SAFE_RELEASE(oldRT);

    g_RenderTargetManager.g_CurrentBlurTex = dstTex;
    g_RenderTargetManager.g_CurrentBlurSurface = dstSurface;
}

void __cdecl MotionBlurPass::RenderBlurOverride(
    IDirect3DDevice9* device,
    IDirect3DBaseTexture9* src0,
    IDirect3DBaseTexture9* src1,
    IDirect3DSurface9* dstSurface)
{
    if (!device || !src0)
        return;

    // Defensive: engine can hand us stale pointers during reload/transitions.
    // MSVC debug fill (0xEEEEEEEE) indicates freed/uninitialized memory.
    if (IsBadReadPtr(src0, sizeof(void*)) || IsBadReadPtr(*(void**)src0, sizeof(void*)))
        return;
    if (dstSurface && (IsBadReadPtr(dstSurface, sizeof(void*)) || IsBadReadPtr(*(void**)dstSurface, sizeof(void*))))
        return;

    if (!EffectManager::EnsureCustomBlurEffect(device))
        return;

    ID3DXEffect* fx = g_RenderTargetManager.g_CustomBlurEffect;
    if (!fx)
        return;

    // Preserve device state for the engine pipeline.
    ScopedStateBlock stateBlock(device);

    IDirect3DSurface9* targetSurf = g_RenderTargetManager.g_UseTexA
        ? g_RenderTargetManager.g_BlurHistorySurfA
        : g_RenderTargetManager.g_BlurHistorySurfB;
    IDirect3DTexture9* targetTex = g_RenderTargetManager.g_UseTexA
        ? g_RenderTargetManager.g_BlurHistoryTexA
        : g_RenderTargetManager.g_BlurHistoryTexB;
    if (!targetSurf || !targetTex)
        return;

    if (IsBadReadPtr(targetSurf, sizeof(void*)) || IsBadReadPtr(*(void**)targetSurf, sizeof(void*)))
        return;
    if (IsBadReadPtr(targetTex, sizeof(void*)) || IsBadReadPtr(*(void**)targetTex, sizeof(void*)))
        return;

    // When amount is zero, still keep history initialized and stable (copy current scene).
    // This prevents vanilla or uninitialized data from showing up as "blur garbage".
    if (g_MotionBlurAmount <= 0.0f)
    {
        IDirect3DTexture9* srcTex = nullptr;
        IDirect3DSurface9* srcSurf = nullptr;
        if (!IsBadReadPtr(src0, sizeof(void*)) &&
            SUCCEEDED(src0->QueryInterface(__uuidof(IDirect3DTexture9),
                                           reinterpret_cast<void**>(&srcTex))) &&
            srcTex && SUCCEEDED(srcTex->GetSurfaceLevel(0, &srcSurf)) && srcSurf)
        {
            device->StretchRect(srcSurf, nullptr, targetSurf, nullptr, D3DTEXF_NONE);
        }
        SAFE_RELEASE(srcSurf);
        SAFE_RELEASE(srcTex);

        g_RenderTargetManager.g_CurrentBlurTex = targetTex;
        g_RenderTargetManager.g_CurrentBlurSurface = targetSurf;
        g_MotionBlurHistoryReady = true;
        g_RenderTargetManager.g_UseTexA = !g_RenderTargetManager.g_UseTexA;
        g_RenderTargetManager.g_PendingVTRebind = true;
        return;
    }

    // First valid blur frame: seed the history target with the current scene so we never sample
    // undefined RT memory (this is the source of "dark fragments"/frontend ghosts).
    if (!g_MotionBlurHistoryReady || !src1)
    {
        IDirect3DTexture9* srcTex = nullptr;
        IDirect3DSurface9* srcSurf = nullptr;
        if (SUCCEEDED(src0->QueryInterface(__uuidof(IDirect3DTexture9),
                                           reinterpret_cast<void**>(&srcTex))) &&
            srcTex && SUCCEEDED(srcTex->GetSurfaceLevel(0, &srcSurf)) && srcSurf)
        {
            device->StretchRect(srcSurf, nullptr, targetSurf, nullptr, D3DTEXF_NONE);
        }
        SAFE_RELEASE(srcSurf);
        SAFE_RELEASE(srcTex);

        g_RenderTargetManager.g_CurrentBlurTex = targetTex;
        g_RenderTargetManager.g_CurrentBlurSurface = targetSurf;
        g_MotionBlurHistoryReady = true;
    }

    D3DSURFACE_DESC sd{};
    targetSurf->GetDesc(&sd);

    if (FAILED(device->SetRenderTarget(0, targetSurf)))
        return;

    D3DVIEWPORT9 vp{};
    vp.X = 0;
    vp.Y = 0;
    vp.Width = sd.Width;
    vp.Height = sd.Height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    device->SetViewport(&vp);

    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);

    D3DXHANDLE hDiffuse = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    D3DXHANDLE hDepth = fx->GetParameterByName(nullptr, "DEPTHBUFFER_TEXTURE");
    if (hDiffuse)
        fx->SetTexture(hDiffuse, src0);
    if (hDepth)
        fx->SetTexture(hDepth, g_RenderTargetManager.g_VTDepthTex ? g_RenderTargetManager.g_VTDepthTex
                                                                  : g_RenderTargetManager.g_DepthTex);

    D3DXHANDLE hPrev = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
    if (hPrev)
        fx->SetTexture(hPrev, src1 ? src1 : nullptr);

    D3DXHANDLE hMotionVec = fx->GetParameterByName(nullptr, "MotionVec");
    if (hMotionVec)
    {
        const float mv[2] = {g_MotionVec[0], g_MotionVec[1]};
        fx->SetFloatArray(hMotionVec, mv, 2);
    }

    D3DXHANDLE hMotionScale = fx->GetParameterByName(nullptr, "MotionBlurScale");
    if (hMotionScale)
    {
        const float mvx = g_MotionVec[0];
        const float mvy = g_MotionVec[1];
        float scale = std::sqrt(mvx * mvx + mvy * mvy) * 0.5f;
        fx->SetFloat(hMotionScale, scale);
    }

    D3DXHANDLE hBlend = fx->GetParameterByName(nullptr, "MotionBlurBlend");
    if (hBlend)
        fx->SetFloat(hBlend, g_MotionBlurAmount);

    D3DXHANDLE hTexel = fx->GetParameterByName(nullptr, "BlurTexelSize");
    if (hTexel)
    {
        const float blurScale = 2.0f;
        fx->SetVector(hTexel, &D3DXVECTOR4(blurScale / (float)sd.Width,
                                           blurScale / (float)sd.Height, 0.0f, 0.0f));
    }

    // Support both our external file (`fx/motionblur.fx` uses technique `motionblur`)
    // and the embedded fallback (`EffectManager.cpp` uses technique `blur`).
    const char* const techs[] = {"motionblur", "blur"};
    if (!EffectManager::TrySetTechnique(fx, techs, sizeof(techs) / sizeof(techs[0])))
        return;

    IDirect3DVertexDeclaration9* oldDecl = nullptr;
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->GetVertexDeclaration(&oldDecl);
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->SetVertexDeclaration(g_RenderTargetManager.g_ScreenQuadDecl);

    UINT passes = 0;
    if (SUCCEEDED(fx->Begin(&passes, 0)))
    {
        for (UINT i = 0; i < passes; ++i)
        {
            if (SUCCEEDED(fx->BeginPass(i)))
            {
                RenderTargetManager::QuadVertex8 vertices[4]{};
                const float x[4] = {-1.0f, 1.0f, -1.0f, 1.0f};
                const float y[4] = {1.0f, 1.0f, -1.0f, -1.0f};
                const float uBase[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                const float vBase[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                const float blurScales[7] = {0.01f, 0.02f, 0.03f, 0.04f, 0.05f, 0.06f, 0.07f};

                float motionU = g_MotionVec[0];
                float motionV = g_MotionVec[1];
                float motionZ = g_MotionVec[2];
                const bool hasMotion = (std::fabs(motionU) + std::fabs(motionV) >= 1e-4f);
                // If we don't have real motion vectors, do NOT blur. Any fake fallback direction
                // creates a constant fullscreen haze (exact symptom you're seeing).
                if (hasMotion)
                {
                    const float mvScale = max(0.0f, min(1.0f, g_MotionBlurAmount));
                    motionU *= mvScale;
                    motionV *= mvScale;
                }
                else
                {
                    motionU = 0.0f;
                    motionV = 0.0f;
                    motionZ = 0.0f;
                }

                for (int vi = 0; vi < 4; ++vi)
                {
                    vertices[vi].x = x[vi];
                    vertices[vi].y = y[vi];
                    vertices[vi].z = 0.0f;
                    vertices[vi].rhw = 1.0f;

                    float u0 = uBase[vi];
                    float v0 = vBase[vi];
                    const float uv0[4] = {u0, v0, 0.0f, 0.0f};
                    float uvs[7][4]{};
                    float baseU = u0 - 0.5f;
                    float baseV = v0 - 0.5f;
                    for (int si = 0; si < 7; ++si)
                    {
                        float offsetU = u0;
                        float offsetV = v0;
                        if (hasMotion)
                        {
                            const float scale = blurScales[si];
                            const float blurFactor = 1.0f - motionZ * scale;
                            offsetU = baseU * blurFactor + motionU * scale + 0.5f;
                            offsetV = baseV * blurFactor + motionV * scale + 0.5f;
                            // Avoid sampling outside; clamp prevents weird wrap artifacts under DXVK.
                            offsetU = max(0.0f, min(1.0f, offsetU));
                            offsetV = max(0.0f, min(1.0f, offsetV));
                        }
                        uvs[si][0] = offsetU;
                        uvs[si][1] = offsetV;
                        uvs[si][2] = 0.0f;
                        uvs[si][3] = 0.0f;
                    }

                    memcpy(vertices[vi].t0, uv0, sizeof(uv0));
                    memcpy(vertices[vi].t1, uvs[0], sizeof(uvs[0]));
                    memcpy(vertices[vi].t2, uvs[1], sizeof(uvs[1]));
                    memcpy(vertices[vi].t3, uvs[2], sizeof(uvs[2]));
                    memcpy(vertices[vi].t4, uvs[3], sizeof(uvs[3]));
                    memcpy(vertices[vi].t5, uvs[4], sizeof(uvs[4]));
                    memcpy(vertices[vi].t6, uvs[5], sizeof(uvs[5]));
                    memcpy(vertices[vi].t7, uvs[6], sizeof(uvs[6]));
                }

                device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
                                        vertices, sizeof(RenderTargetManager::QuadVertex8));
                fx->EndPass();
            }
        }
        fx->End();
    }

    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->SetVertexDeclaration(oldDecl);
    SAFE_RELEASE(oldDecl);

    g_RenderTargetManager.g_CurrentBlurTex = targetTex;
    g_RenderTargetManager.g_CurrentBlurSurface = targetSurf;
    g_MotionBlurHistoryReady = true;
    g_RenderTargetManager.g_PendingVTRebind = true;
    g_RenderTargetManager.g_UseTexA = !g_RenderTargetManager.g_UseTexA;

    // Also pass-through copy into the engine RT so downstream effects (NOS, etc.) stay intact.
    if (dstSurface && SUCCEEDED(device->SetRenderTarget(0, dstSurface)))
    {
        D3DSURFACE_DESC dstDesc{};
        dstSurface->GetDesc(&dstDesc);
        D3DVIEWPORT9 dstVp{};
        dstVp.X = 0;
        dstVp.Y = 0;
        dstVp.Width = dstDesc.Width;
        dstVp.Height = dstDesc.Height;
        dstVp.MinZ = 0.0f;
        dstVp.MaxZ = 1.0f;
        device->SetViewport(&dstVp);

        if (hDiffuse)
            fx->SetTexture(hDiffuse, src0);
        if (hDepth)
            fx->SetTexture(hDepth, g_RenderTargetManager.g_DepthTex);

        const char* const copyTechs[] = {"copy"};
        if (EffectManager::TrySetTechnique(fx, copyTechs, sizeof(copyTechs) / sizeof(copyTechs[0])))
        {
            if (g_RenderTargetManager.g_ScreenQuadDecl)
                device->SetVertexDeclaration(g_RenderTargetManager.g_ScreenQuadDecl);

            UINT copyPasses = 0;
            if (SUCCEEDED(fx->Begin(&copyPasses, 0)))
            {
                for (UINT i = 0; i < copyPasses; ++i)
                {
                    if (SUCCEEDED(fx->BeginPass(i)))
                    {
                        RenderTargetManager::QuadVertex8 vertices[4]{};
                        const float x[4] = {-1.0f, 1.0f, -1.0f, 1.0f};
                        const float y[4] = {1.0f, 1.0f, -1.0f, -1.0f};
                        const float uBase[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                        const float vBase[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                        for (int vi = 0; vi < 4; ++vi)
                        {
                            vertices[vi].x = x[vi];
                            vertices[vi].y = y[vi];
                            vertices[vi].z = 0.0f;
                            vertices[vi].rhw = 1.0f;
                            const float uv0[4] = {uBase[vi], vBase[vi], 0.0f, 0.0f};
                            memcpy(vertices[vi].t0, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t1, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t2, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t3, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t4, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t5, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t6, uv0, sizeof(uv0));
                            memcpy(vertices[vi].t7, uv0, sizeof(uv0));
                        }
                        device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2,
                                                vertices, sizeof(RenderTargetManager::QuadVertex8));
                        fx->EndPass();
                    }
                }
                fx->End();
            }
        }
    }
}

void __cdecl MotionBlurPass::CompositeToSurface(
    IDirect3DDevice9* device,
    IDirect3DBaseTexture9* sceneTex,
    IDirect3DBaseTexture9* historyTex,
    IDirect3DSurface9* dstSurface,
    float amount)
{
    if (!device || !sceneTex || !historyTex || !dstSurface)
        return;

    if (!EffectManager::EnsureCustomBlurEffect(device))
        return;
    ID3DXEffect* fx = g_RenderTargetManager.g_CustomBlurEffect;
    if (!fx)
        return;

    ScopedStateBlock stateBlock(device);

    // Set RT
    IDirect3DSurface9* oldRT = nullptr;
    if (FAILED(device->GetRenderTarget(0, &oldRT)) || !oldRT)
        return;

    if (FAILED(device->SetRenderTarget(0, dstSurface)))
    {
        SAFE_RELEASE(oldRT);
        return;
    }

    D3DSURFACE_DESC sd{};
    dstSurface->GetDesc(&sd);
    D3DVIEWPORT9 vp{};
    vp.X = 0;
    vp.Y = 0;
    vp.Width = sd.Width;
    vp.Height = sd.Height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    device->SetViewport(&vp);

    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);

    // Params
    D3DXHANDLE hDiffuse = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    D3DXHANDLE hPrev = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
    D3DXHANDLE hBlend = fx->GetParameterByName(nullptr, "MotionBlurBlend");
    D3DXHANDLE hScale = fx->GetParameterByName(nullptr, "MotionBlurScale");
    D3DXHANDLE hDepth = fx->GetParameterByName(nullptr, "DEPTHBUFFER_TEXTURE");
    D3DXHANDLE hMask = fx->GetParameterByName(nullptr, "MOTIONBLUR_MASK_TEXTURE");
    if (!hMask) hMask = fx->GetParameterByName(nullptr, "MotionBlurMask");

    if (hDiffuse) fx->SetTexture(hDiffuse, sceneTex);
    if (hPrev) fx->SetTexture(hPrev, historyTex);
    if (hBlend) fx->SetFloat(hBlend, amount);
    // (MotionBlurScale intentionally unused)
    if (hDepth) fx->SetTexture(hDepth, g_RenderTargetManager.g_VTDepthTex ? g_RenderTargetManager.g_VTDepthTex
                                                                          : g_RenderTargetManager.g_DepthTex);
    // Prefer the game-provided VT gainmap/vignette (UVESVIGNETTE) as mask; fall back to BlurMask.png,
    // and then to the default 1x1 white created in OnDeviceReset.
    IDirect3DTexture9* maskTex = g_RenderTargetManager.g_VTGainMapTex
        ? g_RenderTargetManager.g_VTGainMapTex
        : g_RenderTargetManager.g_MotionBlurMaskTex;
    if (hMask) fx->SetTexture(hMask, maskTex);

    // Debug cycle (F9): (F8 is reserved in user's setup)
    // 0: normal
    // 1: solid tint (proves our pass is visible)
    // 2: visualize abs(curr-prev) (proves history timing)
    // 3: visualize history alpha (prev.a)
    // 4: visualize DEPTHBUFFER_TEXTURE (if bound)
    // 5: visualize MOTIONBLUR_MASK_TEXTURE (UVESVIGNETTE/BlurMask)
    // 6: visualize blurred texture (MOTIONBLUR_TEXTURE)
    // 7: visualize curr texture (DIFFUSEMAP_TEXTURE)
    static int s_dbgMode = 0;
    static SHORT s_prevKey = 0;
    const SHORT curKey = GetAsyncKeyState(VK_F9);
    if ((curKey & 0x8000) && !(s_prevKey & 0x8000))
        s_dbgMode = (s_dbgMode + 1) % 8;
    s_prevKey = curKey;

    // Force-tint while holding F10 to prove our composite draw is actually visible.
    // IMPORTANT: do not mutate s_dbgMode permanently (otherwise it "sticks" magenta).
    const bool forceTint = (GetAsyncKeyState(VK_F10) & 0x8000) != 0;

    const int effectiveDbgMode = forceTint ? 1 : s_dbgMode;
    const char* tech = "composite";
    if (effectiveDbgMode == 1) tech = "tint";
    else if (effectiveDbgMode == 2) tech = "dbg_diff";
    else if (effectiveDbgMode == 3) tech = "dbg_alpha";
    else if (effectiveDbgMode == 4) tech = "dbg_depth";
    else if (effectiveDbgMode == 5) tech = "dbg_mask";
    else if (effectiveDbgMode == 6) tech = "dbg_blur";
    else if (effectiveDbgMode == 7) tech = "dbg_curr";
    const char* const techs[] = {tech};
    if (!EffectManager::TrySetTechnique(fx, techs, sizeof(techs) / sizeof(techs[0])))
        goto cleanup;

    // No periodic logging here: this code runs at gameplay framerate; logging tanks FPS under DXVK.

    IDirect3DVertexDeclaration9* oldDecl = nullptr;
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->GetVertexDeclaration(&oldDecl);
    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->SetVertexDeclaration(g_RenderTargetManager.g_ScreenQuadDecl);

    UINT passes = 0;
    if (SUCCEEDED(fx->Begin(&passes, 0)))
    {
        for (UINT i = 0; i < passes; ++i)
        {
            if (SUCCEEDED(fx->BeginPass(i)))
            {
                RenderTargetManager::QuadVertex8 v[4]{};
                const float x[4] = {-1.0f, 1.0f, -1.0f, 1.0f};
                const float y[4] = {1.0f, 1.0f, -1.0f, -1.0f};
                const float u[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                const float w[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                for (int vi = 0; vi < 4; ++vi)
                {
                    v[vi].x = x[vi];
                    v[vi].y = y[vi];
                    v[vi].z = 0.0f;
                    v[vi].rhw = 1.0f;
                    const float uv4[4] = {u[vi], w[vi], 0.0f, 0.0f};
                    memcpy(v[vi].t0, uv4, sizeof(uv4));
                    memcpy(v[vi].t1, uv4, sizeof(uv4));
                    memcpy(v[vi].t2, uv4, sizeof(uv4));
                    memcpy(v[vi].t3, uv4, sizeof(uv4));
                    memcpy(v[vi].t4, uv4, sizeof(uv4));
                    memcpy(v[vi].t5, uv4, sizeof(uv4));
                    memcpy(v[vi].t6, uv4, sizeof(uv4));
                    memcpy(v[vi].t7, uv4, sizeof(uv4));
                }
                device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(RenderTargetManager::QuadVertex8));
                fx->EndPass();
            }
        }
        fx->End();
    }

    if (g_RenderTargetManager.g_ScreenQuadDecl)
        device->SetVertexDeclaration(oldDecl);
    SAFE_RELEASE(oldDecl);

cleanup:
    device->SetRenderTarget(0, oldRT);
    SAFE_RELEASE(oldRT);
}
