// dllmain.cpp - Backbuffer hook for NFS MW (2005)
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <cctype>
#include "includes/injector/injector.hpp"
#include <d3dx9effect.h>
#include <cstdio>
#include <psapi.h>  // Required for MODULEINFO and GetModuleInformation
#include <d3d9.h>
#include <unordered_map>
#include <unordered_set>
#include "Hooks.h"
#include "RenderTargetManager.h"
#include "Globals.h"
#include "EffectManager.h"
#include "MotionBlurPass.h"
#include "Validators.h"
#include "ScopedStateBlock.h"
#include "Modules/minhook/include/MinHook.h"
#pragma comment(lib, "d3d9.lib")

// -------------------- GLOBALS --------------------
#define CHECKMARK(x) ((x) ? "OK" : "MISSING")

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

static void PatchMotionBlurFix()
{
    struct PatchByte
    {
        uintptr_t addr;
        BYTE value;
    };
    const PatchByte patches[] =
    {
        {0x006DBE79, 0x01},
        {0x006DBE7B, 0x02},
        {0x006DF1D2, 0xEB}, // skip vanilla blur call (NGG behavior)
    };

    for (const auto& p : patches)
    {
        BYTE* ptr = reinterpret_cast<BYTE*>(p.addr);
        DWORD oldProtect = 0;
        if (VirtualProtect(ptr, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            *ptr = p.value;
            VirtualProtect(ptr, 1, oldProtect, &oldProtect);
        }
    }
    printf_s("[XNFS] ? NextGenMotionBlur patch applied (disable vanilla blur)\n");
}

// IVisualTreatment_Reset block
void* g_LastEView = nullptr;

void (__fastcall*ApplyGraphicsSettingsOriginal)(void* ecx, void* edx, void* arg1) = nullptr;
int (__thiscall*ApplyGraphicsManagerMainOriginal)(void* thisptr) = nullptr;
bool g_WaitingForReset = false;

typedef void (__thiscall*UpdateFunc)(void* thisptr, void* eView);
typedef void (__cdecl*FrameRenderFn)();
FrameRenderFn ForceFrameRender = (FrameRenderFn)FrameRenderFn_ADDR;
void** g_pVisualTreatment = (void**)pVisualTreatmentPlat_ADDRESS;

using IVisualTreatment_ResetFn = void(__thiscall*)(void* thisPtr);
IVisualTreatment_ResetFn IVisualTreatment_Reset = (IVisualTreatment_ResetFn)Reset_16IVisualTreatment_ADDRESS;

using RenderDispatchFn = void(__cdecl*)(void* arg0, void* arg4);
static RenderDispatchFn g_RenderDispatchOriginal = (RenderDispatchFn)0x00744DD0;
using IVisualTreatmentGetFn = void* (__cdecl*)();
static IVisualTreatmentGetFn RealIVisualTreatmentGet = nullptr;
using D3DXCreateEffectFromFileAFn = HRESULT (WINAPI*)(
    LPDIRECT3DDEVICE9,
    LPCSTR,
    const D3DXMACRO*,
    LPD3DXINCLUDE,
    DWORD,
    LPD3DXEFFECTPOOL,
    LPD3DXEFFECT*,
    LPD3DXBUFFER*);
static D3DXCreateEffectFromFileAFn RealCreateFromFileA = nullptr;
using D3DXCreateEffectFromFileExAFn = HRESULT (WINAPI*)(
    LPDIRECT3DDEVICE9,
    LPCSTR,
    const D3DXMACRO*,
    LPD3DXINCLUDE,
    LPCSTR,
    DWORD,
    LPD3DXEFFECTPOOL,
    LPD3DXEFFECT*,
    LPD3DXBUFFER*);
static D3DXCreateEffectFromFileExAFn RealCreateFromFileExA = nullptr;
using D3DXCreateEffectFn = HRESULT (WINAPI*)(
    LPDIRECT3DDEVICE9,
    LPCVOID,
    UINT,
    const D3DXMACRO*,
    LPD3DXINCLUDE,
    DWORD,
    LPD3DXEFFECTPOOL,
    LPD3DXEFFECT*,
    LPD3DXBUFFER*);
static D3DXCreateEffectFn RealCreateEffect = nullptr;
static bool g_D3DXHooksInstalled = false;

static void __cdecl RenderDispatchHook(void* arg0, void* arg4)
{
    // const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    // if (gameFlow < 3 || g_RenderTargetManager.g_DeviceResetInProgress)
    // {
    //     if (g_RenderDispatchOriginal)
    //         g_RenderDispatchOriginal(arg0, arg4);
    //     return;
    // }

    static uint32_t lastLogFrame = 0;
    if (eFrameCounter != lastLogFrame)
    {
        lastLogFrame = eFrameCounter;
        printf_s("[Blur] dispatch index=0\n");
    }
    MotionBlurPass::CustomMotionBlurHook();
    if (g_RenderDispatchOriginal)
        g_RenderDispatchOriginal(arg0, arg4);
}

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

void ScrubShaderCleanupTable()
{
    return;
    int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    if (gameFlow < 3 || !g_Device || g_RenderTargetManager.g_DeviceResetInProgress)
        return;

    for (int i = 0; i < 11; i++)
    {
        uintptr_t entryBase = SHADER_CLEANUP_TABLE_ADDRESS + i * 0x1C;
        void** fxPtr = (void**)(entryBase + 0x0C); // offset to ID3DXEffect*

        if (*fxPtr && IsBadReadPtr(*fxPtr, 4))
        {
            printf_s("[Scrub] ❌ fx at index %d is unreadable (%p) — nulling out\n", i, *fxPtr);
            *fxPtr = nullptr;
        }
        else if (*fxPtr)
        {
            __try
            {
                void** vtbl = *(void***)(*fxPtr);
                if (IsBadReadPtr(vtbl, sizeof(void*)))
                {
                    printf_s("[Scrub] ⚠️ fx[%d] vtable unreadable — nulling\n", i);
                    *fxPtr = nullptr;
                }
                else if (!IsBadCodePtr((FARPROC)vtbl[2]))
                {
                    printf_s("[Scrub] ✅ fx[%d] valid: %p (vtable: %p)\n", i, *fxPtr, vtbl);
                }
                else
                {
                    printf_s("[Scrub] ⚠️ fx[%d] has invalid vtable func ptr: %p — nulling\n", i, vtbl[2]);
                    *fxPtr = nullptr;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                printf_s("[Scrub] ⚠️ fx[%d] threw during vtable read — nulling\n", i);
                *fxPtr = nullptr;
            }
        }
    }
}

void VisualTreatment_Reset()
{
    if (g_pVisualTreatment && IsValidThis(*g_pVisualTreatment))
    {
        void* vt = *g_pVisualTreatment;

        if (!vt) // <-- 🛑 ADD THIS CHECK
        {
            printf_s("[HotReload] ❌ vt was null in VisualTreatment_Reset() (early)\n");
            return;
        }

        // ⚠️ force invalidate ptr+0x140 to trigger rebuild
        void** fx140 = (void**)((char*)vt + 0x140);
        *fx140 = nullptr;

        ID3DXEffect** fxSlot = (ID3DXEffect**)((char*)vt + 0x18C);
        if (IsValidShaderPointer(*fxSlot))
        {
            if (*fxSlot)
            {
                if (*fxSlot == g_RenderTargetManager.g_LastReloadedFx)
                {
                    g_RenderTargetManager.g_LastReloadedFx = nullptr;
                }

                for (int i = 0; i < 62; i++)
                {
                    if (g_SlotRetainedFx[i] == *fxSlot)
                        g_SlotRetainedFx[i] = nullptr;
                }

                printf_s("🔧 Releasing fx at +0x18C (%p)\n", *fxSlot);
                (*fxSlot)->Release();
                *fxSlot = nullptr;
            }
        }
        else
            return;

        // 🧪 Optional: Add null check if unsure about vt
        if (vt)
        {
            IVisualTreatment_Reset(vt);
            printf_s("[HotReload] ✅ Reset() called on IVisualTreatment at %p\n", vt);
        }
        else
        {
            printf_s("[HotReload] ❌ vt was null in VisualTreatment_Reset()\n");
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
        g_SlotRetainedFx[slotIndex] = g_RenderTargetManager.g_LastReloadedFx;
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
    g_SlotRetainedFx[slotIndex] = g_RenderTargetManager.g_LastReloadedFx;

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

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name = "")
{
    static uint32_t lastEnterLogFrame = 0;
    if (eFrameCounter != lastEnterLogFrame)
    {
        lastEnterLogFrame = eFrameCounter;
        printf_s("[BlurRebind] enter fx=%p blurTex=%p reset=%u\n",
                 fx,
                 g_RenderTargetManager.g_CurrentBlurTex,
                 g_RenderTargetManager.g_DeviceResetInProgress ? 1u : 0u);
    }
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

    if (g_RenderTargetManager.g_DeviceResetInProgress)
    {
        printf_s("[BlurRebind] ⚠️ Skipped during device reset\n");
        return;
    }

    D3DXHANDLE hDiffuse = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    if (!hDiffuse)
        hDiffuse = fx->GetParameterByName(nullptr, "DiffuseMap");
    if (!hDiffuse)
        hDiffuse = fx->GetParameterBySemantic(nullptr, "DiffuseMap");

    D3DXHANDLE hBlur = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
    if (!hBlur)
        hBlur = fx->GetParameterByName(nullptr, "MOTIONBLUR");
    if (!hBlur)
        hBlur = fx->GetParameterBySemantic(nullptr, "MOTIONBLUR");

    if (!hDiffuse || !hBlur)
    {
        printf_s("[BlurRebind] ? Missing handles: DIFFUSE=%p BLUR=%p\n", hDiffuse, hBlur);
        static uint32_t lastDumpFrame = 0;
        if (eFrameCounter != lastDumpFrame)
        {
            lastDumpFrame = eFrameCounter;
            D3DXEFFECT_DESC ed = {};
            if (SUCCEEDED(fx->GetDesc(&ed)))
            {
                printf_s("[BlurRebind] ? Effect params: %u\n", ed.Parameters);
                for (UINT i = 0; i < ed.Parameters; ++i)
                {
                    D3DXHANDLE p = fx->GetParameter(nullptr, i);
                    if (!p)
                        continue;
                    D3DXPARAMETER_DESC pd = {};
                    if (FAILED(fx->GetParameterDesc(p, &pd)))
                        continue;
                    if (pd.Class == D3DXPC_OBJECT && pd.Type == D3DXPT_TEXTURE)
                    {
                        printf_s("[BlurRebind] ? tex[%u] name=%s semantic=%s\n",
                                 i,
                                 pd.Name ? pd.Name : "(null)",
                                 pd.Semantic ? pd.Semantic : "(null)");
                    }
                }
            }
        }
    }

    if (hDiffuse)
    {
        if (g_RenderTargetManager.g_LastSceneFullTex)
        {
            fx->SetTexture(hDiffuse, g_RenderTargetManager.g_LastSceneFullTex);
        }
        else if (g_RenderTargetManager.g_MotionBlurTexA)
        {
            fx->SetTexture(hDiffuse, g_RenderTargetManager.g_MotionBlurTexA);
        }
        else
        {
            fx->SetTexture(hDiffuse, g_RenderTargetManager.g_CurrentBlurTex);
        }
    }

    if (hBlur)
        fx->SetTexture(hBlur, g_RenderTargetManager.g_CurrentBlurTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP1_TEXTURE"), g_RenderTargetManager.g_ExposureTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP2_TEXTURE"), g_RenderTargetManager.g_VignetteTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP3_TEXTURE"), g_RenderTargetManager.g_BloomLUTTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "MISCMAP4_TEXTURE"), g_RenderTargetManager.g_DofTex);
    fx->SetTexture(fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE"), g_RenderTargetManager.g_DepthTex);

    // BlurParams / XNFS_MotionBlurAmount
    const D3DXVECTOR4 blurVec(1.0f, 0.2f, 1.0f, 0.0f);
    D3DXHANDLE h = fx->GetParameterByName(nullptr, "BlurParams");
    if (h)
        fx->SetVector(h, &blurVec);
    D3DXHANDLE hMb = fx->GetParameterByName(nullptr, "XNFS_MotionBlurAmount");
    if (hMb)
        fx->SetFloat(hMb, g_MotionBlurAmount);

    static uint32_t lastBindLogFrame = 0;
    if (eFrameCounter != lastBindLogFrame)
    {
        lastBindLogFrame = eFrameCounter;
        IDirect3DBaseTexture9* diff = nullptr;
        IDirect3DBaseTexture9* blur = nullptr;
        D3DXHANDLE hDiff = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
        D3DXHANDLE hBlur = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
        if (hDiff)
            fx->GetTexture(hDiff, &diff);
        if (hBlur)
            fx->GetTexture(hBlur, &blur);
        printf_s("[BlurRebind] diff=%p blur=%p amount=%.3f\n", diff, blur, g_MotionBlurAmount);
        SAFE_RELEASE(diff);
        SAFE_RELEASE(blur);
    }

    if (name == "IDI_VISUALTREATMENT_FX")
    {
        if (g_RenderTargetManager.g_Width && g_RenderTargetManager.g_Height)
        {
            const float texelX = 1.0f / static_cast<float>(g_RenderTargetManager.g_Width);
            const float texelY = 1.0f / static_cast<float>(g_RenderTargetManager.g_Height);
            D3DXHANDLE hOff0 = fx->GetParameterByName(nullptr, "DownSampleOffset0");
            D3DXHANDLE hOff1 = fx->GetParameterByName(nullptr, "DownSampleOffset1");
            if (hOff0 && hOff1)
            {
                const D3DXVECTOR4 off0(-1.5f * texelX, -1.5f * texelY,
                                       0.5f * texelX, -1.5f * texelY);
                const D3DXVECTOR4 off1(-1.5f * texelX, 0.5f * texelY,
                                       0.5f * texelX, 0.5f * texelY);
                fx->SetVector(hOff0, &off0);
                fx->SetVector(hOff1, &off1);
            }
        }

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
    if (!device || !pResource)
    {
        printf_s("[XNFS] ⚠️ Skipping invalid shader load — device or pResource is null\n");
        if (outEffect) *outEffect = nullptr;
        return E_FAIL;
    }

    HRESULT hr = E_FAIL;
    // Only process specific shaders
    bool isVisualFx = strcmp(pResource, "IDI_VISUALTREATMENT_FX") == 0;
    bool isOverbrightFx = strcmp(pResource, "IDI_OVERBRIGHT_FX") == 0;
    if (isVisualFx || isOverbrightFx)
        SetGameDevice(device);

    // If a file override exists, compile it instead of the embedded resource.
    if (isVisualFx || isOverbrightFx)
    {
        const char* overrideFile = isVisualFx ? "fx\\visualtreatment.fx" : "fx\\overbright.fx";
        DWORD attr = GetFileAttributesA(overrideFile);
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
        {
            D3DXCreateEffectFromFileAFn createFromFile = RealCreateFromFileA;
            if (!createFromFile)
            {
                const char* kD3dxDlls[] = {"d3dx9_43.dll", "d3dx9_42.dll", "d3dx9_41.dll", "d3dx9_40.dll"};
                HMODULE d3dx = nullptr;
                for (const char* dll : kD3dxDlls)
                {
                    d3dx = GetModuleHandleA(dll);
                    if (d3dx)
                        break;
                }
                if (d3dx)
                    createFromFile = reinterpret_cast<D3DXCreateEffectFromFileAFn>(
                        GetProcAddress(d3dx, "D3DXCreateEffectFromFileA"));
            }

            if (createFromFile)
            {
                printf_s("[XNFS] ? Using FX override: %s\n", overrideFile);
                HRESULT hrFile = createFromFile(
                    device, overrideFile, defines, include, flags, pool, outEffect, outErrors);
                if (SUCCEEDED(hrFile) && outEffect && *outEffect)
                {
                    // Treat the file override as the resource load result.
                    hr = hrFile;
                    goto track_effect;
                }
                if (outErrors && *outErrors)
                {
                    printf_s("[XNFS] ? FX override compile failed: %s\n",
                             (const char*)(*outErrors)->GetBufferPointer());
                }
                // Fall back to the embedded resource on failure.
            }
        }
    }

    if (!g_RenderTargetManager.RealCreateFromResource)
    {
        printf_s("? RealCreateFromResource was null!\\n");
        return E_FAIL;
    }

    hr = g_RenderTargetManager.RealCreateFromResource(
        device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);

    if (FAILED(hr) || !outEffect || !*outEffect)
    {
        if (isVisualFx || isOverbrightFx)
            printf_s("[XNFS] ❌ Shader creation failed or returned null (hr=0x%08X)\n", hr);
        return hr;
    }

track_effect:
    if (isVisualFx || isOverbrightFx)
    {
        auto& existingFx = g_RenderTargetManager.g_ActiveEffects[pResource];

        if (existingFx && existingFx != *outEffect)
        {
            printf_s("[XNFS] 🔁 Replacing previous effect (%p) for %s\n", existingFx, pResource);
            existingFx->Release();
        }

        existingFx = *outEffect;
        (*outEffect)->AddRef(); // Retain since we're storing it
        if (g_RenderTargetManager.g_LastReloadedFx)
            g_RenderTargetManager.g_LastReloadedFx->Release();
        g_RenderTargetManager.g_LastReloadedFx = *outEffect;
        (*outEffect)->AddRef();
        printf_s("✅ Shader created and tracked: %s (%p)\n", pResource, *outEffect);
        SAFE_RELEASE(g_RenderTargetManager.g_BlurEffect);
        if (FAILED((*outEffect)->CloneEffect(device, &g_RenderTargetManager.g_BlurEffect)))
        {
            printf_s("[XNFS] ?? CloneEffect failed for %s\\n", pResource);
        }

        if (g_RenderTargetManager.g_CurrentBlurTex)
        {
            printf_s("[XNFS] 🔁 ReloadBlurBindings (early)\n");
            ReloadBlurBindings(*outEffect, pResource);
        }
        else
        {
            printf_s("[XNFS] ⏩ Skipping ReloadBlurBindings — g_CurrentBlurTex not ready\n");
        }
    }

    return hr;
}

static void* __cdecl HookedIVisualTreatmentGet()
{
    static bool inHook = false;
    if (inHook)
        return RealIVisualTreatmentGet ? RealIVisualTreatmentGet() : nullptr;

    inHook = true;
    const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    void* vt = RealIVisualTreatmentGet ? RealIVisualTreatmentGet() : nullptr;

    if (gameFlow >= 3 && !g_RenderTargetManager.g_DeviceResetInProgress && vt)
    {
        if (!IsBadReadPtr(vt, sizeof(void*)))
        {
            ID3DXEffect* fx = *reinterpret_cast<ID3DXEffect**>((char*)vt + 0x18C);
            if (IsValidShaderPointer(fx))
            {
                auto& existingFx = g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"];
                if (existingFx && existingFx != fx)
                    existingFx->Release();
                existingFx = fx;
                existingFx->AddRef();

                if (g_RenderTargetManager.g_LastReloadedFx)
                    g_RenderTargetManager.g_LastReloadedFx->Release();
                g_RenderTargetManager.g_LastReloadedFx = fx;
                g_RenderTargetManager.g_LastReloadedFx->AddRef();

                if (g_RenderTargetManager.g_CurrentBlurTex)
                    ReloadBlurBindings(fx, "IDI_VISUALTREATMENT_FX");
            }
        }
    }

    inHook = false;
    return vt;
}

static bool IsTargetEffectFile(const char* path, std::string& outKey)
{
    if (!path)
        return false;

    const char* base = strrchr(path, '\\');
    if (!base)
        base = strrchr(path, '/');
    base = base ? (base + 1) : path;

    std::string lower(base);
    for (char& c : lower)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

    if (lower == "visualtreatment.fx")
    {
        outKey = "IDI_VISUALTREATMENT_FX";
        return true;
    }
    if (lower == "overbright.fx")
    {
        outKey = "IDI_OVERBRIGHT_FX";
        return true;
    }
    return false;
}

static bool IsVisualTreatmentEffect(ID3DXEffect* fx)
{
    if (!fx)
        return false;

    D3DXHANDLE h = fx->GetParameterByName(nullptr, "VisualEffectRadialBlur");
    if (h)
        return true;
    h = fx->GetParameterByName(nullptr, "VisualEffectBrightness");
    if (h)
        return true;
    h = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
    if (h)
        return true;
    h = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    return h != nullptr;
}

HRESULT WINAPI HookedCreateEffect(
    LPDIRECT3DDEVICE9 device,
    LPCVOID pSrcData,
    UINT srcDataLen,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors)
{
    if (!RealCreateEffect)
        return E_FAIL;

    HRESULT hr = RealCreateEffect(
        device, pSrcData, srcDataLen, defines, include, flags, pool, outEffect, outErrors);

    if (FAILED(hr) || !outEffect || !*outEffect)
        return hr;

    if (IsVisualTreatmentEffect(*outEffect))
    {
        auto& existingFx = g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"];
        if (existingFx != *outEffect)
        {
            // VT-owned effects can be recreated often; keep raw pointers without refcount changes.
            existingFx = *outEffect;
            g_RenderTargetManager.g_LastReloadedFx = *outEffect;
            printf_s("[XNFS] ✅ Shader created and tracked (effect): IDI_VISUALTREATMENT_FX (%p)\n", *outEffect);
        }

        const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
        if (gameFlow >= 3 && !g_RenderTargetManager.g_DeviceResetInProgress && g_RenderTargetManager.g_CurrentBlurTex)
            ReloadBlurBindings(*outEffect, "IDI_VISUALTREATMENT_FX");
    }

    return hr;
}

HRESULT WINAPI HookedCreateFromFileA(
    LPDIRECT3DDEVICE9 device,
    LPCSTR pFilename,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors)
{
    if (!RealCreateFromFileA)
        return E_FAIL;

    std::string key;
    const bool isTarget = IsTargetEffectFile(pFilename, key);
    if (isTarget)
        printf_s("[XNFS] ? D3DXCreateEffectFromFileA: %s\n", pFilename ? pFilename : "(null)");

    HRESULT hr = RealCreateFromFileA(
        device, pFilename, defines, include, flags, pool, outEffect, outErrors);

    if (!isTarget || FAILED(hr) || !outEffect || !*outEffect)
        return hr;

    auto& existingFx = g_RenderTargetManager.g_ActiveEffects[key];
    if (existingFx && existingFx != *outEffect)
    {
        printf_s("[XNFS] ?? Replacing previous effect (%p) for %s\n", existingFx, key.c_str());
        existingFx->Release();
    }

    existingFx = *outEffect;
    existingFx->AddRef();

    if (g_RenderTargetManager.g_LastReloadedFx)
        g_RenderTargetManager.g_LastReloadedFx->Release();
    g_RenderTargetManager.g_LastReloadedFx = *outEffect;
    g_RenderTargetManager.g_LastReloadedFx->AddRef();

    printf_s("[XNFS] ? Shader created and tracked (file): %s (%p)\n", key.c_str(), *outEffect);

    {
        const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
        if (gameFlow >= 3 && !g_RenderTargetManager.g_DeviceResetInProgress && g_RenderTargetManager.g_CurrentBlurTex)
            ReloadBlurBindings(*outEffect, key);
    }

    return hr;
}

HRESULT WINAPI HookedCreateFromFileExA(
    LPDIRECT3DDEVICE9 device,
    LPCSTR pFilename,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    LPCSTR skipConstants,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors)
{
    if (!RealCreateFromFileExA)
        return E_FAIL;

    std::string key;
    const bool isTarget = IsTargetEffectFile(pFilename, key);
    if (isTarget)
        printf_s("[XNFS] ? D3DXCreateEffectFromFileExA: %s\n", pFilename ? pFilename : "(null)");

    HRESULT hr = RealCreateFromFileExA(
        device, pFilename, defines, include, skipConstants, flags, pool, outEffect, outErrors);

    if (!isTarget || FAILED(hr) || !outEffect || !*outEffect)
        return hr;

    auto& existingFx = g_RenderTargetManager.g_ActiveEffects[key];
    if (existingFx && existingFx != *outEffect)
    {
        printf_s("[XNFS] ?? Replacing previous effect (%p) for %s\n", existingFx, key.c_str());
        existingFx->Release();
    }

    existingFx = *outEffect;
    existingFx->AddRef();

    if (g_RenderTargetManager.g_LastReloadedFx)
        g_RenderTargetManager.g_LastReloadedFx->Release();
    g_RenderTargetManager.g_LastReloadedFx = *outEffect;
    g_RenderTargetManager.g_LastReloadedFx->AddRef();

    printf_s("[XNFS] ? Shader created and tracked (file ex): %s (%p)\n", key.c_str(), *outEffect);

    {
        const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
        if (gameFlow >= 3 && !g_RenderTargetManager.g_DeviceResetInProgress && g_RenderTargetManager.g_CurrentBlurTex)
            ReloadBlurBindings(*outEffect, key);
    }

    return hr;
}

static void TryInstallD3DXHooks()
{
    if (g_D3DXHooksInstalled)
        return;

    const char* kD3dxDlls[] =
    {
        "d3dx9_43.dll",
        "d3dx9_42.dll",
        "d3dx9_41.dll",
        "d3dx9_40.dll",
    };

    HMODULE d3dx = nullptr;
    for (const char* dll : kD3dxDlls)
    {
        d3dx = GetModuleHandleA(dll);
        if (d3dx)
        {
            printf_s("[Init] ? Found %s\n", dll);
            break;
        }
    }
    if (!d3dx)
        return;

    void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
    if (addr && !g_RenderTargetManager.RealCreateFromResource)
        g_RenderTargetManager.RealCreateFromResource =
            (RenderTargetManager::D3DXCreateEffectFromResourceAFn)addr;

    void* fileAddr = GetProcAddress(d3dx, "D3DXCreateEffectFromFileA");
    void* fileExAddr = GetProcAddress(d3dx, "D3DXCreateEffectFromFileExA");
    void* effectAddr = GetProcAddress(d3dx, "D3DXCreateEffect");

    if (!fileAddr && !fileExAddr && !effectAddr)
        return;

    if (MH_Initialize() != MH_OK)
    {
        printf_s("[Init] ? MH_Initialize failed\n");
        return;
    }

    bool hooked = false;
    if (fileAddr)
    {
        if (!RealCreateFromFileA)
            RealCreateFromFileA = reinterpret_cast<D3DXCreateEffectFromFileAFn>(fileAddr);
        if (MH_CreateHook(fileAddr, HookedCreateFromFileA,
                          reinterpret_cast<void**>(&RealCreateFromFileA)) == MH_OK &&
            MH_EnableHook(fileAddr) == MH_OK)
        {
            printf_s("[Init] Hooked D3DXCreateEffectFromFileA (late)\n");
            hooked = true;
        }
    }

    if (fileExAddr)
    {
        if (!RealCreateFromFileExA)
            RealCreateFromFileExA = reinterpret_cast<D3DXCreateEffectFromFileExAFn>(fileExAddr);
        if (MH_CreateHook(fileExAddr, HookedCreateFromFileExA,
                          reinterpret_cast<void**>(&RealCreateFromFileExA)) == MH_OK &&
            MH_EnableHook(fileExAddr) == MH_OK)
        {
            printf_s("[Init] Hooked D3DXCreateEffectFromFileExA (late)\n");
            hooked = true;
        }
    }

    if (effectAddr)
    {
        if (!RealCreateEffect)
            RealCreateEffect = reinterpret_cast<D3DXCreateEffectFn>(effectAddr);
        if (MH_CreateHook(effectAddr, HookedCreateEffect,
                          reinterpret_cast<void**>(&RealCreateEffect)) == MH_OK &&
            MH_EnableHook(effectAddr) == MH_OK)
        {
            printf_s("[Init] Hooked D3DXCreateEffect (late)\n");
            hooked = true;
        }
    }

    if (hooked)
        g_D3DXHooksInstalled = true;
}

void OnDeviceReset(LPDIRECT3DDEVICE9 device)
{
    if (!device) return;

    g_RenderTargetManager.g_DeviceResetInProgress = true;

    if (!g_RenderTargetManager.OnDeviceReset(device))
    {
        printf_s("[XNFS] ❌ RenderTargetManager::OnDeviceReset failed\n");
    }
    else
    {
        printf_s("[XNFS] ✅ RenderTargetManager::OnDeviceReset succeeded\n");
    }

    g_RenderTargetManager.g_DeviceResetInProgress = false;

    // Reload effect states only if render targets were rebuilt
    if (g_RenderTargetManager.g_CurrentBlurTex)
    {
        for (auto& [name, fx] : g_RenderTargetManager.g_ActiveEffects)
        {
            if (!IsValidShaderPointer(fx))
                continue;
            fx->OnResetDevice();

            if (name == "IDI_VISUALTREATMENT_FX" || name == "IDI_OVERBRIGHT_FX")
                ReloadBlurBindings(fx, name);
        }
    }
    else
    {
        printf_s("[XNFS] ⚠️ g_CurrentBlurTex null — skipped effect rebinding\n");
    }
}

void OnDeviceLost()
{
    int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    g_RenderTargetManager.OnDeviceLost(); // Delegate to RenderTargetManager's cleanup
    if (gameFlow >= 3)
        ScrubShaderCleanupTable();
    printf_s("[XNFS] ✅ Delegated OnDeviceLost to RenderTargetManager\n");
}


HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params)
{
    if (!device || !params)
        return D3DERR_INVALIDCALL;

    if (g_RenderTargetManager.g_DeviceResetInProgress)
        return g_RenderTargetManager.oReset(device, params);

    g_RenderTargetManager.g_DeviceResetInProgress = true;

    HRESULT coop = device->TestCooperativeLevel();

    if (coop == D3D_OK)
    {
        g_RenderTargetManager.g_DeviceResetInProgress = false;
        return g_RenderTargetManager.oReset(device, params);
    }

    if (coop == D3DERR_DEVICELOST)
    {
        printf_s("[XNFS] Device still lost, TestCooperativeLevel = 0x%08X\n", coop);
        g_RenderTargetManager.g_DeviceResetInProgress = false;
        return coop;
    }

    if (coop != D3DERR_DEVICENOTRESET)
    {
        printf_s("[XNFS] Device not ready for Reset, TestCooperativeLevel = 0x%08X\n", coop);
        g_RenderTargetManager.g_DeviceResetInProgress = false;
        return coop;
    }

    printf_s(
        "[XNFS] Reset start: coop=0x%08X bb=%ux%u fmt=%u win=%d msaa=%u\n",
        coop,
        params->BackBufferWidth,
        params->BackBufferHeight,
        params->BackBufferFormat,
        params->Windowed,
        params->MultiSampleType
    );

    if (params->Windowed && params->BackBufferFormat != D3DFMT_UNKNOWN)
    {
        printf_s(
            "[XNFS] Forcing BackBufferFormat to UNKNOWN for windowed reset (was %u)\n",
            params->BackBufferFormat
        );
        params->BackBufferFormat = D3DFMT_UNKNOWN;
    }

    OnDeviceLost(); // release safely

    // device->EvictManagedResources();  // don't call — crashes in MW

    HRESULT hr = g_RenderTargetManager.oReset(device, params);

    if (SUCCEEDED(hr))
        OnDeviceReset(device);
    else
        printf_s("[XNFS] oReset failed: HRESULT = 0x%08X\n", hr);

    g_RenderTargetManager.g_DeviceResetInProgress = false;
    return hr;
}

static void OnFramePresent()
{
    static bool forceApplied = false;

    if (!g_RenderTargetManager.g_LastReloadedFx || !IsValidShaderPointer(g_RenderTargetManager.g_LastReloadedFx))
        return;

    ReloadBlurBindings(g_RenderTargetManager.g_LastReloadedFx);

    if (!forceApplied &&
        g_pVisualTreatment && *g_pVisualTreatment && IsValidThis(*g_pVisualTreatment))
    {
        printf_s("[Debug] Trying to patch +0x18C on %p...\n", *g_pVisualTreatment);

        if (TryPatchSlotIfWritable(*g_pVisualTreatment, 0x18C, g_RenderTargetManager.g_LastReloadedFx))
        {
            printf_s("[Fallback] 🔧 Forced shader slot patch to g_LastReloadedFx at +0x18C\n");

            ScrubShaderCleanupTable();
            VisualTreatment_Reset();
            *(BYTE*)LoadedFlagMaybe_ADDRESS = 1;
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
    static uint32_t lastSceneLogFrame = 0;
    if (!device) return oEndScene(device);
    if (!g_Device) SetGameDevice(device);

    OnFramePresent();

    if (!g_RenderTargetManager.g_CurrentBlurSurface ||
        !g_RenderTargetManager.g_MotionBlurTexA ||
        !g_RenderTargetManager.g_MotionBlurTexB)
        return oEndScene(device);

    // 🔒 Save ALL render state (including viewport) and restore on exit
    ScopedStateBlock _sb(device);

    return oEndScene(device);
}

HRESULT WINAPI hkSetRenderTarget(LPDIRECT3DDEVICE9 device, DWORD index, IDirect3DSurface9* renderTarget)
{
    if (device && index == 0)
    {
        static uint32_t lastRtLogFrame = 0;
        static uint32_t lastRtFrame = 0;
        static IDirect3DSurface9* firstFullSizeRt = nullptr;
        static bool sawSmallRtThisFrame = false;
        if (eFrameCounter != lastRtFrame)
        {
            lastRtFrame = eFrameCounter;
            g_CustomMotionBlurRanThisFrame = false;
            g_SceneTargetThisFrame = nullptr;
            g_SceneTargetBestArea = 0;
            SAFE_RELEASE(firstFullSizeRt);
            sawSmallRtThisFrame = false;
        }
        IDirect3DSurface9* backBuffer = nullptr;
        D3DSURFACE_DESC bbDesc{};
        if (SUCCEEDED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer)) && backBuffer)
        {
            backBuffer->GetDesc(&bbDesc);
            if (bbDesc.Width != g_LastBackBufferW || bbDesc.Height != g_LastBackBufferH)
            {
                g_LastBackBufferW = bbDesc.Width;
                g_LastBackBufferH = bbDesc.Height;
                printf_s("[XNFS] ? BackBuffer: %ux%u fmt=%u ptr=%p\n",
                         bbDesc.Width, bbDesc.Height, bbDesc.Format, backBuffer);
            }
        }
        SAFE_RELEASE(g_RenderTargetManager.g_BackBufferSurface);
        g_RenderTargetManager.g_BackBufferSurface = backBuffer;

        const bool isOurBlurSurface =
            renderTarget &&
            (renderTarget == g_RenderTargetManager.g_CurrentBlurSurface ||
                renderTarget == g_RenderTargetManager.g_MotionBlurSurfaceA ||
                renderTarget == g_RenderTargetManager.g_MotionBlurSurfaceB);

        if (renderTarget)
        {
            D3DSURFACE_DESC sd{};
            if (SUCCEEDED(renderTarget->GetDesc(&sd)))
            {
                if (g_KnownRenderTargets.insert(renderTarget).second)
                {
                    printf_s("[XNFS] ? RT0 set: %ux%u fmt=%u ptr=%p\n",
                             sd.Width, sd.Height, sd.Format, renderTarget);
                }
            }
        }

        if (renderTarget && renderTarget != g_RenderTargetManager.g_BackBufferSurface && !isOurBlurSurface)
        {
            D3DSURFACE_DESC sd{};
            if (SUCCEEDED(renderTarget->GetDesc(&sd)) && bbDesc.Width && bbDesc.Height)
            {
                if (sd.Width == bbDesc.Width && sd.Height == bbDesc.Height && !sawSmallRtThisFrame &&
                    !firstFullSizeRt)
                {
                    firstFullSizeRt = renderTarget;
                    firstFullSizeRt->AddRef();
                }
                if (sd.Width < bbDesc.Width || sd.Height < bbDesc.Height)
                    sawSmallRtThisFrame = true;

                const float bbAspect = (float)bbDesc.Width / (float)bbDesc.Height;
                const float rtAspect = (float)sd.Width / (float)sd.Height;
                const float aspectDiff = fabsf(rtAspect - bbAspect);
                const uint32_t area = sd.Width * sd.Height;
                const uint32_t bbArea = bbDesc.Width * bbDesc.Height;
                if (aspectDiff <= 0.02f && area >= (bbArea * 8 / 10))
                {
                    const bool betterArea = area > g_SceneTargetBestArea;
                    const bool sameArea = area == g_SceneTargetBestArea;
                    const bool lowerPtr = g_SceneTargetThisFrame &&
                        reinterpret_cast<uintptr_t>(renderTarget) <
                        reinterpret_cast<uintptr_t>(g_SceneTargetThisFrame);
                    if (betterArea || (sameArea && (g_SceneTargetThisFrame == nullptr || lowerPtr)))
                    {
                        g_SceneTargetBestArea = area;
                        g_SceneTargetThisFrame = renderTarget;
                        g_LastSceneFullFrame = eFrameCounter;
                        if (g_RenderTargetManager.g_LastSceneFullSurface != renderTarget)
                        {
                            SAFE_RELEASE(g_RenderTargetManager.g_LastSceneFullSurface);
                            g_RenderTargetManager.g_LastSceneFullSurface = renderTarget;
                            g_RenderTargetManager.g_LastSceneFullSurface->AddRef();
                        }
                        if (g_RenderTargetManager.g_LastSceneFullSurface)
                        {
                            IDirect3DTexture9* sceneTex = nullptr;
                            if (SUCCEEDED(g_RenderTargetManager.g_LastSceneFullSurface->GetContainer(
                                    __uuidof(IDirect3DTexture9), reinterpret_cast<void**>(&sceneTex))) &&
                                sceneTex)
                            {
                                if (sceneTex != g_RenderTargetManager.g_LastSceneFullTex)
                                {
                                    SAFE_RELEASE(g_RenderTargetManager.g_LastSceneFullTex);
                                    g_RenderTargetManager.g_LastSceneFullTex = sceneTex;
                                }
                                else
                                {
                                    sceneTex->Release();
                                }
                            }
                        }
                        if (eFrameCounter != lastRtLogFrame)
                        {
                            lastRtLogFrame = eFrameCounter;
                            printf_s("[XNFS] ? Scene RT picked: %ux%u fmt=%u usage=%u ptr=%p\n",
                                     sd.Width, sd.Height, sd.Format, sd.Usage, renderTarget);
                        }
                    }
                }
            }

            if (renderTarget != g_RenderTargetManager.g_LastSceneSurface)
            {
                SAFE_RELEASE(g_RenderTargetManager.g_LastSceneSurface);
                g_RenderTargetManager.g_LastSceneSurface = renderTarget;
                g_RenderTargetManager.g_LastSceneSurface->AddRef();
            }
        }
    }

    return g_RenderTargetManager.oSetRenderTarget(device, index, renderTarget);
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND hwnd,
                             const RGNDATA* dirty)
{
    static uint32_t lastSceneLogFrame = 0;
    SetGameDevice(device);

    if (device)
    {
        IDirect3DSurface9* backBuffer = nullptr;
        if (SUCCEEDED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer)) && backBuffer)
        {
            D3DSURFACE_DESC bbDesc{};
            if (SUCCEEDED(backBuffer->GetDesc(&bbDesc)) &&
                (bbDesc.Width != g_RenderTargetManager.g_Width || bbDesc.Height != g_RenderTargetManager.g_Height))
            {
                printf_s("[XNFS] ? Blur RT resize: %ux%u -> %ux%u\n",
                         g_RenderTargetManager.g_Width, g_RenderTargetManager.g_Height,
                         bbDesc.Width, bbDesc.Height);
                g_RenderTargetManager.OnDeviceLost();
                g_RenderTargetManager.OnDeviceReset(device);
            }
        }
        SAFE_RELEASE(backBuffer);
    }

    void** vtable = *reinterpret_cast<void***>(device);

    if (!oEndScene)
    {
        DWORD oldProtect;
        oEndScene = reinterpret_cast<EndScene_t>(vtable[42]);
        VirtualProtect(&vtable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[42] = reinterpret_cast<void*>(&hkEndScene);
        VirtualProtect(&vtable[42], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[XNFS] ? hkEndScene installed via HookedPresent\n");
    }

    if (!g_RenderTargetManager.oSetRenderTarget)
    {
        DWORD oldProtect;
        g_RenderTargetManager.oSetRenderTarget =
            reinterpret_cast<RenderTargetManager::SetRenderTarget_t>(vtable[37]);
        VirtualProtect(&vtable[37], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[37] = reinterpret_cast<void*>(&hkSetRenderTarget);
        VirtualProtect(&vtable[37], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[XNFS] ? hkSetRenderTarget installed via HookedPresent\n");
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

bool IsProbablyValidManager(void* manager)
{
    if (!manager) return false;
    uintptr_t field4 = *((uintptr_t*)manager + 1);
    return field4 > 0x10000 && !IsBadReadPtr((void*)field4, 4);
}

bool SafeReloadFx(ID3DXEffect* fx, const char* context)
{
    if (!fx)
    {
        printf_s("[HotReload:%s] ❌ fx is NULL\n", context);
        return false;
    }

    void** vtable = nullptr;

    // Validate vtable access
    if (IsBadReadPtr(fx, sizeof(void*)) || IsBadReadPtr(*(void**)fx, sizeof(void*)))
    {
        printf_s("[HotReload:%s] ❌ fx or fx->vtable is unreadable\n", context);
        return false;
    }

    vtable = *(void***)fx;

    if (!vtable || IsBadCodePtr((FARPROC)vtable[0]))
    {
        printf_s("[HotReload:%s] ❌ Invalid or corrupt vtable: %p\n", context, vtable);
        return false;
    }

    // We assume fx is valid now — call the methods
    fx->OnResetDevice();
    ReloadBlurBindings(fx);
    return true;
}

void LogEffectStatus(ID3DXEffect* fx, const char* label)
{
    if (!fx)
    {
        printf_s("[Debug:%s] ❌ fx is NULL\n", label);
        return;
    }

    void** vtbl = nullptr;
    __try
    {
        vtbl = *(void***)fx;
        printf_s("[Debug:%s] fx = %p, vtable = %p, vtable[0] = %p\n", label, fx, vtbl, vtbl[0]);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf_s("[Debug:%s] ❌ EXCEPTION reading vtable from fx = %p\n", label, fx);
    }
}

void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject)
{
    LogApplyGraphicsSettingsCall(manager, vtObject, 1);

    if (g_WaitingForReset)
        return;

    if (IsValidThis(manager))
    {
        g_RenderTargetManager.g_ApplyGraphicsManagerThis = manager;
    }
    else
    {
        static void* lastInvalidMgr = nullptr;
        if (manager != lastInvalidMgr)
        {
            printf_s("[HookApplyGraphicsSettings] ❌ Invalid manager: %p\n", manager);
            lastInvalidMgr = manager;
        }
    }

    if (!IsValidThis(vtObject))
        return;

    // Only proceed if there's a new effect to inject
    if (g_RenderTargetManager.g_LastReloadedFx)
    {
        printf_s("[HotReload] 🔁 Patching vtObject = %p\n", vtObject);

        const auto& fx = g_RenderTargetManager.g_LastReloadedFx;
        bool validFx =
            fx &&
            reinterpret_cast<uintptr_t>(fx) > 0x10000 &&
            IsValidShaderPointer(fx);

        if (!validFx)
        {
            printf_s("[HotReload] ❌ g_LastReloadedFx is invalid — skipping\n");
        }
        else
        {
            auto& slotFx = g_SlotRetainedFx[62];
            auto rawPtr = reinterpret_cast<uintptr_t>(slotFx ? slotFx : nullptr);

            if (rawPtr < 0x10000 || rawPtr == 0x3f800000 || !IsValidShaderPointer(slotFx))
            {
                printf_s("[Patch] ❌ g_SlotRetainedFx[62] invalid or null (fx=0x%08X) — skipping patch\n",
                         (unsigned)rawPtr);
            }
            else
            {
                printf_s("[Patch] 📦 Using g_SlotRetainedFx[62] = %p\n", (void*)rawPtr);
                TryPatchSlotIfWritable(vtObject, 0x18C, fx);

                LogEffectStatus(fx, "BeforeReload");

                if (SafeReloadFx(fx, "ApplyGraphicsSettings"))
                {
                    if (g_pVisualTreatment && *g_pVisualTreatment)
                    {
                        IVisualTreatment_Reset(*g_pVisualTreatment);
                        printf_s("[HotReload] 🔁 Called IVisualTreatment::Reset()\n");
                    }
                }
                else
                {
                    printf_s("[HotReload] ❌ ReloadHandles failed — skipping Reset\n");
                }
            }
        }
    }

    LogApplyGraphicsSettingsCall(manager, vtObject, 2);

    if (ApplyGraphicsSettingsOriginal)
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
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
    g_RenderTargetManager.oReset = (Reset_t)vtable[16];
    VirtualProtect(&vtable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    vtable[16] = (void*)&hkReset;
    VirtualProtect(&vtable[16], sizeof(void*), oldProtect, &oldProtect);
    printf_s("[Init] Hooked IDirect3DDevice9::Reset (deferred)\n");

    for (int i = 0; i < 200 && !g_D3DXHooksInstalled; ++i)
    {
        TryInstallD3DXHooks();
        if (!g_D3DXHooksInstalled)
            Sleep(50);
    }
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
            g_RenderTargetManager.RealCreateFromResource = (RenderTargetManager::D3DXCreateEffectFromResourceAFn)addr;
            injector::MakeCALL(call_D3DXCreateEffectFromResourceA_ADDRESS, HookedCreateFromResource, true);

            void* fileAddr = GetProcAddress(d3dx, "D3DXCreateEffectFromFileA");
            void* effectAddr = GetProcAddress(d3dx, "D3DXCreateEffect");
            if (fileAddr)
            {
                RealCreateFromFileA = reinterpret_cast<D3DXCreateEffectFromFileAFn>(fileAddr);
                if (MH_Initialize() == MH_OK)
                {
                    if (MH_CreateHook(fileAddr, HookedCreateFromFileA,
                                      reinterpret_cast<void**>(&RealCreateFromFileA)) == MH_OK &&
                        MH_EnableHook(fileAddr) == MH_OK)
                    {
                        printf_s("[Init] Hooked D3DXCreateEffectFromFileA\n");
                        g_D3DXHooksInstalled = true;
                    }
                    else
                    {
                        printf_s("[Init] ? Failed to hook D3DXCreateEffectFromFileA\n");
                    }
                }
                else
                {
                    printf_s("[Init] ? MH_Initialize failed\n");
                }
            }

            if (effectAddr)
            {
                if (!RealCreateEffect)
                    RealCreateEffect = reinterpret_cast<D3DXCreateEffectFn>(effectAddr);
                const MH_STATUS createStatus =
                    MH_CreateHook(effectAddr, HookedCreateEffect,
                                  reinterpret_cast<void**>(&RealCreateEffect));
                const MH_STATUS enableStatus =
                    (createStatus == MH_OK || createStatus == MH_ERROR_ALREADY_CREATED)
                        ? MH_EnableHook(effectAddr)
                        : createStatus;

                if ((createStatus == MH_OK || createStatus == MH_ERROR_ALREADY_CREATED) &&
                    (enableStatus == MH_OK || enableStatus == MH_ERROR_ENABLED))
                {
                    printf_s("[Init] Hooked D3DXCreateEffect\n");
                }
                else
                {
                    printf_s("[Init] ? Failed to hook D3DXCreateEffect: create=%d enable=%d\n",
                             (int)createStatus,
                             (int)enableStatus);
                }
            }

            {
                const MH_STATUS initStatus = MH_Initialize();
                if (initStatus != MH_OK && initStatus != MH_ERROR_ALREADY_INITIALIZED)
                {
                    printf_s("[Init] ? MH_Initialize failed for IVisualTreatment::Get: %d\n",
                             (int)initStatus);
                }
                else
                {
                    const MH_STATUS createStatus =
                        MH_CreateHook((LPVOID)0x006DFAF0, HookedIVisualTreatmentGet,
                                      reinterpret_cast<void**>(&RealIVisualTreatmentGet));
                    const MH_STATUS enableStatus =
                        (createStatus == MH_OK || createStatus == MH_ERROR_ALREADY_CREATED)
                            ? MH_EnableHook((LPVOID)0x006DFAF0)
                            : createStatus;

                    if ((createStatus == MH_OK || createStatus == MH_ERROR_ALREADY_CREATED) &&
                        (enableStatus == MH_OK || enableStatus == MH_ERROR_ENABLED))
                    {
                        printf_s("[Init] Hooked IVisualTreatment::Get\n");
                    }
                    else
                    {
                        printf_s("[Init] ? Failed to hook IVisualTreatment::Get: create=%d enable=%d\n",
                                 (int)createStatus,
                                 (int)enableStatus);
                    }
                }
            }

            injector::MakeJMP(0x00750B10, RenderDispatchHook, true);
            PatchMotionBlurFix();

            ApplyGraphicsManagerMainOriginal = (decltype(ApplyGraphicsManagerMainOriginal))
                call_ApplyGraphicsManagerMainOriginal_ADDRESS;
            printf_s("[Init] ApplyGraphicsManagerMainOriginal set to 0x004F17F0\n");

            ApplyGraphicsSettingsOriginal = reinterpret_cast<ApplyGraphicsSettingsFn>(
                sub_ApplyGraphicsSettingsFn_ADDRESS);
            injector::MakeCALL(call_ApplyGraphicsSettingsFn_ADDRESS, HookApplyGraphicsSettings, true);

            CreateThread(nullptr, 0, DeferredHookThread, nullptr, 0, nullptr);

            printf_s("[Init] Hooked D3DXCreateEffectFromResourceA\n");
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        printf_s("[Shutdown] Cleaning up...\n");
        g_RenderTargetManager.g_DeviceResetInProgress = true;

        for (auto& [name, fx] : g_RenderTargetManager.g_ActiveEffects)
        {
            if (fx && IsValidShaderPointer(fx))
            {
                printf_s("[Shutdown] Releasing shader: %s (%p)\n", name.c_str(), fx);
                fx->Release();
            }
        }
        g_RenderTargetManager.g_ActiveEffects.clear();

        if (g_RenderTargetManager.g_LastReloadedFx &&
            IsValidShaderPointer(g_RenderTargetManager.g_LastReloadedFx))
        {
            g_RenderTargetManager.g_LastReloadedFx->Release();
            g_RenderTargetManager.g_LastReloadedFx = nullptr;
        }

        for (int i = 0; i < 62; ++i)
        {
            if (g_SlotRetainedFx[i] && IsValidShaderPointer(g_SlotRetainedFx[i]))
            {
                g_SlotRetainedFx[i]->Release();
                g_SlotRetainedFx[i] = nullptr;
            }
        }

        g_RenderTargetManager.g_ApplyGraphicsManagerThis = nullptr;
        g_Device = nullptr;
        g_LastEView = nullptr;
        g_pVisualTreatment = nullptr;

        // Avoid touching engine tables during detach.

        printf_s("[Shutdown] ✅ Cleanup complete.\n");
    }

    return TRUE;
}
