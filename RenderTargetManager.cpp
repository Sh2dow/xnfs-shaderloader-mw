#include "globals.h"
#include <d3dx9effect.h>
#include "Validators.h"

void RenderTargetManager::ReloadBlurBindings(ID3DXEffect* fx, const std::string& name)
{
    if (!fx)
    {
        printf_s("[BlurRebind] ❌ fx is null\n");
        return;
    }

    if (!g_RenderTargetManager.g_CurrentBlurTex || IsBadReadPtr(g_RenderTargetManager.g_CurrentBlurTex,
                                                                sizeof(IDirect3DTexture9)))
    {
        printf_s("[BlurRebind] ❌ g_CurrentBlurTex is null or invalid\n");
        return;
    }

    if (g_PendingVisualReset)
    {
        printf_s("[BlurRebind] ⚠️ Skipped during device reset\n");
        return;
    }

    // Basic texture setup
    fx->SetTexture(fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE"), g_RenderTargetManager.g_CurrentBlurTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP1_TEXTURE"), g_RenderTargetManager.g_ExposureTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP2_TEXTURE"), g_RenderTargetManager.g_VignetteTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP3_TEXTURE"), g_RenderTargetManager.g_BloomLUTTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP4_TEXTURE"), g_RenderTargetManager.g_DofTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE"), g_RenderTargetManager.g_DepthTex);

    // BlurParams
    D3DXHANDLE h = fx->GetParameterByName(nullptr, "BlurParams");
    if (h)
    {
        D3DXVECTOR4 blurVec(0.5f, 0.2f, 1.0f, 0.0f);
        fx->SetVector(h, &blurVec);
    }

    if (name == "IDI_VISUALTREATMENT_FX")
    {
        D3DXHANDLE tech = fx->GetTechniqueByName("visualtreatment_branching");
        if (tech && SUCCEEDED(fx->ValidateTechnique(tech)))
            fx->SetTechnique(tech);

        fx->SetFloat(fx->GetParameterByName(nullptr, "g_fBloomScale"), 1.0f);
        fx->SetFloat(fx->GetParameterByName(nullptr, "VisualEffectBrightness"), 1.0f);
        fx->SetFloat(fx->GetParameterByName(nullptr, "Desaturation"), 0.0f);

        float dofParams[4] = {1.0f, 0.1f, 0.1f, 0.0f};
        fx->SetFloatArray(fx->GetParameterByName(nullptr, "DepthOfFieldParams"), dofParams, 4);

        fx->SetBool(fx->GetParameterByName(nullptr, "bDepthOfFieldEnabled"), TRUE);
        fx->SetBool(fx->GetParameterByName(nullptr, "bHDREnabled"), TRUE);
        fx->SetFloat(fx->GetParameterByName(nullptr, "g_fAdaptiveLumCoeff"), 1.0f);
    }

    if (FAILED(fx->CommitChanges()))
        printf_s("[BlurRebind] ⚠️ CommitChanges failed\n");
}


void RenderTargetManager::RenderBlurPass(IDirect3DDevice9* device)
{
    if (!device || !g_RenderTargetManager.g_CurrentBlurTex)
        return;

    // Get active shader
    auto it = g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
    if (it == g_ActiveEffects.end() || !it->second || !IsValidShaderPointer(it->second))
        return;
    
    auto fx = it->second;

    // Backup current render target
    IDirect3DSurface9* oldRT = nullptr;
    if (FAILED(device->GetRenderTarget(0, &oldRT)) || !oldRT)
        return;

    IDirect3DTexture9* srcTex = g_RenderTargetManager.g_UseTexA
                                    ? g_RenderTargetManager.g_MotionBlurTexA
                                    : g_RenderTargetManager.g_MotionBlurTexB;
    IDirect3DTexture9* dstTex = g_RenderTargetManager.g_UseTexA
                                    ? g_RenderTargetManager.g_MotionBlurTexB
                                    : g_RenderTargetManager.g_MotionBlurTexA;

    IDirect3DSurface9* dstSurface = nullptr;
    if (FAILED(dstTex->GetSurfaceLevel(0, &dstSurface)) || !dstSurface)
    {
        SAFE_RELEASE(oldRT);
        return;
    }

    // Set new render target
    device->SetRenderTarget(0, dstSurface);
    // D3DVIEWPORT9 vp = {0, 0, g_Width, g_Height, 0.0f, 1.0f};
    D3DVIEWPORT9 vp = {};
    if (SUCCEEDED(device->GetViewport(&vp)))
    {
        // Use actual viewport for correct dimensions
        device->SetViewport(&vp);
    }
    device->SetViewport(&vp);

    // Debug color (pink) to test visibility
    device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 0, 255), 1.0f, 0);

    // Setup shader
    if (FAILED(fx->GetEffect()->SetTexture("DIFFUSEMAP_TEXTURE", srcTex)))
        printf_s("⚠️ Failed to set DIFFUSEMAP_TEXTURE\n");

    D3DXVECTOR4 blurParams(0.5f, 0.0005f, 0.0f, 0.0f);
    if (FAILED(fx->GetEffect()->SetVector("BlurParams", &blurParams)))
        printf_s("⚠️ Failed to set BlurParams\n");

    if (FAILED(fx->GetEffect()->CommitChanges()))
        printf_s("⚠️ CommitChanges failed\n");

    // Validate technique
    D3DXHANDLE tech = fx->GetEffect()->GetTechniqueByName("visualtreatment_branching");

    if (!tech || FAILED(fx->GetEffect()->SetTechnique(tech)))
    {
        printf_s("⚠️ Missing or invalid technique: visualtreatment_branching\n");
        goto cleanup;
    }

    // Render state setup
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); // ✅ No blending
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE); // ✅ Irrelevant since blend is off
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE,
                           D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                           D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

    // test Alpha or additive blending — the shader already applies HDR, tone, vignette, bloom,
    // so it's better to avoid these params
    // device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    // device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);


    // Draw full-screen quad
    UINT passes = 0;
    if (SUCCEEDED(fx->GetEffect()->Begin(&passes, 0)))
    {
        for (UINT i = 0; i < passes; ++i)
        {
            if (SUCCEEDED(fx->GetEffect()->BeginPass(i)))
            {
                device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
                device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, screenQuadVerts,
                                        sizeof(Vertex));
                printf_s("[XNFS] ✅ DrawPrimitiveUP executed\n");
                fx->GetEffect()->EndPass();
            }
            else
            {
                printf_s("[XNFS] ⚠️ BeginPass(%u) failed\n", i);
            }
        }
        fx->GetEffect()->End();
    }
    else
    {
        printf_s("[XNFS] ❌ fx->Begin failed\n");
    }


cleanup:
    device->SetRenderTarget(0, oldRT);
    SAFE_RELEASE(dstSurface);
    SAFE_RELEASE(oldRT);

    g_RenderTargetManager.g_UseTexA = !g_RenderTargetManager.g_UseTexA;
    g_RenderTargetManager.g_CurrentBlurTex = dstTex;

    SAFE_RELEASE(g_RenderTargetManager.g_CurrentBlurSurface);
    dstTex->GetSurfaceLevel(0, &g_RenderTargetManager.g_CurrentBlurSurface);
}