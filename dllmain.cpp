// dllmain.cpp - Backbuffer hook for NFS MW (2005)
#include <windows.h>
#include <filesystem>
#include <string>
#include <cctype>
#include <cstdio>
#include <cmath>
#include <d3dx9math.h>
#include <psapi.h>  // Required for MODULEINFO and GetModuleInformation
#include <d3d9.h>
#include <unordered_map>
#include <unordered_set>

#include "includes/injector/injector.hpp"
#include "Modules/minhook/include/MinHook.h"

#include "Hooks.h"
#include "RenderTargetManager.h"
#include "Globals.h"
#include "MotionBlurPass.h"
#include "Validators.h"
#include "ExposureStandalone.h"

// -------------------- GLOBALS --------------------
#define CHECKMARK(x) ((x) ? "OK" : "MISSING")

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name);

static ExposureStandalone::ExposureSampler g_ExposureSampler;
static bool g_ExposureInitialized = false;
static DWORD g_ExposureLastTick = 0;

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
    printf_s("[XNFS] ? NextGenMotionBlur patch applied (keep blur path)\n");
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

using Sub6D3B80_t = int(__cdecl*)(void* ctx, int a2, void* a3, void* a4, int a5, int a6, int a7);
static Sub6D3B80_t g_Sub6D3B80 = reinterpret_cast<Sub6D3B80_t>(0x006D3B80);

using Sub4499E0_t = int(__thiscall*)(void* self, float dt);
static Sub4499E0_t g_Sub4499E0 = reinterpret_cast<Sub4499E0_t>(0x004499E0);
static Sub4499E0_t g_Sub4499E0_VtableOrig = nullptr;
using Sub4467A0_t = void(__thiscall*)(void* self, int arg0, float* outVec3);
static Sub4467A0_t g_Sub4467A0 = reinterpret_cast<Sub4467A0_t>(0x004467A0);
static D3DXMATRIX g_LastViewMatrix{};

static bool IsMotionBlurVariant(int a6, int a7)
{
    return (a6 == 8 && a7 == 7) || (a6 == 0x0E && a7 == 0x0F);
}

static void PatchCall(uintptr_t callSite, void* newFunc)
{
    DWORD oldProt{};
    VirtualProtect(reinterpret_cast<void*>(callSite), 5, PAGE_EXECUTE_READWRITE, &oldProt);
    *reinterpret_cast<uint8_t*>(callSite) = 0xE8;
    const int32_t rel = static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(newFunc) - (callSite + 5));
    *reinterpret_cast<int32_t*>(callSite + 1) = rel;
    VirtualProtect(reinterpret_cast<void*>(callSite), 5, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(callSite), 5);
}

static int __fastcall hkSub4499E0(void* self, void* /*edx*/, float dt)
{
    int ret = g_Sub4499E0(self, dt);
    if (self)
    {
        const char* base = reinterpret_cast<const char*>(self);
        void* node = *reinterpret_cast<void* const*>(base + 0x0C);
        if (node)
        {
            const float* v = reinterpret_cast<const float*>(reinterpret_cast<const char*>(node) + 0xB0);
            g_MotionVec[0] = v[0];
            g_MotionVec[1] = v[1];
            g_MotionVec[2] = v[2];
            g_MotionVec[3] = 0.0f;
        }
    }
    static DWORD s_lastLogTick = 0;
    const DWORD nowTick = GetTickCount();
    if (nowTick - s_lastLogTick >= 1000)
    {
        s_lastLogTick = nowTick;
        printf_s("[MotionVec] sub_4499E0 (%.4f, %.4f, %.4f)\n",
                 g_MotionVec[0], g_MotionVec[1], g_MotionVec[2]);
    }
    return ret;
}

static int __cdecl XNFS_Sub6D3B80_Hook(void* ctx, int a2, void* a3, void* a4, int a5, int a6, int a7)
{
    if (!IsMotionBlurVariant(a6, a7) || g_RenderTargetManager.g_DeviceResetInProgress)
        return g_Sub6D3B80(ctx, a2, a3, a4, a5, a6, a7);

    IDirect3DDevice9* dev = g_Device;
    if (!dev)
        return 0;

    IDirect3DSurface9* dstRT = nullptr;
    IDirect3DBaseTexture9* src0 = nullptr;
    IDirect3DBaseTexture9* src1 = nullptr;

    dev->GetRenderTarget(0, &dstRT);
    dev->GetTexture(0, &src0);
    dev->GetTexture(1, &src1);

    bool addedSrc0Ref = false;
    bool addedSrc1Ref = false;
    if (g_RenderTargetManager.g_LastSceneFullTex)
    {
        if (src0)
            src0->Release();
        src0 = g_RenderTargetManager.g_LastSceneFullTex;
        src0->AddRef();
        addedSrc0Ref = true;
    }
    else if (!src0 && g_RenderTargetManager.g_SceneCopyTex)
    {
        src0 = g_RenderTargetManager.g_SceneCopyTex;
        src0->AddRef();
        addedSrc0Ref = true;
    }
    if (!src1 && g_RenderTargetManager.g_CurrentBlurTex)
    {
        src1 = g_RenderTargetManager.g_CurrentBlurTex;
        src1->AddRef();
        addedSrc1Ref = true;
    }

    static DWORD s_lastLogTick = 0;
    DWORD nowTick = GetTickCount();
    if (nowTick - s_lastLogTick >= 1000)
    {
        s_lastLogTick = nowTick;
        printf_s("[BlurOverride] rt=%p src0=%p src1=%p fallback0=%d fallback1=%d\n",
                 dstRT, src0, src1, addedSrc0Ref ? 1 : 0, addedSrc1Ref ? 1 : 0);
    }

    if (dstRT && src0)
    {
        MotionBlurPass::RenderBlurOverride(dev, src0, src1, dstRT);

        static DWORD s_lastBlurLogTick = 0;
        const DWORD nowTick = GetTickCount();
        if (nowTick - s_lastBlurLogTick >= 1000)
        {
            s_lastBlurLogTick = nowTick;
            printf_s("[BlurOverride] outTex=%p historyReady=%d motionAmount=%.3f\n",
                     g_RenderTargetManager.g_CurrentBlurTex,
                     g_MotionBlurHistoryReady ? 1 : 0,
                     g_MotionBlurAmount);
        }
    }

    if (src1 && (addedSrc1Ref || src1 != nullptr)) src1->Release();
    if (src0 && (addedSrc0Ref || src0 != nullptr)) src0->Release();
    if (dstRT) dstRT->Release();

    return 0;
}

static void InstallSub6D3B80BlurHook()
{
    PatchCall(0x006DBE8B, reinterpret_cast<void*>(&XNFS_Sub6D3B80_Hook));
    PatchCall(0x006DBEB0, reinterpret_cast<void*>(&XNFS_Sub6D3B80_Hook));
}

static void __fastcall hkSub4467A0(void* self, void* /*edx*/, int arg0, float* outVec3)
{
    g_Sub4467A0(self, arg0, outVec3);
    if (!outVec3)
        return;

    static DWORD s_lastCallLog = 0;
    const DWORD callTick = GetTickCount();
    if (callTick - s_lastCallLog >= 1000)
    {
        s_lastCallLog = callTick;
        printf_s("[MotionVec] sub_4467A0 called (out=%p)\n", outVec3);
    }

    const float curr[3] = {outVec3[0], outVec3[1], outVec3[2]};
    if (!g_HasLastCameraPos)
    {
        g_LastCameraPos[0] = curr[0];
        g_LastCameraPos[1] = curr[1];
        g_LastCameraPos[2] = curr[2];
        g_HasLastCameraPos = true;
        return;
    }

    g_MotionVec[0] = curr[0] - g_LastCameraPos[0];
    g_MotionVec[1] = curr[1] - g_LastCameraPos[1];
    g_MotionVec[2] = curr[2] - g_LastCameraPos[2];
    g_MotionVec[3] = 0.0f;

    g_LastCameraPos[0] = curr[0];
    g_LastCameraPos[1] = curr[1];
    g_LastCameraPos[2] = curr[2];

    static DWORD s_lastLogTick = 0;
    const DWORD nowTick = GetTickCount();
    if (nowTick - s_lastLogTick >= 1000)
    {
        s_lastLogTick = nowTick;
        printf_s("[MotionVec] sub_4467A0 vec (%.4f, %.4f, %.4f)\n",
                 g_MotionVec[0], g_MotionVec[1], g_MotionVec[2]);
    }
}

static HRESULT WINAPI hkSetTransform(LPDIRECT3DDEVICE9 device, D3DTRANSFORMSTATETYPE state,
                                     const D3DMATRIX* matrix)
{
    if (state == D3DTS_VIEW && matrix)
    {
        g_LastViewMatrix = *reinterpret_cast<const D3DXMATRIX*>(matrix);
        g_HasLastViewMatrix = true;
    }
    return g_RenderTargetManager.oSetTransform
        ? g_RenderTargetManager.oSetTransform(device, state, matrix)
        : D3D_OK;
}

static void InstallMotionVectorHook()
{
    const MH_STATUS initStatus = MH_Initialize();
    if (initStatus != MH_OK && initStatus != MH_ERROR_ALREADY_INITIALIZED)
    {
        printf_s("[Init] ❌ MH_Initialize failed for motion vector hook: %d\n", (int)initStatus);
        return;
    }

    if (MH_CreateHook(reinterpret_cast<LPVOID>(0x004499E0),
                      reinterpret_cast<LPVOID>(&hkSub4499E0),
                      reinterpret_cast<LPVOID*>(&g_Sub4499E0)) == MH_OK &&
        MH_EnableHook(reinterpret_cast<LPVOID>(0x004499E0)) == MH_OK)
    {
        printf_s("[Init] ✅ sub_4499E0 motion vector hook installed\n");
    }
    else
    {
        printf_s("[Init] ❌ Failed to hook sub_4499E0\n");
    }
}

static void InstallMotionVectorHook4467A0()
{
    const MH_STATUS initStatus = MH_Initialize();
    if (initStatus != MH_OK && initStatus != MH_ERROR_ALREADY_INITIALIZED)
    {
        printf_s("[Init] ❌ MH_Initialize failed for sub_4467A0 hook: %d\n", (int)initStatus);
        return;
    }

    if (MH_CreateHook(reinterpret_cast<LPVOID>(0x004467A0),
                      reinterpret_cast<LPVOID>(&hkSub4467A0),
                      reinterpret_cast<LPVOID*>(&g_Sub4467A0)) == MH_OK &&
        MH_EnableHook(reinterpret_cast<LPVOID>(0x004467A0)) == MH_OK)
    {
        printf_s("[Init] ✅ sub_4467A0 motion vector hook installed\n");
    }
    else
    {
        printf_s("[Init] ❌ Failed to hook sub_4467A0\n");
    }
}

static void InstallMotionVectorVtablePatch()
{
    constexpr uintptr_t kSub4499E0_VtableSlot = 0x00893530;
    auto* slot = reinterpret_cast<void**>(kSub4499E0_VtableSlot);
    DWORD oldProtect = 0;
    if (!VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf_s("[Init] ❌ Failed to unprotect motion vtable slot\n");
        return;
    }

    g_Sub4499E0_VtableOrig = reinterpret_cast<Sub4499E0_t>(*slot);
    *slot = reinterpret_cast<void*>(&hkSub4499E0);
    VirtualProtect(slot, sizeof(void*), oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));
    printf_s("[Init] ✅ sub_4499E0 vtable slot patched (orig=%p)\n", g_Sub4499E0_VtableOrig);
}

static void CopyScene(IDirect3DDevice9* device)
{
    if (!device) return;

    if (!g_RenderTargetManager.g_BackBufferSurface ||
        !g_RenderTargetManager.g_SceneCopySurface)
        return;

    // copy backbuffer (final, post-VT) -> SceneCopySurface
    const HRESULT hr = device->StretchRect(
        g_RenderTargetManager.g_BackBufferSurface,
        nullptr,
        g_RenderTargetManager.g_SceneCopySurface,
        nullptr,
        D3DTEXF_NONE
    );
    if (FAILED(hr))
        return;

    // publish "final scene copy" for blur-generation ONLY
    g_RenderTargetManager.g_SceneCopyTex   = g_RenderTargetManager.g_SceneCopyTex; // (no-op, just clarity)
    g_RenderTargetManager.g_LastSceneFullFrame = eFrameCounter;
}

static void UpdateMotionBlurFromEView()
{
    static char* eViewsBase = reinterpret_cast<char*>(0x009195E0);
    void* viewPtr = nullptr;
    for (int i = 0; i <= 0x16; ++i)
    {
        char* entry = eViewsBase + (i * 0x70);
        if (IsBadReadPtr(entry, sizeof(void*)))
            continue;
        void* candidate = *reinterpret_cast<void**>(entry);
        if (candidate && reinterpret_cast<uintptr_t>(candidate) > 0x10000)
        {
            viewPtr = candidate;
            break;
        }
    }
    if (!viewPtr)
    {
        g_MotionBlurAmount = 0.0f;
        g_MotionVec[0] = g_MotionVec[1] = g_MotionVec[2] = g_MotionVec[3] = 0.0f;
        return;
    }

    auto view = static_cast<char*>(viewPtr);
    if (IsBadReadPtr(view, 0x200))
    {
        g_MotionBlurAmount = 0.0f;
        g_MotionVec[0] = g_MotionVec[1] = g_MotionVec[2] = g_MotionVec[3] = 0.0f;
        return;
    }
    // eView deltas identified by probe: 0x1D0, 0x1D4, 0x1D8
    if (IsBadReadPtr(view + 0x1D0, sizeof(float) * 3))
    {
        g_MotionBlurAmount = 0.0f;
        g_MotionVec[0] = g_MotionVec[1] = g_MotionVec[2] = g_MotionVec[3] = 0.0f;
        return;
    }
    float curr[3] = {
        *reinterpret_cast<float*>(view + 0x1D0),
        *reinterpret_cast<float*>(view + 0x1D4),
        *reinterpret_cast<float*>(view + 0x1D8)
    };
    static float last[3] = {};
    static bool hasLast = false;
    if (!hasLast)
    {
        last[0] = curr[0];
        last[1] = curr[1];
        last[2] = curr[2];
        hasLast = true;
        g_MotionBlurAmount = 0.0f;
        g_MotionVec[0] = g_MotionVec[1] = g_MotionVec[2] = g_MotionVec[3] = 0.0f;
        return;
    }

    float dx = curr[0] - last[0];
    float dy = curr[1] - last[1];
    float dz = curr[2] - last[2];
    last[0] = curr[0];
    last[1] = curr[1];
    last[2] = curr[2];

    float mag = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (!std::isfinite(mag) || mag <= 0.0001f)
    {
        g_MotionBlurAmount = 0.0f;
        g_MotionVec[0] = g_MotionVec[1] = g_MotionVec[2] = g_MotionVec[3] = 0.0f;
        return;
    }

    float invMag = 1.0f / mag;
    g_MotionVec[0] = dx * invMag;
    g_MotionVec[1] = dy * invMag;
    g_MotionVec[2] = dz * invMag;
    g_MotionVec[3] = 0.0f;

    const float scale = mag * 0.002f;
    g_MotionBlurAmount = (scale > 0.5f) ? 0.5f : scale;
}

static void __cdecl RenderDispatchHook(void* arg0, void* arg4)
{
    const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
    if (gameFlow < 3 || g_RenderTargetManager.g_DeviceResetInProgress)
    {
        g_RenderDispatchOriginal(arg0, arg4);
        return;
    }

    g_RenderDispatchOriginal(arg0, arg4);

    if (!g_RenderTargetManager.g_DeviceResetInProgress)
    {
        IDirect3DDevice9* dev = g_Device;
        if (dev)
        {
            if (!g_ExposureInitialized)
            {
                g_ExposureInitialized = g_ExposureSampler.Init(dev);
            }

            if (g_ExposureInitialized)
            {
                IDirect3DSurface9* backBuffer = nullptr;
                if (SUCCEEDED(dev->GetRenderTarget(0, &backBuffer)) && backBuffer)
                {
                    g_ExposureSampler.SampleLuminance(backBuffer);
                    backBuffer->Release();
                }

                const DWORD nowTick = GetTickCount();
                float dt = 1.0f / 60.0f;
                if (g_ExposureLastTick != 0)
                    dt = (nowTick - g_ExposureLastTick) / 1000.0f;
                g_ExposureLastTick = nowTick;

                g_ExposureSampler.UpdateExposure(dt, 0.0f);

                auto it = g_RenderTargetManager.g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
                if (it != g_RenderTargetManager.g_ActiveEffects.end() && IsValidEffectObject(it->second))
                {
                    ID3DXEffect* fx = it->second;
                    D3DXHANDLE hExposure = fx->GetParameterByName(nullptr, "Exposure");
                    if (hExposure)
                        fx->SetFloat(hExposure, g_ExposureSampler.GetExposure());
                }
            }
        }
    }

    UpdateMotionBlurFromEView();
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

static inline bool IsValidFx(ID3DXEffect* fx)
{
    if (!fx) return false;
    __try {
        void** vtbl = *(void***)fx;
        return vtbl && vtbl[0] != nullptr;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name = "")
{
    if (!IsValidFx(fx))
        return;

    if (g_RenderTargetManager.g_DeviceResetInProgress)
        return;

    // We want VT to sample the *scene* from your CopyScene() output (TexA),
    // NOT the already-graded/non-VT image and NOT a random RT from SetRT hook.
    IDirect3DTexture9* sceneTex = g_RenderTargetManager.g_LastSceneFullTex;
    if (!sceneTex)
        sceneTex = g_RenderTargetManager.g_SceneCopyTex;

    // Blur/hist input must be a different texture than scene
    IDirect3DTexture9* blurTex = (g_MotionBlurHistoryReady ? g_RenderTargetManager.g_CurrentBlurTex : nullptr);
    if (blurTex == sceneTex)
        blurTex = nullptr;

    // --- locate params (names/semantics you actually use) ---
    D3DXHANDLE hDiffuse = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
    if (!hDiffuse) hDiffuse = fx->GetParameterByName(nullptr, "DiffuseMap");
    if (!hDiffuse) hDiffuse = fx->GetParameterBySemantic(nullptr, "DiffuseMap");

    D3DXHANDLE hBlur = fx->GetParameterByName(nullptr, "MOTIONBLUR_TEXTURE");
    if (!hBlur) hBlur = fx->GetParameterByName(nullptr, "MOTIONBLUR");
    if (!hBlur) hBlur = fx->GetParameterBySemantic(nullptr, "MOTIONBLUR");
    if (!hBlur) hBlur = fx->GetParameterBySemantic(nullptr, "PREV");

    D3DXHANDLE hAmt = fx->GetParameterByName(nullptr, "XNFS_MotionBlurAmount");
    if (!hAmt) hAmt = fx->GetParameterByName(nullptr, "MotionBlurAmount"); // optional fallback
    D3DXHANDLE hBlurVector = fx->GetParameterByName(nullptr, "BLUR_VECTOR");
    if (!hBlurVector) hBlurVector = fx->GetParameterByName(nullptr, "MotionBlurVector");
    D3DXHANDLE hBlurParams = fx->GetParameterByName(nullptr, "IS_MOTIONBLUR_VIGNETTED");
    if (!hBlurParams) hBlurParams = fx->GetParameterByName(nullptr, "MotionBlurParams");

    D3DXHANDLE hTexelSize = fx->GetParameterByName(nullptr, "XNFS_TexelSize");
    D3DXHANDLE hBlurDir = fx->GetParameterByName(nullptr, "XNFS_BlurDir");
    D3DXHANDLE hBlurScale = fx->GetParameterByName(nullptr, "XNFS_BlurScale");

    // --- bind scene ---
    if (hDiffuse && sceneTex)
        fx->SetTexture(hDiffuse, sceneTex);

    // --- bind blur + amount ---
    if (!blurTex || !hBlur)
    {
        if (hBlur) fx->SetTexture(hBlur, nullptr);
        if (hAmt)  fx->SetFloat(hAmt, 0.0f);
        g_MotionBlurAmount = 0.0f;
    }
    else
    {
        fx->SetTexture(hBlur, blurTex);

        // if you want to drive it globally, keep g_MotionBlurAmount authoritative:
        if (hAmt) fx->SetFloat(hAmt, g_MotionBlurAmount);
    }

    if (hBlurVector)
        fx->SetVector(hBlurVector, reinterpret_cast<const D3DXVECTOR4*>(g_MotionVec));
    if (hBlurParams)
    {
        const D3DXVECTOR4 params(g_MotionBlurAmount, 1.0f, 0.0f, 0.0f);
        fx->SetVector(hBlurParams, &params);
    }

    if (hTexelSize)
    {
        UINT w = g_LastBackBufferW ? g_LastBackBufferW : 1;
        UINT h = g_LastBackBufferH ? g_LastBackBufferH : 1;
        if (sceneTex)
        {
            D3DSURFACE_DESC desc{};
            if (SUCCEEDED(sceneTex->GetLevelDesc(0, &desc)) && desc.Width > 0 && desc.Height > 0)
            {
                w = desc.Width;
                h = desc.Height;
            }
        }
        const D3DXVECTOR4 texel(1.0f / (float)w, 1.0f / (float)h, 0.0f, 0.0f);
        fx->SetVector(hTexelSize, &texel);
    }

    if (hBlurDir || hBlurScale)
    {
        const float mvx = g_MotionVec[0];
        const float mvy = g_MotionVec[1];
        const float len = std::sqrt(mvx * mvx + mvy * mvy);
        const float invLen = (len > 0.0001f) ? (1.0f / len) : 0.0f;
        const bool enableBlur = (g_MotionBlurAmount > 0.0f);
        if (hBlurDir)
        {
            const float dirX = enableBlur ? (mvx * invLen) : 0.0f;
            const float dirY = enableBlur ? (mvy * invLen) : 0.0f;
            const D3DXVECTOR4 dir(dirX, dirY, 0.0f, 0.0f);
            fx->SetVector(hBlurDir, &dir);
        }
        if (hBlurScale)
            fx->SetFloat(hBlurScale, enableBlur ? len : 0.0f);
    }

    // Only bind the extra MISCMAP/DEPTH stuff for *non-VT* effects.
    // VT already gets these from the game; stomping them is how you "see non-VT".
    const bool isVT = (name == "IDI_VISUALTREATMENT_FX");
    if (!isVT)
    {
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP1_TEXTURE")) fx->SetTexture(h, g_RenderTargetManager.g_ExposureTex);
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP2_TEXTURE")) fx->SetTexture(h, g_RenderTargetManager.g_VignetteTex);
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP3_TEXTURE")) fx->SetTexture(h, g_RenderTargetManager.g_BloomLUTTex);
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP4_TEXTURE")) fx->SetTexture(h, g_RenderTargetManager.g_DofTex);
        if (auto h = fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE")) fx->SetTexture(h, g_RenderTargetManager.g_DepthTex);
    }
    else
    {
        if (auto h = fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE"))
        {
            IDirect3DBaseTexture9* current = nullptr;
            fx->GetTexture(h, &current);
            if (!current && g_RenderTargetManager.g_DepthTex)
                fx->SetTexture(h, g_RenderTargetManager.g_DepthTex);
            if (current)
                current->Release();
        }
    }

    // Commit is fine (and cheap) - but don't spam logs every call
    fx->CommitChanges();

    static DWORD s_lastBindLogTick = 0;
    const DWORD nowTick = GetTickCount();
    if (nowTick - s_lastBindLogTick >= 1000)
    {
        s_lastBindLogTick = nowTick;
        printf_s("[VTBind] scene=%p blur=%p amt=%.3f\n",
                 sceneTex, blurTex, g_MotionBlurAmount);
    }
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
                HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
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
    if (isVisualFx)
    {
        if (outEffect && *outEffect && IsValidEffectObject(*outEffect))
        {
            auto& existingFx = g_RenderTargetManager.g_ActiveEffects[pResource];
            if (existingFx != *outEffect)
            {
                g_RenderTargetManager.g_ActiveEffects[pResource] = *outEffect;
                g_RenderTargetManager.g_LastReloadedFx = *outEffect;
                printf_s("[XNFS] ✅ Shader created and tracked (effect): %s (%p)\n",
                         pResource, (void*)*outEffect);
            }
        }
        return hr;
    }

    if (isOverbrightFx)
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
    static ID3DXEffect* lastFx = nullptr;
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
            ID3DXEffect* fallbackFx = nullptr;
            auto it = g_RenderTargetManager.g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
            if (it != g_RenderTargetManager.g_ActiveEffects.end() && IsValidEffectObject(it->second))
                fallbackFx = it->second;

            if (fx && !IsValidEffectObject(fx))
            {
                printf_s("[XNFS] VT slot invalid: %p\n", fx);
                fx = fallbackFx;
            }
            else if (!fx)
            {
                fx = fallbackFx;
            }

            if (fx && fx != lastFx)
            {
                // VT effect is owned by the game; do not AddRef/Release.
                g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"] = fx;
                g_RenderTargetManager.g_LastReloadedFx = fx;
                lastFx = fx;
                printf_s("[XNFS] VT active effect: %p\n", fx);
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
        if (IsValidEffectObject(*outEffect))
        {
            auto& existingFx = g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"];
            if (existingFx != *outEffect)
            {
                g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"] = *outEffect;
                g_RenderTargetManager.g_LastReloadedFx = *outEffect;
                printf_s("[XNFS] ✅ Shader created and tracked (effect): IDI_VISUALTREATMENT_FX (%p)\n",
                         (void*)*outEffect);
            }
        }
        return hr;
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

    HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
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

    if (g_ExposureInitialized)
    {
        g_ExposureSampler.Shutdown();
        g_ExposureInitialized = false;
    }

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
    g_RenderTargetManager.OnDeviceLost(); // Delegate to RenderTargetManager's cleanup
    if (g_ExposureInitialized)
    {
        g_ExposureSampler.Shutdown();
        g_ExposureInitialized = false;
    }
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
        OnDeviceLost();
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

    // device->EvictManagedResources();  // don't call - crashes in MW

    HRESULT hr = g_RenderTargetManager.oReset(device, params);

    if (SUCCEEDED(hr))
        OnDeviceReset(device);
    else
        printf_s("[XNFS] oReset failed: HRESULT = 0x%08X\n", hr);

    g_RenderTargetManager.g_DeviceResetInProgress = false;
    return hr;
}

HRESULT WINAPI hkEndScene(IDirect3DDevice9* device)
{
    if (!device) return oEndScene(device);

    if (!g_Device) SetGameDevice(device);

    return oEndScene(device);
}

HRESULT WINAPI hkPresent(IDirect3DDevice9* dev,
                         const RECT* src, const RECT* dst,
                         HWND wnd, const RGNDATA* dirty)
{
    if (!dev)
        return oPresent(dev, src, dst, wnd, dirty);

    if (!g_Device)
        SetGameDevice(dev);

    // Make sure g_BackBufferSurface is valid right now
    // (hkSetRenderTarget usually sets it, but don't rely on it 100%)
    if (!g_RenderTargetManager.g_BackBufferSurface)
    {
        IDirect3DSurface9* backBuffer = nullptr;
        if (SUCCEEDED(dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer)) && backBuffer)
        {
            SAFE_RELEASE(g_RenderTargetManager.g_BackBufferSurface);
            g_RenderTargetManager.g_BackBufferSurface = backBuffer; // keep ref
        }
        else
        {
            SAFE_RELEASE(backBuffer);
        }
    }

    if (!g_RenderTargetManager.g_DeviceResetInProgress)
    {
        // blur is driven from RenderDispatchHook
    }

    return oPresent(dev, src, dst, wnd, dirty);
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
                        g_RenderTargetManager.g_LastSceneFullFrame = eFrameCounter;
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

                        static uint32_t lastRebindFrame = 0;
                        if (lastRebindFrame != eFrameCounter)
                        {
                            lastRebindFrame = eFrameCounter;
                            if (g_RenderTargetManager.g_LastReloadedFx)
                            {
                                ReloadBlurBindings(g_RenderTargetManager.g_LastReloadedFx,
                                                   "IDI_VISUALTREATMENT_FX");
                            }
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
    
    if (!oPresent)
    {
        DWORD oldProtect;
        oPresent = reinterpret_cast<PresentFn>(vtable[17]);
        VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[17] = reinterpret_cast<void*>(&hkPresent);
        VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[XNFS] ? hkPresent installed\n");
    }

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

    if (!g_RenderTargetManager.oSetTransform)
    {
        DWORD oldProtect;
        g_RenderTargetManager.oSetTransform =
            reinterpret_cast<RenderTargetManager::SetTransform_t>(vtable[44]);
        VirtualProtect(&vtable[44], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[44] = reinterpret_cast<void*>(&hkSetTransform);
        VirtualProtect(&vtable[44], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[XNFS] ? hkSetTransform installed via HookedPresent\n");
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
        InstallSub6D3B80BlurHook();
        InstallMotionVectorHook();
        InstallMotionVectorVtablePatch();
        InstallMotionVectorHook4467A0();

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
