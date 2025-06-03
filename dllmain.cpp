// dllmain.cpp - Backbuffer hook for NFS MW (2005)
#include <windows.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include "includes/injector/injector.hpp"
#include <d3dx9effect.h>
#include <cstdio>
#include <psapi.h>  // Required for MODULEINFO and GetModuleInformation
#include <d3d9.h>
#include <unordered_map>
#include <unordered_set>
#include "globals.h"
#include "Hooks.h"
#include "Validators.h"
#include "MinHook.h"
#pragma comment(lib, "d3d9.lib")

// -------------------- GLOBALS --------------------
#define CHECKMARK(x) ((x) ? "OK" : "MISSING")

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

HMODULE g_hModule = nullptr;
IDirect3DTexture9* g_MotionBlurTex = nullptr;
bool g_DeviceResetInProgress = false;
int g_ApplyDelayCounter = 0;
bool g_ApplyScheduled = false;
int g_ApplyGraphicsTriggerDelay = 0;
static size_t lastCount = 0;

std::unordered_map<std::string, LPD3DXEFFECT> g_ActiveEffects;

using EndScene_t = HRESULT(WINAPI*)(LPDIRECT3DDEVICE9);
EndScene_t oEndScene = nullptr;
Present_t oPresent = nullptr;

void* g_ApplyGraphicsManagerThis = nullptr;

ID3DXEffect* g_LastReloadedFx = nullptr;


typedef void (__fastcall*ApplyGraphicsSettingsFn)(void* ecx, void* edx, void* arg1);
ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr; // ✅ definition

typedef int (__thiscall*ApplyGraphicsManagerMain_t)(void* thisptr);
ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal = nullptr; // ✅ definition

typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);

// IVisualTreatment_Reset block
void* g_LastEView = nullptr;

typedef void (__thiscall*UpdateFunc)(void* thisptr, void* eView);
typedef void (__cdecl*FrameRenderFn)();
FrameRenderFn ForceFrameRender = (FrameRenderFn)0x006DE300;
void** g_pVisualTreatment = (void**)0x00982AF0;

using IVisualTreatment_ResetFn = void(__thiscall*)(void* thisPtr);
// ✅ Header declaration only:
IVisualTreatment_ResetFn IVisualTreatment_Reset = (IVisualTreatment_ResetFn)0x0073DE50;

// IVisualTreatment_Reset block end

IDirect3DTexture9* GetDummyWhiteTexture(IDirect3DDevice9* device)
{
    static IDirect3DTexture9* tex = nullptr;
    if (!tex)
    {
        device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, nullptr);
        D3DLOCKED_RECT lr;
        if (SUCCEEDED(tex->LockRect(0, &lr, nullptr, 0)))
        {
            *((DWORD*)lr.pBits) = 0xFFFFFFFF;
            tex->UnlockRect(0);
        }
    }
    return tex;
}

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name = "")
{
    if (!fx)
    {
        printf_s("[BlurRebind] ❌ fx is null\n");
        return;
    }

    if (!g_CurrentBlurTex || IsBadReadPtr(g_CurrentBlurTex, sizeof(IDirect3DTexture9)))
    {
        printf_s("[BlurRebind] ❌ g_CurrentBlurTex is null or invalid\n");
        return;
    }

    // Optional: detect if we're in the middle of Reset
    if (g_DeviceResetInProgress)
    {
        printf_s("[BlurRebind] ⚠️ Skipped during device reset\n");
        return;
    }

    D3DXEFFECT_DESC desc;
    if (SUCCEEDED(fx->GetDesc(&desc)))
    {
        printf_s("[BlurRebind] Shader has %u techniques and %u parameters\n",
                 desc.Techniques, desc.Parameters);
    }

    // Set texture to DIFFUSEMAP_TEXTURE safely
    D3DXHANDLE hDiffuseMapTex = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    if (hDiffuseMapTex)
    {
        if (FAILED(fx->SetTexture("DIFFUSEMAP_TEXTURE", g_CurrentBlurTex)))
        {
            printf_s("[BlurRebind] ⚠️ SetTexture failed for DIFFUSEMAP_TEXTURE\n");
        }
    }
    else
    {
        printf_s("[BlurRebind] ⚠️ Shader missing parameter DIFFUSEMAP_TEXTURE\n");
    }

    static IDirect3DTexture9* dummy1x1 = GetDummyWhiteTexture(g_Device); // helper function
    fx->SetTexture("MISCMAP1_TEXTURE", g_GainMapTex ? g_GainMapTex : dummy1x1);
    fx->SetTexture("MISCMAP2_TEXTURE", g_VignetteTex ? g_VignetteTex : dummy1x1);
    fx->SetTexture("MISCMAP3_TEXTURE", g_BloomTex ? g_BloomTex : dummy1x1);
    fx->SetTexture("MISCMAP4_TEXTURE", g_DofTex ? g_DofTex : dummy1x1);
    fx->SetTexture("HEIGHTMAP_TEXTURE", g_LinearDepthTex ? g_LinearDepthTex : dummy1x1);
    
    // fx->SetTexture("MISCMAP1_TEXTURE", g_ExposureTex);
    // fx->SetTexture("MISCMAP2_TEXTURE", g_VignetteTex);
    // fx->SetTexture("MISCMAP3_TEXTURE", g_BloomLUTTex);
    // fx->SetTexture("MISCMAP4_TEXTURE", g_DofTex);
    // fx->SetTexture("HEIGHTMAP_TEXTURE", g_DepthTex);

    // Set BlurParams vector safely
    D3DXHANDLE h = nullptr;

    h = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    if (h) fx->SetTexture(h, g_CurrentBlurTex);

    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP1_TEXTURE"), g_ExposureTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP2_TEXTURE"), g_VignetteTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP3_TEXTURE"), g_BloomLUTTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP4_TEXTURE"), g_DofTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE"), g_DepthTex);

    h = fx->GetParameterByName(nullptr, "BlurParams");
    if (h) fx->SetVector(h, new D3DXVECTOR4(0.5f, 0.2f, 1.0f, 0.0f));

    if (name == "IDI_VISUALTREATMENT_FX")
    {
        D3DXHANDLE tech = fx->GetTechniqueByName("visualtreatment_branching");
        if (tech && SUCCEEDED(fx->ValidateTechnique(tech)))
            fx->SetTechnique(tech);

        h = fx->GetParameterByName(nullptr, "g_fBloomScale");
        if (h) fx->SetFloat(h, 1.0f);

        h = fx->GetParameterByName(nullptr, "VisualEffectBrightness");
        if (h) fx->SetFloat(h, 1.0f);

        h = fx->GetParameterByName(nullptr, "Desaturation");
        if (h) fx->SetFloat(h, 0.0f);

        h = fx->GetParameterByName(nullptr, "DepthOfFieldParams");
        if (h) fx->SetFloatArray(h, new float[4]{1.0f, 0.1f, 0.1f, 0.0f}, 4);

        h = fx->GetParameterByName(nullptr, "bDepthOfFieldEnabled");
        if (h) fx->SetBool(h, TRUE);

        h = fx->GetParameterByName(nullptr, "bHDREnabled");
        if (h) fx->SetBool(h, TRUE);

        h = fx->GetParameterByName(nullptr, "g_fAdaptiveLumCoeff");
        if (h) fx->SetFloat(h, 1.0f);

        fx->CommitChanges();
    }
}

// Hooked D3DXCreateEffectFromResourceA
HRESULT WINAPI HookedCreateFromResource(
    LPDIRECT3DDEVICE9 device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors)
{
    if (!device || !pResource || !RealCreateFromResource)
    {
        printf_s("[XNFS] ⚠️ Skipping invalid shader load — device or pResource is null\n");
        if (outEffect) *outEffect = nullptr;
        return E_FAIL;
    }

    // Only process specific shader
    bool isVisualFx = strcmp(pResource, "IDI_VISUALTREATMENT_FX") == 0;
    if (isVisualFx)
        printf_s("[XNFS] 🎯 HookedCreateFromResource called for: %s\n", pResource);

    SetGameDevice(device);

    HRESULT hr = RealCreateFromResource(
        device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);

    if (FAILED(hr) || !outEffect || !*outEffect)
    {
        if (isVisualFx)
            printf_s("[XNFS] ❌ Shader creation failed or returned null (hr=0x%08X)\n", hr);
        return hr;
    }

    if (isVisualFx)
    {
        g_ActiveEffects[pResource] = *outEffect;
        printf_s("✅ Shader created and tracked: %s (%p)\n", pResource, *outEffect);

        printf_s("[XNFS] ?? ReloadBlurBindings\n");
        ReloadBlurBindings(*outEffect, pResource);
    }

    return hr;
}

void OnDeviceReset(LPDIRECT3DDEVICE9 device)
{
    if (!device) return;

    g_DeviceResetInProgress = true;

    D3DVIEWPORT9 vp;
    if (SUCCEEDED(device->GetViewport(&vp)))
    {
        g_Width = vp.Width;
        g_Height = vp.Height;

        SAFE_RELEASE(g_MotionBlurTexA);
        SAFE_RELEASE(g_MotionBlurTexB);
        SAFE_RELEASE(g_CurrentBlurSurface);

        device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                              D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTexA, nullptr);
        device->CreateTexture(g_Width, g_Height, 1, D3DUSAGE_RENDERTARGET,
                              D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_MotionBlurTexB, nullptr);

        g_UseTexA = true;
        g_CurrentBlurTex = g_MotionBlurTexA;
        g_CurrentBlurTex->GetSurfaceLevel(0, &g_CurrentBlurSurface);

        printf_s("[OnDeviceReset] Recreated motion blur ping-pong targets (%ux%u)\n", g_Width, g_Height);
    }

    g_DeviceResetInProgress = false; // ✅ Move before reload

    if (g_CurrentBlurTex)
    {
        for (auto& [name, fx] : g_ActiveEffects)
        {
            if (strcmp(name.c_str(), "IDI_VISUALTREATMENT_FX") == 0)
            {
                printf_s("[XNFS] ?? ReloadBlurBindings\n");
                ReloadBlurBindings(fx, name.c_str());
            }
        }
    }
    else
    {
        printf_s("[OnDeviceReset] ⚠️ Skipping ReloadBlurBindings due to null g_CurrentBlurTex\n");
    }
}

void OnDeviceLost()
{
    g_DeviceResetInProgress = true;

    SAFE_RELEASE(g_MotionBlurTexA);
    SAFE_RELEASE(g_MotionBlurTexB);
    SAFE_RELEASE(g_CurrentBlurSurface);
    g_CurrentBlurTex = nullptr;

    printf_s("[OnDeviceLost] Released motion blur resources\n");
}

void RenderBlurPass(IDirect3DDevice9* device)
{
    if (!device || !g_CurrentBlurTex)
        return;

    // Get active shader
    auto it = g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
    if (it == g_ActiveEffects.end() || !it->second || !IsValidShaderPointer(it->second))
        return;

    ID3DXEffect* fx = it->second;

    // Backup current render target
    IDirect3DSurface9* oldRT = nullptr;
    if (FAILED(device->GetRenderTarget(0, &oldRT)) || !oldRT)
        return;

    IDirect3DTexture9* srcTex = g_UseTexA ? g_MotionBlurTexA : g_MotionBlurTexB;
    IDirect3DTexture9* dstTex = g_UseTexA ? g_MotionBlurTexB : g_MotionBlurTexA;

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
    if (FAILED(fx->SetTexture("DIFFUSEMAP_TEXTURE", srcTex)))
        printf_s("⚠️ Failed to set DIFFUSEMAP_TEXTURE\n");

    D3DXVECTOR4 blurParams(0.5f, 0.0005f, 0.0f, 0.0f);
    if (FAILED(fx->SetVector("BlurParams", &blurParams)))
        printf_s("⚠️ Failed to set BlurParams\n");

    if (FAILED(fx->CommitChanges()))
        printf_s("⚠️ CommitChanges failed\n");

    // Validate technique
    D3DXHANDLE tech = fx->GetTechniqueByName("visualtreatment_branching");

    if (!tech || FAILED(fx->SetTechnique(tech)))
    {
        printf_s("⚠️ Missing or invalid technique: visualtreatment_branching\n");
        goto cleanup;
    }

    // Render state setup
    // device->SetRenderState(D3DRS_ZENABLE, FALSE);
    // device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    // device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    // device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    // device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE,
                           D3DCOLORWRITEENABLE_RED |
                           D3DCOLORWRITEENABLE_GREEN |
                           D3DCOLORWRITEENABLE_BLUE |
                           D3DCOLORWRITEENABLE_ALPHA);

    // Draw full-screen quad
    UINT passes = 0;
    if (SUCCEEDED(fx->Begin(&passes, 0)))
    {
        for (UINT i = 0; i < passes; ++i)
        {
            if (SUCCEEDED(fx->BeginPass(i)))
            {
                device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
                device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, screenQuadVerts, sizeof(Vertex));
                printf_s("[XNFS] ✅ DrawPrimitiveUP executed\n");
                fx->EndPass();
            }
            else
            {
                printf_s("[XNFS] ⚠️ BeginPass(%u) failed\n", i);
            }
        }
        fx->End();
    }
    else
    {
        printf_s("[XNFS] ❌ fx->Begin failed\n");
    }


cleanup:
    device->SetRenderTarget(0, oldRT);
    SAFE_RELEASE(dstSurface);
    SAFE_RELEASE(oldRT);

    g_UseTexA = !g_UseTexA;
    g_CurrentBlurTex = dstTex;

    SAFE_RELEASE(g_CurrentBlurSurface);
    dstTex->GetSurfaceLevel(0, &g_CurrentBlurSurface);
}

HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params)
{
    printf_s("[hkReset] Called\n");
    OnDeviceLost();

    // This is mostly for D3DPOOL_MANAGED, but calling it after OnDeviceLost():
    device->EvictManagedResources();

    HRESULT hr = oReset(device, params);
    if (SUCCEEDED(hr))
    {
        OnDeviceReset(device);
    }
    return hr;
}

void ReplaceShaderSlot(void* objectBase, int offset, ID3DXEffect* replacement)
{
    ID3DXEffect** slot = (ID3DXEffect**)((BYTE*)objectBase + offset);
    if (!IsBadWritePtr(slot, sizeof(void*)))
    {
        if (*slot != replacement)
        {
            if (*slot) (*slot)->Release();
            *slot = replacement;
            replacement->AddRef();
        }
    }
}

void VisualTreatment_Reset()
{
    if (g_pVisualTreatment && IsValidThis(*g_pVisualTreatment))
    {
        void* vt = *g_pVisualTreatment;

        ID3DXEffect** fxSlot = (ID3DXEffect**)((char*)vt + 0x18C);
        if (IsValidShaderPointer(*fxSlot))
        {
            printf_s("🔧 Releasing fx at +0x18C (%p)\n", *fxSlot);
            (*fxSlot)->Release();
            *fxSlot = nullptr;
        }

        __try
        {
            IVisualTreatment_Reset(vt);
            printf_s("[HotReload] ✅ Reset() called on IVisualTreatment at %p\n", vt);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf_s("[HotReload] ❌ Exception while calling IVisualTreatment::Reset()\n");
        }
    }
    else
    {
        printf_s("[HotReload] ❌ g_pVisualTreatment invalid or null\n");
    }
}

bool ReplaceShaderSlot_RawEffect(
    BYTE* baseObject,
    int offset,
    ID3DXEffect* newRawFx,
    int slotIndex)
{
    void* slotPtr = (void*)(baseObject + offset);
    MEMORY_BASIC_INFORMATION mbi = {};

    // First, try to see if it’s already writable:
    if (VirtualQuery(slotPtr, &mbi, sizeof(mbi)) &&
        (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)) &&
        mbi.State == MEM_COMMIT)
    {
        // We’re good to write
        if (newRawFx) newRawFx->AddRef();
        *reinterpret_cast<ID3DXEffect**>(slotPtr) = newRawFx;
        g_SlotRetainedFx[slotIndex] = g_LastReloadedFx;
        printf_s(
            "[ReplaceShaderSlot] ✅ Wrote new ID3DXEffect* (0x%p) into offset +0x%X (slot %d) (no VirtualProtect needed)\n",
            newRawFx, offset, slotIndex);
        return true;
    }

    // If we reach here, the memory is not writable. Log why:
    if (!VirtualQuery(slotPtr, &mbi, sizeof(mbi)))
    {
        printf_s("[ReplaceShaderSlot] ❌ VirtualQuery failed on %p\n", slotPtr);
        return false;
    }
    if (!(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)))
    {
        printf_s("[ReplaceShaderSlot] ❌ Memory at %p is not writable (Protect = 0x%X)\n",
                 slotPtr, mbi.Protect);
    }
    if (mbi.State != MEM_COMMIT)
    {
        printf_s("[ReplaceShaderSlot] ❌ Memory at %p is not committed (State = 0x%X)\n",
                 slotPtr, mbi.State);
    }

    // Now attempt to force write access via VirtualProtect:
    DWORD oldProtect = 0;
    if (!VirtualProtect(slotPtr, sizeof(ID3DXEffect*), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf_s("[ReplaceShaderSlot] ❌ VirtualProtect failed on %p\n", slotPtr);
        return false;
    }

    // Perform the write while protection is lifted:
    if (newRawFx) newRawFx->AddRef();
    *reinterpret_cast<ID3DXEffect**>(slotPtr) = newRawFx;
    g_SlotRetainedFx[slotIndex] = g_LastReloadedFx;

    // Restore the original protection:
    VirtualProtect(slotPtr, sizeof(ID3DXEffect*), oldProtect, &oldProtect);

    printf_s("[ReplaceShaderSlot] ✅ Patched with VirtualProtect at offset +0x%X (slot %d)\n",
             offset, slotIndex);
    return true;
}

bool TryPatchSlotIfWritable(void* obj, size_t offset, ID3DXEffect* fx)
{
    if (!obj || !fx || !IsValidShaderPointer(fx)) return false;

    if (IsBadWritePtr(reinterpret_cast<BYTE*>(obj) + offset, sizeof(fx)))
    {
        printf_s("[Patch] ❌ Cannot write to offset +0x%X at %p — skipping\n", (unsigned)offset, obj);
        return false;
    }

    bool result = ReplaceShaderSlot_RawEffect(
        reinterpret_cast<BYTE*>(obj),
        offset,
        fx,
        62 // used for slot tracking
    );

    if (result && offset == 0x18C)
        printf_s("[Patch] ✅ ReplaceShaderSlot_RawEffect succeeded on slot 62\n");

    return result;
}

static void OnFramePresent()
{
    static bool forceApplied = false;

    if (!g_LastReloadedFx || !IsValidShaderPointer(g_LastReloadedFx))
        return;

    ReloadBlurBindings(g_LastReloadedFx);

    if (!forceApplied &&
        g_pVisualTreatment && *g_pVisualTreatment && IsValidThis(*g_pVisualTreatment))
    {
        printf_s("[Debug] Trying to patch +0x18C on %p...\n", *g_pVisualTreatment);

        if (TryPatchSlotIfWritable(*g_pVisualTreatment, 0x18C, g_LastReloadedFx))
        {
            printf_s("[Fallback] 🔧 Forced shader slot patch to g_LastReloadedFx at +0x18C\n");

            VisualTreatment_Reset();
            *(BYTE*)0x00982C39 = 1;
            printf_s("[Fallback] ✅ Set LoadedFlagMaybe = 1 (0x00982C39)\n");

            forceApplied = true;
        }
        else
        {
            printf_s("[Fallback] ❌ TryPatchSlotIfWritable failed\n");
        }
    }
}

// Hooked EndScene
HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 device)
{
    if (!device)
        return oEndScene(device);

    if (!g_Device)
        SetGameDevice(device);

    OnFramePresent();

    // Check if blur system is initialized
    if (!g_CurrentBlurSurface || !g_MotionBlurTexA || !g_MotionBlurTexB)
        return oEndScene(device);

    // Check if shader exists and is valid
    auto it = g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
    if (it == g_ActiveEffects.end() || !it->second || !IsValidShaderPointer(it->second))
        return oEndScene(device);

    // Step 1: Copy current backbuffer into g_CurrentBlurSurface
    IDirect3DSurface9* backBuffer = nullptr;
    if (SUCCEEDED(device->GetRenderTarget(0, &backBuffer)))
    {
        device->StretchRect(backBuffer, nullptr, g_CurrentBlurSurface, nullptr, D3DTEXF_LINEAR);
        backBuffer->Release();
    }

    // ✅ Step 2: Run the actual blur pass using the full effect system
    RenderBlurPass(device);

    return oEndScene(device);
}

HRESULT WINAPI hkEndScene2(LPDIRECT3DDEVICE9 device)
{
    if (!device)
        return oEndScene(device);

    if (!g_Device)
        SetGameDevice(device);

    OnFramePresent();

    // Make sure our targets and shader are valid
    if (!g_CurrentBlurSurface || !g_MotionBlurTexA)
        return oEndScene(device);

    auto it = g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
    if (it == g_ActiveEffects.end() || !it->second || !IsValidShaderPointer(it->second))
        return oEndScene(device);

    ID3DXEffect* fx = it->second;

    // Step 1: Copy current backbuffer into g_CurrentBlurSurface
    IDirect3DSurface9* backBuffer = nullptr;
    if (SUCCEEDED(device->GetRenderTarget(0, &backBuffer)))
    {
        device->StretchRect(backBuffer, nullptr, g_CurrentBlurSurface, nullptr, D3DTEXF_LINEAR);

        // Step 2: Set render target back to original (if it changed)
        device->SetRenderTarget(0, backBuffer);

        // Step 3: Setup and render blur pass
        fx->SetTechnique("visualtreatment_branching"); // or "vt", depending on use

        // Bind motion blur texture as DIFFUSEMAP_TEXTURE
        fx->SetTexture("DIFFUSEMAP_TEXTURE", g_CurrentBlurTex); // or TexB if ping-ponging

        // Dummy fallbacks if needed
        static IDirect3DTexture9* dummy1x1 = GetDummyWhiteTexture(device); // helper function
        fx->SetTexture("MISCMAP1_TEXTURE", g_GainMapTex ? g_GainMapTex : dummy1x1);
        fx->SetTexture("MISCMAP2_TEXTURE", g_VignetteTex ? g_VignetteTex : dummy1x1);
        fx->SetTexture("MISCMAP3_TEXTURE", g_BloomTex ? g_BloomTex : dummy1x1);
        fx->SetTexture("MISCMAP4_TEXTURE", g_DofTex ? g_DofTex : dummy1x1);
        fx->SetTexture("HEIGHTMAP_TEXTURE", g_LinearDepthTex ? g_LinearDepthTex : dummy1x1);

        UINT passes = 0;
        if (SUCCEEDED(fx->Begin(&passes, 0)))
        {
            for (UINT i = 0; i < passes; ++i)
            {
                if (SUCCEEDED(fx->BeginPass(i)))
                {
                    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
                    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, screenQuadVerts, sizeof(Vertex));
                    printf_s("[XNFS] ✅ DrawPrimitiveUP executed\n");
                    fx->EndPass();
                }
                else
                {
                    printf_s("[XNFS] ⚠️ BeginPass(%u) failed\n", i);
                }
            }
            fx->End();
        }
        else
        {
            printf_s("[XNFS] ❌ fx->Begin failed\n");
        }

        backBuffer->Release();
    }

    return oEndScene(device);
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND hwnd,
                             const RGNDATA* dirty)
{
    SetGameDevice(device);

    void** vtable = *reinterpret_cast<void***>(device);

    if (!oEndScene)
    {
        DWORD oldProtect;
        oEndScene = reinterpret_cast<EndScene_t>(vtable[42]);
        VirtualProtect(&vtable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[42] = reinterpret_cast<void*>(&hkEndScene);
        VirtualProtect(&vtable[42], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[XNFS] ✅ hkEndScene installed via HookedPresent\n");
    }

    return RealPresent(device, src, dest, hwnd, dirty);
}

void LogApplyGraphicsSettingsCall(void* manager, void* object, int objectType)
{
    std::pair<void*, int> key = std::make_pair(object, objectType);
    static std::unordered_set<void*> logged;

    if (logged.insert(manager).second)
    {
        void* vfn0 = nullptr;
        DWORD field4 = 0;

        if (!IsBadReadPtr(manager, sizeof(void*)))
        {
            vfn0 = ((void**)manager)[0];
        }
        if (!IsBadReadPtr((BYTE*)manager + 4, sizeof(DWORD)))
        {
            field4 = *(DWORD*)((BYTE*)manager + 4);
        }

        printf_s("[HookApplyGraphicsSettings] 🧩 manager = %p | vtable[0] = %p | field4 = 0x%08X\n", manager, vfn0,
                 field4);
    }
}

static void SafeApplyGraphicsSettingsMain(void* manager)
{
    __try
    {
        ApplyGraphicsManagerMainOriginal(manager);
        printf_s("[HotReload] ✅ Applied GraphicsSettings\n");
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf_s("[HotReload] ❌ Exception during ApplyGraphicsManagerMainOriginal\n");
    }
}

void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject)
{
    if (IsValidThis(manager))
    {
        if (!g_ApplyGraphicsManagerThis)
        {
            g_ApplyGraphicsManagerThis = manager;
            printf_s("[HookApplyGraphicsSettings] ✅ Captured g_ApplyGraphicsManagerThis = %p\n", manager);
        }

        LogApplyGraphicsSettingsCall(manager, vtObject, 1);
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
    }
}

void __fastcall HookApplyGraphicsSettings1(void* manager, void*, void* vtObject)
{
    LogApplyGraphicsSettingsCall(manager, vtObject, 1);

    if (!manager || IsBadReadPtr(manager, sizeof(void*)))
    {
        printf_s("[HookApplyGraphicsSettings] ❌ manager is NULL or unreadable — skipping\n");
        return;
    }

    void** vtbl = *(void***)manager;

    DWORD field4 = 0;
    bool canReadField = !IsBadReadPtr((BYTE*)manager + 4, sizeof(DWORD));
    if (canReadField)
        field4 = *(DWORD*)((BYTE*)manager + 4);

    static std::unordered_set<void*> seenInvalidManagers;

    if (IsBadReadPtr(vtbl, sizeof(void*)) || !canReadField || IsBadReadPtr((void*)field4, 4))
    {
        // Only log once per invalid manager pointer
        if (seenInvalidManagers.insert(manager).second)
        {
            printf_s("[HookApplyGraphicsSettings] ⚠️ Skipping: manager = %p, field4 = 0x%08X (invalid)\n", manager,
                     field4);
        }
        return;
    }

    static std::unordered_set<void*> triedManagers;
    if (triedManagers.insert(manager).second)
    {
        printf_s("[HookApplyGraphicsSettings] 🔍 First-time seen manager = %p, field4 = 0x%08X\n", manager, field4);
    }

    if (!g_ApplyGraphicsManagerThis &&
        IsValidThis(manager) &&
        !IsBadReadPtr((void*)field4, 4) && // field4 must be readable if used internally
        field4 > 0x10000) // skip tiny constants
    {
        g_ApplyGraphicsManagerThis = manager;
        printf_s("[HookApplyGraphicsSettings] ✅ Captured g_ApplyGraphicsManagerThis = %p\n", manager);
    }

    if (g_LastReloadedFx && IsValidShaderPointer(g_LastReloadedFx) &&
        g_pVisualTreatment && *g_pVisualTreatment && IsValidThis(*g_pVisualTreatment) &&
        g_ApplyGraphicsManagerThis && IsValidThis(g_ApplyGraphicsManagerThis))
    {
        if (TryPatchSlotIfWritable(*g_pVisualTreatment, 0x18C, g_LastReloadedFx))
        {
            printf_s("[HotReload] 🔧 Patched fx slot at +0x18C\n");
        }

        VisualTreatment_Reset();
        *(BYTE*)0x00982C39 = 1;
        printf_s("[HotReload] ✅ Set LoadedFlagMaybe = 1 (0x00982C39)\n");

        SafeApplyGraphicsSettingsMain(g_ApplyGraphicsManagerThis);

        if (g_LastEView)
        {
            ForceFrameRender();
            printf_s("[HotReload] ✅ ForceFrameRender (sub_6DE300)\n");
        }
    }
    else
    {
        ApplyGraphicsSettingsOriginal(manager, nullptr, manager);
    }
}

DWORD WINAPI DeferredHookThread(LPVOID)
{
    while (!g_Device)
        Sleep(10);

    if (!g_Device) return E_FAIL;
    void** vtable = *(void***)g_Device;
    if (!vtable)
        return E_FAIL;

    // Hook Present
    RealPresent = (PresentFn)vtable[17];
    DWORD oldProtect;
    VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    vtable[17] = (void*)&HookedPresent;
    VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
    printf_s("[Init] Hooked IDirect3DDevice9::Present (deferred)\n");

    // Hook Reset
    oReset = (Reset_t)vtable[16];
    VirtualProtect(&vtable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    vtable[16] = (void*)&hkReset;
    VirtualProtect(&vtable[16], sizeof(void*), oldProtect, &oldProtect);
    printf_s("[Init] Hooked IDirect3DDevice9::Reset (deferred)\n");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        printf_s("[Init] Shader override DLL loaded.\n");

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (d3dx)
        {
            void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
            RealCreateFromResource = (D3DXCreateEffectFromResourceAFn)addr;
            injector::MakeCALL(0x006C60D2, HookedCreateFromResource, true);

            ApplyGraphicsManagerMainOriginal = (decltype(ApplyGraphicsManagerMainOriginal))0x004F17F0;
            printf_s("[Init] ApplyGraphicsManagerMainOriginal set to 0x004F17F0\n");

            ApplyGraphicsSettingsOriginal = reinterpret_cast<ApplyGraphicsSettingsFn>(0x004EA0D0);
            injector::MakeCALL(0x004F186E, HookApplyGraphicsSettings, true);

            CreateThread(nullptr, 0, DeferredHookThread, nullptr, 0, nullptr);

            printf_s("[Init] Hooked D3DXCreateEffectFromResourceA\n");
        }
    }
    return TRUE;
}
