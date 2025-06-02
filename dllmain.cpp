// dllmain.cpp - Backbuffer hook for NFS MW (2005)
#include "Hooks.h"
#include "globals.h"
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

#include "MinHook.h"
#pragma comment(lib, "d3d9.lib")

// -------------------- GLOBALS --------------------
#define CHECKMARK(x) ((x) ? "OK" : "MISSING")

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

HMODULE g_hModule = nullptr;
IDirect3DTexture9* g_MotionBlurTex = nullptr;

int g_ApplyDelayCounter = 0;
bool g_ApplyScheduled = false;
int g_ApplyGraphicsTriggerDelay = 0;
static size_t lastCount = 0;

std::unordered_map<std::string, LPD3DXEFFECT> g_ActiveEffects;

using EndScene_t = HRESULT(WINAPI*)(LPDIRECT3DDEVICE9);
EndScene_t oEndScene = nullptr;

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*,
    LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

void* g_ApplyGraphicsManagerThis = nullptr;

ID3DXEffect* g_LastReloadedFx = nullptr;

typedef void (__fastcall*ApplyGraphicsSettingsFn)(void* ecx, void* edx, void* arg1);
ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr; // ✅ definition

typedef int (__thiscall*ApplyGraphicsManagerMain_t)(void* thisptr);
ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal = nullptr; // ✅ definition

typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
Reset_t oReset = nullptr;

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

void ReloadBlurBindings(ID3DXEffect* fx)
{
    if (!fx || !g_CurrentBlurTex) return;

    D3DXEFFECT_DESC desc;
    if (SUCCEEDED(fx->GetDesc(&desc)))
    {
        printf_s("[BlurRebind] Attached shader has %u techniques and %u parameters\n",
                 desc.Techniques, desc.Parameters);
    }

    fx->SetTexture("MISCMAP3", g_CurrentBlurTex);

    D3DXVECTOR4 blurParams(0.5f, 0.2f, 1.0f, 0);
    fx->SetVector("BlurParams", &blurParams);

    if (FAILED(fx->SetTechnique("visualtreatment_branching")))
    {
        printf_s("[BlurRebind] ⚠️ Failed to set technique 'visualtreatment_branching'\n");
    }

    printf_s("[XNFS] 🔁 ReloadBlurBindings applied to %p\n", fx);
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
    HRESULT hr = RealCreateFromResource(
        device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);

    if (SUCCEEDED(hr) && outEffect && *outEffect && pResource)
    {
        g_ActiveEffects[pResource] = *outEffect;
        printf_s("✅ Shader created and tracked: %s (%p)\n", pResource, *outEffect);

        ReloadBlurBindings(*outEffect); // 🔁 Cleaner
    }

    return hr;
}

DWORD_PTR FindPattern(const char* moduleName, const char* pattern, const char* mask)
{
    MODULEINFO mInfo = {};
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (hModule && GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO)))
    {
        BYTE* base = static_cast<BYTE*>(mInfo.lpBaseOfDll);
        DWORD size = mInfo.SizeOfImage;

        size_t patternLen = strlen(mask);

        for (DWORD i = 0; i <= size - patternLen; i++)
        {
            bool found = true;
            for (size_t j = 0; j < patternLen; j++)
            {
                if (mask[j] != '?' && pattern[j] != *(char*)(base + i + j))
                {
                    found = false;
                    break;
                }
            }

            if (found)
            {
                return reinterpret_cast<DWORD_PTR>(base + i);
            }
        }
    }
    else
    {
        printf_s("Failed to get module info for %s.\n", moduleName);
    }

    return 0;
}

void OnDeviceReset(LPDIRECT3DDEVICE9 device)
{
    if (!device) return;

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
}

void OnDeviceLost()
{
    SAFE_RELEASE(g_MotionBlurTexA);
    SAFE_RELEASE(g_MotionBlurTexB);
    SAFE_RELEASE(g_CurrentBlurSurface);
    printf_s("[OnDeviceLost] Released motion blur resources\n");
}

void RenderBlurPass(IDirect3DDevice9* device, LPD3DXEFFECT blurFx)
{
    if (!blurFx || !g_MotionBlurTexA || !g_MotionBlurTexB) return;

    IDirect3DSurface9* oldRT = nullptr;
    device->GetRenderTarget(0, &oldRT);

    // Setup quad vertices (fullscreen triangle strip)
    struct Vertex
    {
        float x, y, z, rhw;
        float u, v;
    } vertices[4] = {
        {-0.5f, -0.5f, 0, 1, 0, 0},
        {g_Width - 0.5f, -0.5f, 0, 1, 1, 0},
        {-0.5f, g_Height - 0.5f, 0, 1, 0, 1},
        {g_Width - 0.5f, g_Height - 0.5f, 0, 1, 1, 1}
    };
    // Prepare fullscreen quad...
    
    // Prepare ping-pong target
    IDirect3DTexture9* srcTex = g_UseTexA ? g_MotionBlurTexA : g_MotionBlurTexB;
    IDirect3DTexture9* dstTex = g_UseTexA ? g_MotionBlurTexB : g_MotionBlurTexA;

    IDirect3DSurface9* dstSurface = nullptr;
    if (FAILED(dstTex->GetSurfaceLevel(0, &dstSurface)) || !dstSurface)
        return;

    device->SetRenderTarget(0, dstSurface);
    device->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 1.0f, 0);

    // Set shader inputs
    if (FAILED(blurFx->SetTexture("PrevFrame", srcTex)))
        printf_s("⚠️ Failed to set PrevFrame\n");

    D3DXVECTOR4 blurParams(0.5f, 0.0005f, 0.0f, 0.0f);
    if (FAILED(blurFx->SetVector("BlurParams", &blurParams)))
        printf_s("⚠️ Failed to set BlurParams\n");

    blurFx->CommitChanges(); // ✅ flush

    // Draw
    blurFx->Begin(nullptr, 0);
    blurFx->BeginPass(0);
    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
    blurFx->EndPass();
    blurFx->End();

    dstSurface->Release();

    // Restore RT
    if (oldRT)
    {
        device->SetRenderTarget(0, oldRT);
        oldRT->Release();
    }

    // Ping-pong swap
    g_UseTexA = !g_UseTexA;
    g_CurrentBlurTex = dstTex;
    g_CurrentBlurTex->GetSurfaceLevel(0, &g_CurrentBlurSurface);
}

HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params)
{
    printf_s("[hkReset] Called\n");
    OnDeviceLost(); // Release
    HRESULT hr = oReset(device, params);
    if (SUCCEEDED(hr))
    {
        OnDeviceReset(device); // Recreate
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

bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx) return false;
    __try
    {
        void* vtbl = *(void**)fx;
        return vtbl != nullptr;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsValidThis(void* ptr)
{
    if (!ptr)
        return false;

    __try
    {
        void* vtable = *(void**)ptr;
        if (!vtable)
            return false;

        void* fn = *((void**)vtable); // vtable[0]
        return fn != nullptr && !IsBadCodePtr((FARPROC)fn);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
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
        printf_s("[ReplaceShaderSlot] ✅ Wrote new ID3DXEffect* (0x%p) into offset +0x%X (slot %d) (no VirtualProtect needed)\n",
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
    // 1) Capture the device once
    if (!g_Device)
        g_Device = device;

    // 2) If we have a newly reloaded effect, rebind it (but only if it’s valid)
    static ID3DXEffect* lastFxSeen = nullptr;
    if (g_LastReloadedFx && g_LastReloadedFx != lastFxSeen && IsValidShaderPointer(g_LastReloadedFx))
    {
        // 2a) Rebind bloom parameters on the newly reloaded FX
        if (g_CurrentBlurTex)
        {
            // Always check validity before any D3DX call:
            if (IsValidShaderPointer(g_LastReloadedFx))
            {
                g_LastReloadedFx->SetTexture("MISCMAP3", g_CurrentBlurTex);

                D3DXVECTOR4 blurParams(0.5f, 0.2f, 1.0f, 0);
                g_LastReloadedFx->SetVector("BlurParams", &blurParams);

                // Only set technique if it actually exists
                if (SUCCEEDED(g_LastReloadedFx->SetTechnique("visualtreatment_branching")))
                {
                    printf_s("[hkEndScene] [BlurRebind] ✅ Reattached bloom parameters to FX %p\n", g_LastReloadedFx);
                }
                else
                {
                    printf_s("[hkEndScene] [BlurRebind] ⚠️ Could not set technique on FX %p\n", g_LastReloadedFx);
                }
            }
            else
            {
                printf_s("[hkEndScene] [BlurRebind] ❌ g_LastReloadedFx is invalid\n");
            }
        }

        // 2b) Now attempt to force-patch the VisualTreatment slot at +0x18C
        if (g_pVisualTreatment &&
            !IsBadReadPtr(g_pVisualTreatment, sizeof(void*)) &&
            *((void**)g_pVisualTreatment) &&
            IsValidThis(*((void**)g_pVisualTreatment)))
        {
            void* vtObj = *((void**)g_pVisualTreatment);
            BYTE* base = reinterpret_cast<BYTE*>(vtObj);
            const int offset = 0x18C;
            BYTE* slotPtr = base + offset;

            if (!IsBadWritePtr(slotPtr, sizeof(ID3DXEffect*)))
            {
                // Again, check FX validity before writing
                if (IsValidShaderPointer(g_LastReloadedFx))
                {
                    // Write raw ID3DXEffect* here
                    ID3DXEffect** target = reinterpret_cast<ID3DXEffect**>(slotPtr);
                    g_LastReloadedFx->AddRef();
                    *target = g_LastReloadedFx;

                    printf_s("[hkEndScene] [Fallback] 🔧 Patched vtObj+0x18C = %p\n", g_LastReloadedFx);

                    // Call Reset safely
                    if (!IsBadReadPtr(g_pVisualTreatment, sizeof(void*)) &&
                        *((void**)g_pVisualTreatment) &&
                        IsValidThis(*((void**)g_pVisualTreatment)))
                    {
                        VisualTreatment_Reset();
                        *(BYTE*)0x00982C39 = 1;
                        printf_s("[hkEndScene] [Fallback] ✅ Called VisualTreatment_Reset and set LoadedFlagMaybe\n");
                    }
                }
                else
                {
                    printf_s("[hkEndScene] [Fallback] ❌ g_LastReloadedFx invalid when patching slot\n");
                }
            }
            else
            {
                printf_s("[hkEndScene] [Fallback] ❌ Cannot write to vtObj+0x18C (%p)\n", slotPtr);
            }
        }
        else
        {
            printf_s("[hkEndScene] [Fallback] ❌ g_pVisualTreatment or vtObj is invalid; skipping slot patch\n");
        }

        lastFxSeen = g_LastReloadedFx;
    }

    // 3) Copy backbuffer → g_CurrentBlurSurface for next‐frame blur, if available
    if (g_CurrentBlurSurface)
    {
        IDirect3DSurface9* backBuffer = nullptr;
        if (SUCCEEDED(device->GetRenderTarget(0, &backBuffer)))
        {
            device->StretchRect(backBuffer, nullptr, g_CurrentBlurSurface, nullptr, D3DTEXF_LINEAR);
            backBuffer->Release();
        }

        // 4) Render the motion‐blur pass using the VisualTreatment FX, if it’s valid
        auto it = g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
        if (it != g_ActiveEffects.end() && IsValidShaderPointer(it->second))
        {
            RenderBlurPass(device, it->second);
        }
        else if (it != g_ActiveEffects.end())
        {
            printf_s("[hkEndScene] [RenderBlurPass] ⚠️ Effect pointer invalid: %p\n", it->second);
        }

        // 5) If the set of active effects changed, reapply bloom binds to each valid FX
        static size_t lastCount = 0;
        if (g_ActiveEffects.size() != lastCount)
        {
            for (auto& kv : g_ActiveEffects)
            {
                ID3DXEffect* fx = kv.second;
                if (IsValidShaderPointer(fx) && g_CurrentBlurTex)
                {
                    fx->SetTexture("MISCMAP3", g_CurrentBlurTex);

                    D3DXVECTOR4 blurParams(0.5f, 0.2f, 1.0f, 0);
                    fx->SetVector("BlurParams", &blurParams);

                    if (SUCCEEDED(fx->SetTechnique("visualtreatment_branching")))
                    {
                        printf_s("[hkEndScene] [ReloadBlurBindings] ✅ Rebound to FX %p (%s)\n",
                                 fx, kv.first.c_str());
                    }
                    else
                    {
                        printf_s("[hkEndScene] [ReloadBlurBindings] ⚠️ Could not set technique on FX %p (%s)\n",
                                 fx, kv.first.c_str());
                    }
                }
                else
                {
                    printf_s("[hkEndScene] [ReloadBlurBindings] ⚠️ Skipping bind for invalid FX pointer: %p (%s)\n",
                             kv.second, kv.first.c_str());
                }
            }
            lastCount = g_ActiveEffects.size();
        }
    }

    // 6) Finally, call the real EndScene—nothing else here can crash
    return oEndScene(device);
}

HRESULT WINAPI hkEndScene1(LPDIRECT3DDEVICE9 device)
{
    if (!g_Device) g_Device = device;

    OnFramePresent();
    
    if (g_CurrentBlurSurface)
    {
        // 1. Copy current backbuffer to g_CurrentBlurSurface (for snapshotting this frame)
        IDirect3DSurface9* backBuffer = nullptr;
        if (SUCCEEDED(device->GetRenderTarget(0, &backBuffer)))
        {
            device->StretchRect(backBuffer, nullptr, g_CurrentBlurSurface, nullptr, D3DTEXF_LINEAR);
            backBuffer->Release();
        }

        // 2. Render motion blur pass (quad that blends PrevFrame + Current)
        auto it = g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
        if (it != g_ActiveEffects.end() && it->second)
        {
            RenderBlurPass(device, it->second); // render into ping-pong texture
        }

        // 3. Bind the updated blur texture (after quad render) to all active shaders
        static size_t lastCount = 0;
        if (g_ActiveEffects.size() != lastCount)
        {
            for (const auto& pair : g_ActiveEffects)
            {
                ID3DXEffect* fx = pair.second;
                if (fx) ReloadBlurBindings(fx);
            }
            lastCount = g_ActiveEffects.size();
        }
    }

    return oEndScene(device);
}

void HookEndScene()
{
    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) return;

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetForegroundWindow();

    IDirect3DDevice9* pDevice = nullptr;
    if (SUCCEEDED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice)))
    {
        void** vtable = *reinterpret_cast<void***>(pDevice);

        // Hook EndScene (vtable[42])
        oEndScene = (EndScene_t)vtable[42];
        DWORD oldProtect;
        VirtualProtect(&vtable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[42] = (void*)&hkEndScene;
        VirtualProtect(&vtable[42], sizeof(void*), oldProtect, &oldProtect);
        printf_s("EndScene hook installed\n");

        // Hook Reset (vtable[16])
        oReset = (Reset_t)vtable[16];
        VirtualProtect(&vtable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[16] = (void*)&hkReset;
        VirtualProtect(&vtable[16], sizeof(void*), oldProtect, &oldProtect);
        printf_s("Reset hook installed\n");

        pDevice->Release();
    }

    pD3D->Release();
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

        printf_s("[HookApplyGraphicsSettings] 🧩 manager = %p | vtable[0] = %p | field4 = 0x%08X\n", manager, vfn0, field4);
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
    // Always call the original; no slot‐patching here.
    if (IsValidThis(manager))
    {
        LogApplyGraphicsSettingsCall(manager, vtObject, 1);
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
    }
    // (If it's not valid, we skip silently to avoid crashes.)
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
            printf_s("[HookApplyGraphicsSettings] ⚠️ Skipping: manager = %p, field4 = 0x%08X (invalid)\n", manager, field4);
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
        !IsBadReadPtr((void*)field4, 4) &&  // field4 must be readable if used internally
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

    // ✅ Only hook Reset
    DWORD oldProtect;
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

            // CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HotkeyThread, nullptr, 0, nullptr);
            CreateThread(nullptr, 0, DeferredHookThread, nullptr, 0, nullptr);

            printf_s("[Init] Hooked D3DXCreateEffectFromResourceA\n");
        }
    }
    return TRUE;
}
