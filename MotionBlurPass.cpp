#include "MotionBlurPass.h"

#include <cstring>
#include <string>

#include "Globals.h"
#include "Hooks.h"
#include "EffectManager.h"
#include "RenderTargetManager.h"
#include "ScopedStateBlock.h"
#include "Validators.h"

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name);

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

__declspec(noinline)
void* SafeGetVisualTreatment()
{
    __try
    {
        using GetVT = void* (__cdecl*)();
        auto getVT = reinterpret_cast<GetVT>(0x006DFAF0);
        return getVT();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return nullptr;
    }
}

void __cdecl MotionBlurPass::CustomMotionBlurHook()
{
    int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    if (gameFlow < 3 || g_RenderTargetManager.g_DeviceResetInProgress)
        return;

    // Try to bind the live visual treatment effect directly from pVisualTreatmentPlat.
    {
        void* vt = nullptr;
        void* vtPtr = *reinterpret_cast<void**>(pVisualTreatmentPlat_ADDRESS);
        if (vtPtr && !IsBadReadPtr(vtPtr, sizeof(void*)))
            vt = vtPtr;
        if (!vt)
        {
            vt = SafeGetVisualTreatment();
        }
        static bool vtLogged = false;
        static ID3DXEffect* cachedFx = nullptr;
        if (vt)
        {
            if (!IsBadReadPtr(vt, sizeof(void*)))
            {
                ID3DXEffect* fx = *reinterpret_cast<ID3DXEffect**>((char*)vt + 0x18C);
                if (!vtLogged)
                {
                    vtLogged = true;
                    printf_s("[Blur] VT base=%p fx@0x18C=%p valid=%d\n",
                             vt,
                             fx,
                             (IsLikelyEffectPointer(fx) && ProbeEffectHandles(fx)) ? 1 : 0);
                }
                if (!(IsLikelyEffectPointer(fx) && ProbeEffectHandles(fx)))
                {
                    // Fallback: scan the object for a plausible ID3DXEffect* (layout can vary).
                    fx = nullptr;
                    const unsigned char* base = reinterpret_cast<unsigned char*>(vt);
                    for (size_t off = 0; off + sizeof(void*) <= 0x800; off += sizeof(void*))
                    {
                        void* candidate = *reinterpret_cast<void* const*>(base + off);
                        if (IsLikelyEffectPointer(reinterpret_cast<ID3DXEffect*>(candidate)) &&
                            ProbeEffectHandles(reinterpret_cast<ID3DXEffect*>(candidate)))
                        {
                            fx = reinterpret_cast<ID3DXEffect*>(candidate);
                            static bool logged = false;
                            if (!logged)
                            {
                                logged = true;
                                printf_s("[Blur] ? VT effect found at offset 0x%zx (vt=%p fx=%p)\n",
                                         off, vt, fx);
                            }
                            break;
                        }
                        if (!fx && IsLikelyEffectPointer(reinterpret_cast<ID3DXEffect*>(candidate)))
                        {
                            cachedFx = reinterpret_cast<ID3DXEffect*>(candidate);
                        }
                    }
                    if (!fx && cachedFx && IsLikelyEffectPointer(cachedFx) && ProbeEffectHandles(cachedFx))
                    {
                        fx = cachedFx;
                        printf_s("[Blur] ? VT cached effect became valid (vt=%p fx=%p)\n", vt, fx);
                    }
                    if (!fx && (eFrameCounter % 120) == 0)
                        printf_s("[Blur] ? VT scan found no valid effect yet (vt=%p)\n", vt);
                }
                if (ProbeEffectHandles(fx))
                {
                    // Do not AddRef/Release here; the VT-owned effect lifetime is external.
                    // We only cache the raw pointer for binding.
                    g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"] = fx;
                    g_RenderTargetManager.g_LastReloadedFx = fx;

                    if (g_RenderTargetManager.g_CurrentBlurTex)
                        ReloadBlurBindings(fx, "IDI_VISUALTREATMENT_FX");
                }
            }
        }
    }

    // Force a rebind attempt so we can see if ReloadBlurBindings runs.
    if (eFrameCounter % 60 == 0)
    {
        printf_s("[BlurRebind] activeEffects=%zu lastFx=%p\n",
                 g_RenderTargetManager.g_ActiveEffects.size(),
                 g_RenderTargetManager.g_LastReloadedFx);
    }
    auto itRebind = g_RenderTargetManager.g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
    if (itRebind != g_RenderTargetManager.g_ActiveEffects.end() && itRebind->second)
    {
        ReloadBlurBindings(itRebind->second, "IDI_VISUALTREATMENT_FX");
    }
    else if (g_RenderTargetManager.g_LastReloadedFx)
    {
        ReloadBlurBindings(g_RenderTargetManager.g_LastReloadedFx, "IDI_VISUALTREATMENT_FX");
    }

    static bool dumpedListeners = false;
    static uint32_t lastHitLogFrame = 0;
    if (eFrameCounter != lastHitLogFrame)
    {
        lastHitLogFrame = eFrameCounter;
        printf_s("[Blur] hook hit\n");
    }
    if (g_CustomMotionBlurRanThisFrame || g_RenderTargetManager.g_DeviceResetInProgress)
        return;

    if (g_RenderTargetManager.g_DeviceResetInProgress)
        return;

    IDirect3DDevice9* device = g_Device;
    if (!device)
        return;

    // Restore all device state after our pass to avoid leaking render states.
    ScopedStateBlock stateBlock(device);
    uint8_t* motionBlurEnabled = reinterpret_cast<uint8_t*>(0x009017DC);
    uint32_t* motionBlurGate = reinterpret_cast<uint32_t*>(0x008F9218);
    uint8_t* gameFlag = reinterpret_cast<uint8_t*>(0x008F9B28);
    void** eViews = reinterpret_cast<void**>(EViewsBase_ADDRESS);
    if (!motionBlurEnabled || !gameFlag || !(*motionBlurEnabled) || !(*gameFlag))
        return;
    if (!motionBlurGate || *motionBlurGate == 0)
        return;
    g_MotionBlurAmount = 1.0f;
    if (eViews && *eViews && reinterpret_cast<uintptr_t>(*eViews) <= 0x10000)
        return;
    if (!dumpedListeners)
    {
        dumpedListeners = true;
        uint32_t* listBase = reinterpret_cast<uint32_t*>(0x009B37CC);
        uint32_t count = *reinterpret_cast<uint32_t*>(0x009B37D4);
        printf_s("[Blur] listeners: base=%p count=%u\n", listBase, count);
        const uint32_t maxDump = 32;
        for (uint32_t i = 0; i < count && i < maxDump; ++i)
        {
            void* obj = reinterpret_cast<void*>(listBase[i]);
            if (!obj || reinterpret_cast<uintptr_t>(obj) < 0x10000 ||
                IsBadReadPtr(obj, sizeof(void*)))
            {
                printf_s("[Blur] listener[%u]=%p (invalid)\n", i, obj);
                continue;
            }
            void** vtbl = *reinterpret_cast<void***>(obj);
            if (!vtbl || IsBadReadPtr(vtbl, sizeof(void*)))
            {
                printf_s("[Blur] listener[%u]=%p vtbl=%p (invalid)\n", i, obj, vtbl);
                continue;
            }
            void* fn = vtbl[3];
            printf_s("[Blur] listener[%u]=%p vtbl=%p fn+0x0C=%p\n", i, obj, vtbl, fn);
        }
        if (count > maxDump)
            printf_s("[Blur] listeners: truncated to %u entries\n", maxDump);
    }
    static uint32_t lastStateLogFrame = 0;
    if (eFrameCounter != lastStateLogFrame)
    {
        lastStateLogFrame = eFrameCounter;
        printf_s(
            "[Blur] hook state: device=%p blurSurface=%p sceneRT=%p lastScene=%p lastSceneFrame=%u frame=%u mb=%u gf=%u eView=%p\n",
            device,
            g_RenderTargetManager.g_CurrentBlurSurface,
            g_SceneTargetThisFrame,
            g_RenderTargetManager.g_LastSceneFullSurface,
            g_LastSceneFullFrame,
            eFrameCounter,
            motionBlurEnabled ? *motionBlurEnabled : 0,
            gameFlag ? *gameFlag : 0,
            (eViews && *eViews) ? *eViews : nullptr);
    }
    if (!device || !g_RenderTargetManager.g_MotionBlurSurfaceA)
        return;


    g_RenderTargetManager.g_UseTexA = true;
    g_RenderTargetManager.g_CurrentBlurTex = g_RenderTargetManager.g_MotionBlurTexA;
    g_RenderTargetManager.g_CurrentBlurSurface = g_RenderTargetManager.g_MotionBlurSurfaceA;

    if (!g_RenderTargetManager.g_LastSceneFullSurface)
        return;
    if (g_LastSceneFullFrame + 1 < eFrameCounter)
        return;

    IDirect3DSurface9* src = g_RenderTargetManager.g_LastSceneFullSurface;
    if (!src)
        return;

    if (src != g_RenderTargetManager.g_CurrentBlurSurface)
    {
        device->StretchRect(src, nullptr, g_RenderTargetManager.g_CurrentBlurSurface, nullptr, D3DTEXF_LINEAR);
    }

    RenderBlurPass(device);
    auto it = g_RenderTargetManager.g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
    if (it != g_RenderTargetManager.g_ActiveEffects.end() && it->second)
        ReloadBlurBindings(it->second, "IDI_VISUALTREATMENT_FX");
    g_CustomMotionBlurRanThisFrame = true;
}

void __cdecl MotionBlurPass::RenderBlurPass(IDirect3DDevice9* device)
{
    if (!device || !g_RenderTargetManager.g_CurrentBlurTex) return;
    if (!device || !g_RenderTargetManager.g_CurrentBlurTex) return;

    if (!EffectManager::EnsureCustomBlurEffect(device))
    {
        printf_s("[Blur] EnsureCustomBlurEffect failed\n");
        return;
    }
    ID3DXEffect* fx = g_RenderTargetManager.g_CustomBlurEffect;

    IDirect3DSurface9* oldRT = nullptr;
    if (FAILED(device->GetRenderTarget(0, &oldRT)) || !oldRT) return;

    D3DVIEWPORT9 savedVP{};
    device->GetViewport(&savedVP);

    IDirect3DTexture9* srcTex = g_RenderTargetManager.g_MotionBlurTexA;
    IDirect3DTexture9* dstTex = g_RenderTargetManager.g_MotionBlurTexB;
    IDirect3DSurface9* dstSurface = g_RenderTargetManager.g_MotionBlurSurfaceB;
    if (!srcTex || !dstTex || !dstSurface)
    {
        SAFE_RELEASE(oldRT);
        return;
    }

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
    D3DXHANDLE hTexel = fx->GetParameterByName(nullptr, "BlurTexelSize");
    D3DXHANDLE oldTech = fx->GetCurrentTechnique();

    IDirect3DBaseTexture9* oldDiffuse = nullptr;
    if (hDiffuse)
        fx->GetTexture(hDiffuse, &oldDiffuse);
    printf_s("[Blur] hDiffuse=%p\n", hDiffuse);

    if (hDiffuse)
        fx->SetTexture(hDiffuse, srcTex);
    if (hTexel)
    {
        const float blurScale = 2.0f;
        fx->SetVector(hTexel, &D3DXVECTOR4(blurScale / (float)sd.Width,
                                           blurScale / (float)sd.Height, 0.0f, 0.0f));
    }

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
                const float dx = 1.0f / (float)sd.Width;
                const float dy = 0.0f;
                const float offs[8] = {-4.0f, -2.5f, -1.5f, -0.5f, 0.5f, 1.5f, 2.5f, 4.0f};
                for (int vi = 0; vi < 4; ++vi)
                {
                    vertices[vi].x = x[vi];
                    vertices[vi].y = y[vi];
                    vertices[vi].z = 0.0f;
                    vertices[vi].rhw = 1.0f;
                    float u0 = uBase[vi];
                    float v0 = vBase[vi];
                    const float uv0[4] = {u0 + offs[0] * dx, v0 + offs[0] * dy, 0.0f, 0.0f};
                    const float uv1[4] = {u0 + offs[1] * dx, v0 + offs[1] * dy, 0.0f, 0.0f};
                    const float uv2[4] = {u0 + offs[2] * dx, v0 + offs[2] * dy, 0.0f, 0.0f};
                    const float uv3[4] = {u0 + offs[3] * dx, v0 + offs[3] * dy, 0.0f, 0.0f};
                    const float uv4[4] = {u0 + offs[4] * dx, v0 + offs[4] * dy, 0.0f, 0.0f};
                    const float uv5[4] = {u0 + offs[5] * dx, v0 + offs[5] * dy, 0.0f, 0.0f};
                    const float uv6[4] = {u0 + offs[6] * dx, v0 + offs[6] * dy, 0.0f, 0.0f};
                    const float uv7[4] = {u0 + offs[7] * dx, v0 + offs[7] * dy, 0.0f, 0.0f};
                    memcpy(vertices[vi].t0, uv0, sizeof(uv0));
                    memcpy(vertices[vi].t1, uv1, sizeof(uv1));
                    memcpy(vertices[vi].t2, uv2, sizeof(uv2));
                    memcpy(vertices[vi].t3, uv3, sizeof(uv3));
                    memcpy(vertices[vi].t4, uv4, sizeof(uv4));
                    memcpy(vertices[vi].t5, uv5, sizeof(uv5));
                    memcpy(vertices[vi].t6, uv6, sizeof(uv6));
                    memcpy(vertices[vi].t7, uv7, sizeof(uv7));
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
    g_RenderTargetManager.g_CurrentBlurSurface = g_RenderTargetManager.g_MotionBlurSurfaceB;
}

void __cdecl MotionBlurPass::RenderBlurCompositePass(IDirect3DDevice9* device)
{
    if (!device || !g_RenderTargetManager.g_CurrentBlurTex) return;

    static bool s_InComposite = false;
    if (s_InComposite)
        return;
    s_InComposite = true;

    if (!EffectManager::EnsureCustomBlurEffect(device))
    {
        printf_s("[Blur] EnsureCustomBlurEffect failed\n");
        s_InComposite = false;
        return;
    }
    ID3DXEffect* fx = g_RenderTargetManager.g_CustomBlurEffect;

    D3DVIEWPORT9 savedVP{};
    device->GetViewport(&savedVP);

    IDirect3DSurface9* oldRT = nullptr;
    if (FAILED(device->GetRenderTarget(0, &oldRT)) || !oldRT)
    {
        s_InComposite = false;
        return;
    }

    D3DSURFACE_DESC rtDesc{};
    oldRT->GetDesc(&rtDesc);
    if (g_RenderTargetManager.g_CurrentBlurSurface)
    {
        static uint32_t lastSizeLogFrame = 0;
        if (eFrameCounter != lastSizeLogFrame)
        {
            lastSizeLogFrame = eFrameCounter;
            D3DSURFACE_DESC blurDesc{};
            if (SUCCEEDED(g_RenderTargetManager.g_CurrentBlurSurface->GetDesc(&blurDesc)))
            {
                printf_s("[Blur] sizes: rt=%ux%u blur=%ux%u\n",
                         rtDesc.Width, rtDesc.Height, blurDesc.Width, blurDesc.Height);
            }
        }
    }

    D3DVIEWPORT9 vp{};
    vp.X = 0;
    vp.Y = 0;
    vp.Width = rtDesc.Width;
    vp.Height = rtDesc.Height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    device->SetViewport(&vp);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    D3DXHANDLE hDiffuse = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    D3DXHANDLE hTexel = fx->GetParameterByName(nullptr, "BlurTexelSize");
    D3DXHANDLE oldTech = fx->GetCurrentTechnique();

    IDirect3DBaseTexture9* oldDiffuse = nullptr;
    if (hDiffuse)
        fx->GetTexture(hDiffuse, &oldDiffuse);
    printf_s("[Blur] hDiffuse(copy)=%p\n", hDiffuse);

    if (hDiffuse)
        fx->SetTexture(hDiffuse, g_RenderTargetManager.g_CurrentBlurTex);
    if (hTexel)
    {
        const float blurScale = 1.0f;
        fx->SetVector(hTexel, &D3DXVECTOR4(blurScale / (float)rtDesc.Width,
                                           blurScale / (float)rtDesc.Height, 0.0f, 0.0f));
    }
    {
        const char* const techs[] = {"copy"};
        if (!EffectManager::TrySetTechnique(fx, techs, sizeof(techs) / sizeof(techs[0])))
        {
            printf_s("[Blur] TrySetTechnique(copy) failed\n");
            device->SetRenderTarget(0, oldRT);
            device->SetViewport(&savedVP);
            SAFE_RELEASE(oldRT);
            s_InComposite = false;
            return;
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
        printf_s("[Blur] Begin(copy) failed\n");
    }
    else
    {
        printf_s("[Blur] passes(copy)=%u\n", passes);
        for (UINT i = 0; i < passes; ++i)
        {
            if (SUCCEEDED(fx->BeginPass(i)))
            {
                static RenderTargetManager::QuadVertex8 vertices[4];
                const float u[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                const float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
                const float x[4] = {-1.0f, 1.0f, -1.0f, 1.0f};
                const float y[4] = {1.0f, 1.0f, -1.0f, -1.0f};
                for (int vi = 0; vi < 4; ++vi)
                {
                    vertices[vi].x = x[vi];
                    vertices[vi].y = y[vi];
                    vertices[vi].z = 0.0f;
                    vertices[vi].rhw = 1.0f;
                    const float uv[4] = {u[vi], v[vi], 0.0f, 0.0f};
                    memcpy(vertices[vi].t0, uv, sizeof(uv));
                    memcpy(vertices[vi].t1, uv, sizeof(uv));
                    memcpy(vertices[vi].t2, uv, sizeof(uv));
                    memcpy(vertices[vi].t3, uv, sizeof(uv));
                    memcpy(vertices[vi].t4, uv, sizeof(uv));
                    memcpy(vertices[vi].t5, uv, sizeof(uv));
                    memcpy(vertices[vi].t6, uv, sizeof(uv));
                    memcpy(vertices[vi].t7, uv, sizeof(uv));
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

    if (hDiffuse)
        fx->SetTexture(hDiffuse, oldDiffuse);
    if (oldDiffuse)
        oldDiffuse->Release();
    if (oldTech)
        fx->SetTechnique(oldTech);

    device->SetRenderTarget(0, oldRT);
    device->SetViewport(&savedVP);
    SAFE_RELEASE(oldRT);
    s_InComposite = false;
}
