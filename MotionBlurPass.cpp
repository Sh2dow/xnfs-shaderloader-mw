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

    // Source for blur is FINAL image copy (post-VT)
    IDirect3DTexture9* src = g_RenderTargetManager.g_SceneCopyTex;
    if (!src)
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

    // Run blur: src(post-VT) -> dst(history)
    RenderBlurPass(device, src, dstTex, dstSurf);

    // Publish history for VT sampling (next frame / same frame if timing allows)
    g_RenderTargetManager.g_CurrentBlurTex     = dstTex;
    g_RenderTargetManager.g_CurrentBlurSurface = dstSurf;
    g_MotionBlurHistoryReady = true;

    // flip for next frame
    g_RenderTargetManager.g_UseTexA = !g_RenderTargetManager.g_UseTexA;

    g_MotionBlurAmount = 0.20f;
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
    printf_s("[Blur] hDiffuse=%p\n", hDiffuse);

    if (hDiffuse)
        fx->SetTexture(hDiffuse, srcTex);
    if (hDepth)
        fx->SetTexture(hDepth, g_RenderTargetManager.g_DepthTex);
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
    if (hBlend)
        fx->SetFloat(hBlend, g_MotionBlurAmount);

    {
        const char* const techs[] = {"blur"};
        if (!EffectManager::TrySetTechnique(fx, techs, sizeof(techs) / sizeof(techs[0])))
        {
            printf_s("[Blur] TrySetTechnique failed\n");
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
        printf_s("[Blur] passes=%u\n", passes);
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
    g_RenderTargetManager.g_CurrentBlurSurface = g_RenderTargetManager.g_MotionBlurSurfaceA;
}

void __cdecl MotionBlurPass::RenderBlurOverride(
    IDirect3DDevice9* device,
    IDirect3DBaseTexture9* src0,
    IDirect3DBaseTexture9* src1,
    IDirect3DSurface9* dstSurface)
{
    if (!device || !src0)
        return;

    if (!EffectManager::EnsureCustomBlurEffect(device))
        return;

    ID3DXEffect* fx = g_RenderTargetManager.g_CustomBlurEffect;
    if (!fx)
        return;

    // Preserve device state for the engine pipeline.
    ScopedStateBlock stateBlock(device);

    if (g_MotionBlurAmount <= 0.0f)
    {
        if (dstSurface)
        {
            IDirect3DTexture9* srcTex = nullptr;
            IDirect3DSurface9* srcSurf = nullptr;
            if (src0 && SUCCEEDED(src0->QueryInterface(__uuidof(IDirect3DTexture9),
                                                       reinterpret_cast<void**>(&srcTex))) &&
                srcTex && SUCCEEDED(srcTex->GetSurfaceLevel(0, &srcSurf)))
            {
                device->StretchRect(srcSurf, nullptr, dstSurface, nullptr, D3DTEXF_NONE);
            }
            SAFE_RELEASE(srcSurf);
            SAFE_RELEASE(srcTex);
        }
        g_MotionBlurHistoryReady = false;
        g_RenderTargetManager.g_CurrentBlurTex = nullptr;
        g_RenderTargetManager.g_CurrentBlurSurface = nullptr;
        return;
    }

    IDirect3DSurface9* targetSurf = g_RenderTargetManager.g_UseTexA
        ? g_RenderTargetManager.g_BlurHistorySurfA
        : g_RenderTargetManager.g_BlurHistorySurfB;
    IDirect3DTexture9* targetTex = g_RenderTargetManager.g_UseTexA
        ? g_RenderTargetManager.g_BlurHistoryTexA
        : g_RenderTargetManager.g_BlurHistoryTexB;
    if (!targetSurf || !targetTex)
        return;

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
        fx->SetTexture(hDepth, g_RenderTargetManager.g_DepthTex);

    D3DXHANDLE hPrev = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
    if (hPrev && src1)
        fx->SetTexture(hPrev, src1);

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

    const char* const techs[] = {"blur"};
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
