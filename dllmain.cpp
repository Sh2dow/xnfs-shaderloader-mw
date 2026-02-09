// dllmain.cpp - Backbuffer hook for NFS MW (2005)
#include <windows.h>
#include <filesystem>
#include <atomic>
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

// Timestamp of last motion vector/amount update from any hook.
static DWORD g_LastMotionUpdateTick = 0;

// Captured from the live VisualTreatment effect right before it draws.
// This is the most authoritative "vanilla" motion blur vector/amount source on MW PC.
static float g_VtCapturedBlurVec[4] = {};
static float g_VtCapturedBlurAmount = 0.0f;
static DWORD g_VtCapturedTick = 0;
static ID3DXEffect* g_LastVtFxObserved = nullptr; // last known good VT effect pointer

// Track the render target the game's DoMotionBlur pass is writing into on the current frame.
// If we accidentally treat this as "scene color", all our sources become post-blur and appear blurry even in dbg_curr.
static IDirect3DSurface9* g_BlurDstSurfaceThisFrame = nullptr;
static uint32_t g_BlurDstFrame = 0;

// Forward decl used by early helpers.
static bool IsProbablyCOMObject(void* obj);

static inline void SafeReleaseSurfaceTracked(IDirect3DSurface9*& s)
{
    // Shader reload / device lost can leave stale pointers. Never call Release on garbage.
    if (!s)
        return;
    if (!IsProbablyCOMObject((void*)s) || IsBadReadPtr(*(void**)s, sizeof(void*)))
    {
        s = nullptr;
        return;
    }
    s->Release();
    s = nullptr;
}

// Pending shader swap request from our internal hot-reload thread.
static std::atomic<bool> g_PendingVisualTreatmentRecompile{false};

// IVisualTreatment_Reset block
void* g_LastEView = nullptr;

void (__fastcall*ApplyGraphicsSettingsOriginal)(void* ecx, void* edx, void* arg1) = nullptr;
int (__thiscall*ApplyGraphicsManagerMainOriginal)(void* thisptr) = nullptr;
bool g_WaitingForReset = false;
static bool g_HasShaderLoader = false;

// Game-side blur dispatch gates (IDA: 0x006DF1CC..0x006DF1DC)
static constexpr uintptr_t kAddr_DoMotionBlurJz = 0x006DF1D2; // jz loc_6DF1E1
static constexpr uintptr_t kAddr_DoMotionBlurGate2 = 0x008F9218; // cmp [8F9218],0 ; jz skip
static constexpr uintptr_t kAddr_MotionBlurEnable = 0x009017DC; // DWORD g_MotionBlurEnable

static inline void EnsureDoMotionBlurDispatchEnabled()
{
    // This keeps the engine reaching sub_6DBB20 (and thus our sub_6D3B80 callsite hooks).
    // NOTE: 0x006DF1D2=0x74 restores a *conditional* jump; the call still depends on the gates being nonzero.
    injector::WriteMemory<DWORD>(kAddr_MotionBlurEnable, 1, true);
    injector::WriteMemory<DWORD>(kAddr_DoMotionBlurGate2, 1, true);
    injector::WriteMemory<BYTE>(kAddr_DoMotionBlurJz, 0x74, true);
}

// Forward decls: EnsureDeviceVtableHooks refers to these before their definitions later in this file.
HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params);
HRESULT WINAPI hkPresent(IDirect3DDevice9* dev, const RECT* src, const RECT* dst, HWND wnd, const RGNDATA* dirty);
HRESULT WINAPI hkEndScene(IDirect3DDevice9* device);

static inline bool IsFxReadable(ID3DXEffect* fx)
{
    // Be conservative: ShaderLoader-created effects can trip stricter validators during transition.
    // We only need a pointer that is readable; D3DX methods will still fail safely if it's bogus.
    return fx && !IsBadReadPtr(fx, sizeof(void*));
}

static bool DetectShaderLoader()
{
    // Robust detection: ShaderLoader may be renamed; search loaded module filenames for "shaderloader".
    HMODULE mods[1024]{};
    DWORD needed = 0;
    if (!EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &needed))
        return false;

    const DWORD count = needed / sizeof(HMODULE);
    char path[MAX_PATH]{};
    for (DWORD i = 0; i < count; ++i)
    {
        path[0] = '\0';
        if (!GetModuleFileNameA(mods[i], path, (DWORD)sizeof(path)))
            continue;

        std::string s(path);
        for (char& c : s) c = (char)std::tolower((unsigned char)c);
        if (s.find("shaderloader") != std::string::npos)
            return true;
    }
    return false;
}

static bool IsProbablyCOMObject(void* obj)
{
    if (!obj) return false;
    if (IsBadReadPtr(obj, sizeof(void*))) return false;
    void** vtbl = *(void***)obj;
    if (!vtbl || IsBadReadPtr(vtbl, sizeof(void*))) return false;
    void* fn0 = vtbl[0];
    if (!fn0) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(fn0, &mbi, sizeof(mbi))) return false;
    if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
        return false;
    return true;
}

static ID3DXEffect* FindVtEffectFromObject(void* vtObject)
{
    if (!vtObject || IsBadReadPtr(vtObject, 0x80))
        return nullptr;

    auto SafeGetDescOk = [](ID3DXEffect* e) -> bool
    {
        if (!e || !IsFxReadable(e) || !IsProbablyCOMObject(e))
            return false;
        D3DXEFFECT_DESC d{};
        return SUCCEEDED(e->GetDesc(&d));
    };

    static int s_cachedOff = 0x18C;

    auto TryOffset = [&](int off) -> ID3DXEffect*
    {
        void* slotPtr = (char*)vtObject + off;
        if (IsBadReadPtr(slotPtr, sizeof(void*))) return nullptr;
        void* raw = *reinterpret_cast<void**>(slotPtr);
        if (!raw) return nullptr;

        if (IsProbablyCOMObject(raw))
        {
            auto* cand = reinterpret_cast<ID3DXEffect*>(raw);
            return SafeGetDescOk(cand) ? cand : nullptr;
        }

        if (!IsBadReadPtr(raw, sizeof(void*)))
        {
            void* raw2 = *reinterpret_cast<void**>(raw);
            if (raw2 && IsProbablyCOMObject(raw2))
            {
                auto* cand = reinterpret_cast<ID3DXEffect*>(raw2);
                return SafeGetDescOk(cand) ? cand : nullptr;
            }
        }

        return nullptr;
    };

    if (ID3DXEffect* fx = TryOffset(s_cachedOff))
        return fx;

    for (int off = 0x80; off <= 0x700; off += 4)
    {
        if (ID3DXEffect* fx = TryOffset(off))
        {
            s_cachedOff = off;
            return fx;
        }
    }

    return nullptr;
}

static void DetectShaderLoaderLate()
{
    // Called from Present: if ShaderLoader loads after us, update the flag and stop any
    // future D3DX hook installation attempts (we can't safely unhook already-installed ones).
    static bool s_logged = false;
    if (!g_HasShaderLoader && DetectShaderLoader())
    {
        g_HasShaderLoader = true;
        if (!s_logged)
        {
            s_logged = true;
            printf_s("[Init] ShaderLoader detected late: YES\n");
        }
    }
}

HRESULT WINAPI hkEndScene(IDirect3DDevice9* device);
HRESULT WINAPI hkPresent(IDirect3DDevice9* dev,
                         const RECT* src, const RECT* dst,
                         HWND wnd, const RGNDATA* dirty);
HRESULT WINAPI hkSetRenderTarget(LPDIRECT3DDEVICE9 device, DWORD index, IDirect3DSurface9* renderTarget);
HRESULT WINAPI hkSetDepthStencilSurface(LPDIRECT3DDEVICE9 device, IDirect3DSurface9* pNewZStencil);

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

// Direct address hook: 0x006DBB20 (DoMotionBlur on PC). We use this as a reliable per-frame point
// to disable vanilla fullscreen blur parameters and (optionally) steal the game's blur vector/amount.
//
// IMPORTANT: we patch this via injector::MakeJMP (like X360Stuff does), not MinHook. In this game
// MinHook can miss/lose the hook due to code patches and/or loader interactions.
using DoMotionBlur_t = void(__cdecl*)();
static DoMotionBlur_t g_DoMotionBlur_Original = nullptr; // unused for JMP hook; kept for compatibility

static void __cdecl hkDoMotionBlur()
{
    // We intentionally do NOT call the original here. This hook exists to prevent the vanilla
    // fullscreen blur path from running at all (we render our own blur in sub_6D3B80 hook).

    if (!g_pVisualTreatment || IsBadReadPtr(g_pVisualTreatment, sizeof(void*)))
        return;
    if (!*g_pVisualTreatment)
        return;

    void* vtObject = *g_pVisualTreatment;
    if (!vtObject || IsBadReadPtr(vtObject, 0x80))
        return;

    // Shader reload can change the object layout or swap which slot holds ID3DXEffect*.
    // Scan a small region for a plausible effect pointer. Do NOT validate by technique name here:
    // ShaderLoader reload can temporarily invalidate techniques even when the effect is real.
    auto SafeGetDescOk = [](ID3DXEffect* e) -> bool
    {
        // No SEH here (C2712). Rely on pointer validation + COM vtable sanity.
        if (!e || !IsFxReadable(e) || !IsProbablyCOMObject(e))
            return false;
        D3DXEFFECT_DESC d{};
        return SUCCEEDED(e->GetDesc(&d));
    };

    static int s_cachedFxOffset = 0x18C;
    static DWORD s_lastFindLog = 0;
    ID3DXEffect* fx = nullptr;
    int foundOff = -1;

    auto TryOffset = [&](int off) -> ID3DXEffect*
    {
        if (off < 0) return nullptr;
        if (IsBadReadPtr((char*)vtObject + off, sizeof(void*))) return nullptr;
        void* raw = *reinterpret_cast<void**>((char*)vtObject + off);
        if (!raw) return nullptr;

        // Case 1: slot holds ID3DXEffect*
        if (IsProbablyCOMObject(raw))
        {
            auto* cand = reinterpret_cast<ID3DXEffect*>(raw);
            return SafeGetDescOk(cand) ? cand : nullptr;
        }

        // Case 2: slot holds ID3DXEffect** (common in MW objects)
        if (!IsBadReadPtr(raw, sizeof(void*)))
        {
            void* raw2 = *reinterpret_cast<void**>(raw);
            if (raw2 && IsProbablyCOMObject(raw2))
            {
                auto* cand = reinterpret_cast<ID3DXEffect*>(raw2);
                return SafeGetDescOk(cand) ? cand : nullptr;
            }
        }

        return nullptr;
    };

    // Fast path: cached offset.
    fx = TryOffset(s_cachedFxOffset);
    if (fx) foundOff = s_cachedFxOffset;
    if (!fx)
    {
        // Slow path: scan typical range.
        for (int off = 0x80; off <= 0x700; off += 4)
        {
            fx = TryOffset(off);
            if (fx)
            {
                s_cachedFxOffset = off;
                foundOff = off;
                break;
            }
        }
    }
    {
        const DWORD now = GetTickCount();
        if (now - s_lastFindLog >= 1000)
        {
            s_lastFindLog = now;
            printf_s("[DoMotionBlur] find: vt=%p cachedOff=0x%X foundOff=0x%X fx=%p\n",
                     vtObject, s_cachedFxOffset, foundOff, fx);
        }
    }
    if (!fx)
    {
        // Fallback: VT effect pointer is tracked elsewhere (ShaderLoader / ApplyGraphicsSettings).
        // Use last observed VT effect or the active effects map.
        ID3DXEffect* fallbackFx = g_LastVtFxObserved;
        if (!fallbackFx)
        {
            auto it = g_RenderTargetManager.g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
            if (it != g_RenderTargetManager.g_ActiveEffects.end())
                fallbackFx = it->second;
        }
        if (fallbackFx && IsProbablyCOMObject(fallbackFx) && SafeGetDescOk(fallbackFx))
            fx = fallbackFx;
        else
            return;
    }

    // No SEH here (C2712). We only call methods if pointer looks like a COM object.
    if (!IsFxReadable(fx) || !IsProbablyCOMObject(fx))
        return;

    auto SafeGetParam = [](ID3DXEffect* e, const char* name) -> D3DXHANDLE
    {
        return (e && name) ? e->GetParameterByName(nullptr, name) : nullptr;
    };
    auto SafeGetParamSemantic = [](ID3DXEffect* e, const char* sem) -> D3DXHANDLE
    {
        return (e && sem) ? e->GetParameterBySemantic(nullptr, sem) : nullptr;
    };
    auto SafeGetVector = [](ID3DXEffect* e, D3DXHANDLE h, D3DXVECTOR4* out) -> bool
    {
        return (e && h && out) ? SUCCEEDED(e->GetVector(h, out)) : false;
    };
    auto SafeSetVector = [](ID3DXEffect* e, D3DXHANDLE h, const D3DXVECTOR4* v) -> bool
    {
        return (e && h && v) ? SUCCEEDED(e->SetVector(h, v)) : false;
    };
    auto SafeCommit = [](ID3DXEffect* e) -> void
    {
        if (e) e->CommitChanges();
    };

    // Capture + clear right here (address-level hook).
    D3DXHANDLE hVec = SafeGetParam(fx, "BLUR_VECTOR");
    if (!hVec) hVec = SafeGetParam(fx, "MotionBlurVector");
    if (!hVec) hVec = SafeGetParamSemantic(fx, "BLUR_VECTOR");
    D3DXHANDLE hParams = SafeGetParam(fx, "IS_MOTIONBLUR_VIGNETTED");
    if (!hParams) hParams = SafeGetParam(fx, "MotionBlurParams");
    if (!hParams) hParams = SafeGetParam(fx, "BlurParams");
    if (!hParams) hParams = SafeGetParamSemantic(fx, "IS_MOTIONBLUR_VIGNETTED");

    if (hVec)
    {
        D3DXVECTOR4 v{};
        if (SafeGetVector(fx, hVec, &v))
        {
            g_VtCapturedBlurVec[0] = v.x;
            g_VtCapturedBlurVec[1] = v.y;
            g_VtCapturedBlurVec[2] = v.z;
            g_VtCapturedBlurVec[3] = v.w;
            g_VtCapturedTick = GetTickCount();
        }
    }
    if (hParams)
    {
        D3DXVECTOR4 p{};
        if (SafeGetVector(fx, hParams, &p) && std::isfinite(p.x))
        {
            g_VtCapturedBlurAmount = max(0.0f, min(0.65f, p.x));
            g_VtCapturedTick = GetTickCount();
        }
    }

    const D3DXVECTOR4 zero(0, 0, 0, 0);
    if (hVec) SafeSetVector(fx, hVec, &zero);
    if (hParams) SafeSetVector(fx, hParams, &zero);
    SafeCommit(fx);

    // Intentionally quiet: this hook can be called frequently.
}

static void InstallDoMotionBlurHook()
{
    // Patch a JMP exactly like X360Stuff does.
    injector::MakeJMP(0x006DBB20, hkDoMotionBlur, true);
    printf_s("[Init] ✅ DoMotionBlur JMP installed (0x006DBB20)\n");
}

// -------------------- ID3DXEffect draw-time hook (removed) --------------------
// The BeginPass vtable patch caused large FPS drops and conflicts with other injectors/loaders.
// We do not intercept VT effects; the blur pipeline must work without touching ID3DXEffect internals.
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

static ExposureStandalone::ExposureSampler g_ExposureSampler;
static bool g_ExposureInitialized = false;
static DWORD g_ExposureLastTick = 0;

static bool GetFileWriteTimeUtc(const char* path, FILETIME* out)
{
    if (!path || !out) return false;
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data))
        return false;
    *out = data.ftLastWriteTime;
    return true;
}

static DWORD WINAPI HotReloadThread(LPVOID)
{
    // Wait for a device first.
    while (!g_Device)
        Sleep(50);

    // ShaderLoader owns reload; avoid running a second file watcher in-process.
    if (g_HasShaderLoader)
        return 0;

    FILETIME lastVT{};
    GetFileWriteTimeUtc("fx\\visualtreatment.fx", &lastVT);

    for (;;)
    {
        Sleep(250);

        // Only reload during gameplay, and never during reset.
        const int gameFlow = *reinterpret_cast<int*>(GAMEFLOWSTATUS_ADDR);
        if (gameFlow < 3 || g_RenderTargetManager.g_DeviceResetInProgress)
            continue;

        FILETIME nowVT{};
        if (!GetFileWriteTimeUtc("fx\\visualtreatment.fx", &nowVT))
            continue;

        if (CompareFileTime(&nowVT, &lastVT) <= 0)
            continue;
        lastVT = nowVT;

        // Compile on the game/render thread only. D3DX effect compilation is not reliably thread-safe.
        g_PendingVisualTreatmentRecompile.store(true);
        printf_s("[HotReload] 🔔 visualtreatment.fx changed, pending recompile\n");
    }
}

static void PatchMotionBlurFix()
{
    struct PatchByte
    {
        uintptr_t addr;
        BYTE value;
    };
    const PatchByte patches[] =
    {
        // Enable the blur path variants the engine expects.
        {0x006DBE79, 0x01},
        {0x006DBE7B, 0x02},
        // IMPORTANT:
        // Do NOT patch 0x006DF1D2 here. That patch disables the blur dispatch entirely on some builds,
        // which prevents our sub_6D3B80 callsite hooks from running (result: no blur at all).
        // Keep dispatch alive so our override (callsite hooks) can run.
        // {0x006DF1D2, 0xEB},
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
    printf_s("[XNFS] ✅ Blur enable bytes patched\n");
}

static void PatchVanillaMotionBlurControl()
{
    // Mirror NFSMWMotionBlurController behavior:
    // - 0x006DF1D2: toggles a conditional jump in DoMotionBlur (0x74 enable / 0xEB disable).
    // - 0x006DBD46/56/66/74: pointer operands for MotionBlurAmount reads inside sub_6DBB20.
    //
    // The engine reads MotionBlurAmount via hardcoded pointer operands inside DoMotionBlur.
    //
    // IMPORTANT:
    // Keep MotionBlurAmount live (pointing at g_MotionBlurAmount).
    static bool s_patchedPtrs = false;
    if (!s_patchedPtrs)
    {
        // These sites store a pointer to float (NOT a pointer-to-pointer).
        const uintptr_t amtPtr = reinterpret_cast<uintptr_t>(&g_MotionBlurAmount);
        injector::WriteMemory<uintptr_t>(0x006DBD46, amtPtr, true);
        injector::WriteMemory<uintptr_t>(0x006DBD56, amtPtr, true);
        injector::WriteMemory<uintptr_t>(0x006DBD66, amtPtr, true);
        injector::WriteMemory<uintptr_t>(0x006DBD74, amtPtr, true);
        s_patchedPtrs = true;
        printf_s("[XNFS] ✅ Patched vanilla MotionBlurAmount pointers to g_MotionBlurAmount (%p)\n", (void*)amtPtr);
    }

    // Hard-disable the game's own fullscreen motion blur behavior.
    // User requirement: no vanilla blur for sure.
    //
    // IMPORTANT (per your finding): writing 0 to 0x009017DC (DWORD) disables the blur path entirely,
    // which prevents our 0x006DBE8B/0x006DBEB0 callsite hooks from running.
    // So we KEEP the blur path enabled, and we kill vanilla blur visually by:
    // - overwriting its output with our custom pass (XNFS_Sub6D3B80_Hook)
    //
    // Other mods/shader reloads can restore bytes/vars, so we re-assert periodically.
    // g_MotionBlurEnable is a DWORD (dd 1), not a BYTE.
    EnsureDoMotionBlurDispatchEnabled();
    injector::WriteMemory<float>(0x008F9B10, 25.0f, true); // MotionBlurMinEffectiveSpeed = 25.0
    // Do not force MotionBlurAmount here; it is driven by our motion hooks / captured VT params.

    // No periodic logging here; this runs during gameplay and logging destroys FPS.
}

static void LogDoMotionBlurDispatchStateOccasionally()
{
    static DWORD s_last = 0;
    const DWORD now = GetTickCount();
    if (now - s_last < 1000)
        return;
    s_last = now;

    const BYTE jcc = *reinterpret_cast<BYTE*>(kAddr_DoMotionBlurJz);
    const DWORD gate1 = *reinterpret_cast<DWORD*>(kAddr_MotionBlurEnable);
    const DWORD gate2 = *reinterpret_cast<DWORD*>(kAddr_DoMotionBlurGate2);
    // No printf spam unless you want it; keep it minimal.
    printf_s("[XNFS] ? DoMB gates: 6DF1D2=0x%02X g_MotionBlurEnable=%u gate2(8F9218)=%u\n", jcc, gate1, gate2);
}


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
            // Interpret v[0..2] as camera position and derive per-frame delta.
            static float s_lastPos[3] = {};
            static bool s_hasLastPos = false;
            if (!s_hasLastPos)
            {
                s_lastPos[0] = v[0];
                s_lastPos[1] = v[1];
                s_lastPos[2] = v[2];
                s_hasLastPos = true;
                g_MotionVec[0] = g_MotionVec[1] = g_MotionVec[2] = 0.0f;
            }
            else
            {
                g_MotionVec[0] = v[0] - s_lastPos[0];
                g_MotionVec[1] = v[1] - s_lastPos[1];
                g_MotionVec[2] = v[2] - s_lastPos[2];
                s_lastPos[0] = v[0];
                s_lastPos[1] = v[1];
                s_lastPos[2] = v[2];
            }
            g_MotionVec[3] = 0.0f;

            // Drive a usable blur amount from the vector magnitude (fallback path).
            const float mag = std::sqrt(g_MotionVec[0] * g_MotionVec[0] +
                g_MotionVec[1] * g_MotionVec[1] +
                g_MotionVec[2] * g_MotionVec[2]);
            g_MotionBlurAmount = (mag > 0.0f) ? min(0.65f, mag * 0.02f) : 0.0f;
            g_LastMotionUpdateTick = GetTickCount();
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

    // This hook is guaranteed to run when the blur path is active. Use it as the authoritative
    // place to apply vanilla blur control patches (instead of hkEndScene which may not run reliably
    // across all hook chains / ShaderLoader reset loops).
    PatchVanillaMotionBlurControl();

    IDirect3DDevice9* dev = g_Device;
    if (!dev)
        return 0;

    // Ensure our RTs exist. Some mod stacks don't hit our reset/present init paths early enough,
    // but the game can still enter the blur path. Without targets, we'll never publish g_CurrentBlurTex.
    {
        static bool s_inInit = false;
        if (!s_inInit &&
            (!g_RenderTargetManager.g_BlurHistoryTexA || !g_RenderTargetManager.g_BlurHistorySurfA ||
                !g_RenderTargetManager.g_BlurHistoryTexB || !g_RenderTargetManager.g_BlurHistorySurfB))
        {
            s_inInit = true;
            const bool ok = g_RenderTargetManager.OnDeviceReset(dev);
            printf_s("[XNFS] ? Lazy OnDeviceReset from blur hook: %d\n", ok ? 1 : 0);
            s_inInit = false;
        }
    }

    // Do not compute motion from EViewCurrent here. On this build it frequently produces a non-invertible matrix,
    // and in general this hook is too late in the frame to be the authoritative source anyway.
    // We capture the game's own blur vector/amount from IDI_VISUALTREATMENT_FX at draw-time instead.

    IDirect3DSurface9* dstRT = nullptr;
    IDirect3DBaseTexture9* src0 = nullptr;
    IDirect3DBaseTexture9* src1 = nullptr;
    bool gotStage0 = false;
    bool gotStage1 = false;

    dev->GetRenderTarget(0, &dstRT);
    // Remember the RT used for the blur pass this frame so hkSetRenderTarget can ignore it as a scene candidate.
    if (dstRT)
    {
        if (g_BlurDstFrame != eFrameCounter)
        {
            SafeReleaseSurfaceTracked(g_BlurDstSurfaceThisFrame);
            g_BlurDstFrame = eFrameCounter;
        }
        if (!g_BlurDstSurfaceThisFrame)
        {
            g_BlurDstSurfaceThisFrame = dstRT;
            g_BlurDstSurfaceThisFrame->AddRef();
        }
    }
    if (SUCCEEDED(dev->GetTexture(0, &src0)) && src0) gotStage0 = true;
    if (SUCCEEDED(dev->GetTexture(1, &src1)) && src1) gotStage1 = true;

    // Never trust whatever the engine currently has bound at stage 1 here; it is often not the
    // motion-blur history texture and leads to garbage sampling. Always use our tracked history.
    if (src1)
    {
        src1->Release();
        src1 = nullptr;
        gotStage1 = false;
    }

    bool addedSrc0Ref = false;
    bool addedSrc1Ref = false;

    auto getTex2D = [](IDirect3DBaseTexture9* base) -> IDirect3DTexture9*
    {
        if (!base) return nullptr;
        IDirect3DTexture9* t = nullptr;
        if (SUCCEEDED(base->QueryInterface(__uuidof(IDirect3DTexture9), (void**)&t)) && t)
            return t;
        return nullptr;
    };
    auto matchesDst = [&](IDirect3DBaseTexture9* base) -> bool
    {
        if (!base || !dstRT) return false;
        IDirect3DTexture9* t = getTex2D(base);
        if (!t) return false;
        D3DSURFACE_DESC td{};
        IDirect3DSurface9* lvl = nullptr;
        bool ok = false;
        if (SUCCEEDED(t->GetSurfaceLevel(0, &lvl)) && lvl && SUCCEEDED(lvl->GetDesc(&td)))
        {
            D3DSURFACE_DESC dd{};
            if (SUCCEEDED(dstRT->GetDesc(&dd)))
                ok = (td.Width == dd.Width && td.Height == dd.Height);
        }
        SAFE_RELEASE(lvl);
        t->Release();
        return ok;
    };

    // Source priority:
    // 1) Engine-provided pointer (a3) from sub_6DBB20 -> sub_6D3B80 callsites. This is the most reliable
    //    "scene color" the game intends for the blur pass. It can be higher-res than the dst RT (supersampling).
    // 2) Our pre-VT capture (only if updated this frame).
    // 3) Engine stage0 / last-scene fallbacks.
    //
    // IMPORTANT: prefer (a3) even if stage0 is bound. In many mod stacks stage0 can be post-blur or otherwise
    // not the intended source, which makes even dbg_curr look blurry.
    if (a3)
    {
        IDirect3DBaseTexture9* cand = nullptr;
        if (!IsBadReadPtr(a3, sizeof(void*)))
        {
            cand = *reinterpret_cast<IDirect3DBaseTexture9**>(a3);
            if (!IsProbablyCOMObject(cand))
                cand = nullptr;
        }
        if (!cand && IsProbablyCOMObject(a3))
            cand = reinterpret_cast<IDirect3DBaseTexture9*>(a3);
        if (cand)
        {
            // Sanity-check desc (must be a 2D texture with plausible size).
            IDirect3DTexture9* t = getTex2D(cand);
            D3DSURFACE_DESC td{};
            if (t && SUCCEEDED(t->GetLevelDesc(0, &td)) && td.Width >= 64 && td.Height >= 64)
            {
                if (src0) src0->Release();
                src0 = cand;
                src0->AddRef();
                addedSrc0Ref = true;
                gotStage0 = false; // (a3) overrides any engine-bound stage0 assumptions
            }
            if (t) t->Release();
        }
    }
    if (!src0 && g_RenderTargetManager.g_SceneColorTex && g_RenderTargetManager.g_SceneColorFrame == eFrameCounter)
    {
        src0 = g_RenderTargetManager.g_SceneColorTex;
        src0->AddRef();
        addedSrc0Ref = true;
    }

    // Validate the engine-bound stage0 texture; if it doesn't match the destination size, drop it.
    if (src0 && gotStage0 && !matchesDst(src0))
    {
        src0->Release();
        src0 = nullptr;
        gotStage0 = false;
    }

    if (!src0 && g_RenderTargetManager.g_LastSceneFullTex)
    {
        src0 = g_RenderTargetManager.g_LastSceneFullTex;
        src0->AddRef();
        addedSrc0Ref = true;
    }

    // (a3 handling moved into priority block above)

    if (g_MotionBlurHistoryReady && g_RenderTargetManager.g_CurrentBlurTex)
    {
        src1 = g_RenderTargetManager.g_CurrentBlurTex;
        src1->AddRef();
        addedSrc1Ref = true;
    }

    // If we still have no src0, try a safe fallback to SceneCopyTex (post-VT).
    // This is last resort; it can contain UI, but avoids "no blur at all".
    if (!src0 && g_RenderTargetManager.g_SceneCopyTex)
    {
        src0 = g_RenderTargetManager.g_SceneCopyTex;
        src0->AddRef();
        addedSrc0Ref = true;
    }

    // No periodic logging here; this is hot-path code and logging destroys FPS.

    if (dstRT && src0)
    {
        // No periodic logging here; this is hot-path code and logging destroys FPS.

        // Amount source:
        // 1) Prefer VT-derived params only if some other path populated them (no effect hooks here).
        // 2) Fall back to whatever our motion hooks produced.
        float amountForPass = 0.0f;

        {
            const DWORD now = GetTickCount();
            const DWORD vtAge = now - g_VtCapturedTick;
            if (vtAge < 1000 && g_VtCapturedBlurAmount > 0.0f)
            {
                amountForPass = g_VtCapturedBlurAmount;
                // Also use the captured vector for directional sampling.
                g_MotionVec[0] = g_VtCapturedBlurVec[0];
                g_MotionVec[1] = g_VtCapturedBlurVec[1];
                g_MotionVec[2] = g_VtCapturedBlurVec[2];
                g_MotionVec[3] = g_VtCapturedBlurVec[3];
                g_MotionBlurAmount = amountForPass;
                g_LastMotionUpdateTick = now;
            }
        }

        if (amountForPass <= 0.000001f)
            amountForPass = g_MotionBlurAmount;

        // Small heuristic: if we had any motion update recently but amount is stuck at 0, use a tiny minimum.
        // Do NOT force a default nonzero amount; that creates a constant fullscreen haze at rest.
        if (amountForPass <= 0.000001f)
        {
            const DWORD ageMs = GetTickCount() - g_LastMotionUpdateTick;
            if (ageMs < 100)
                amountForPass = 0.06f;
        }

        // Allow forcing amount to validate the pipeline (F11 hold).
        // F9 is used for debug-cycle in MotionBlurPass in some setups.
        if (GetAsyncKeyState(VK_F11) & 0x8000)
            amountForPass = 0.08f;

        // Keep globals in sync so ReloadBlurBindings (VT samplers/params) sees the same amount.
        // Without this, CompositeToSurface can run with a forced amount but VT may still blend with 0.
        g_MotionBlurAmount = amountForPass;
        g_LastMotionUpdateTick = GetTickCount();

        // Vanilla motion blur is hard-disabled in PatchVanillaMotionBlurControl().

        // Update blur history (ping-pong). VisualTreatment will apply the blur using its own mask/vignette
        // as long as we bind DIFFUSEMAP_TEXTURE=scene and MOTIONBLUR/PREV texture=our history.
        MotionBlurPass::RenderBlurOverride(dev, src0, src1, dstRT);

        // Debug only: draw our composite/debug views into the engine blur target.
        // Default behavior must NOT composite here, otherwise blur becomes effectively fullscreen and ignores VT.
        //
        // F9: cycle debug modes (0..7)
        // F10 (hold): force magenta tint (proves our draw is visible)
        static int s_dbgMode = 0;
        static int s_dbgShowFrames = 0;
        static SHORT s_prevF9 = 0;
        const SHORT curF9 = GetAsyncKeyState(VK_F9);
        if ((curF9 & 0x8000) && !(s_prevF9 & 0x8000))
        {
            s_dbgMode = (s_dbgMode + 1) % 8;
            s_dbgShowFrames = 180; // ~3s at 60fps, enough to see the mode without holding keys
        }
        s_prevF9 = curF9;

        const bool forceTint = (GetAsyncKeyState(VK_F10) & 0x8000) != 0;
        MotionBlurPass::SetDebugMode(s_dbgMode);
        MotionBlurPass::SetForceTint(forceTint);

        const bool wantDebugView = forceTint || (s_dbgMode != 0 && s_dbgShowFrames > 0);
        if (wantDebugView && g_RenderTargetManager.g_CurrentBlurTex)
        {
            MotionBlurPass::CompositeToSurface(dev, src0, g_RenderTargetManager.g_CurrentBlurTex, dstRT, amountForPass);
            if (s_dbgShowFrames > 0) --s_dbgShowFrames;
        }

        static DWORD s_lastBlurLogTick = 0;
        const DWORD nowTick = GetTickCount();
        if (nowTick - s_lastBlurLogTick >= 1000)
        {
            s_lastBlurLogTick = nowTick;
            // No periodic logging here; this is hot-path code and logging destroys FPS.
        }
    }

    if (src1) src1->Release();
    if (src0) src0->Release();
    if (dstRT) dstRT->Release();

    // Always swallow the engine blur pass for these variants so vanilla can't overwrite our history.
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

    // Fallback blur amount driver: when EView-based extraction is unavailable/unstable,
    // use camera delta magnitude to at least produce a coherent nonzero amount.
    {
        const float mvx = g_MotionVec[0];
        const float mvy = g_MotionVec[1];
        const float mvz = g_MotionVec[2];
        const float mag = std::sqrt(mvx * mvx + mvy * mvy + mvz * mvz);
        // Heuristic scaling. Tune as needed once we see stable magnitudes in logs.
        const float newAmt = (mag > 0.0f) ? min(0.65f, mag * 0.02f) : 0.0f;
        // Do not stomp a previously-valid amount with 0.0f. Several builds spam this hook with
        // near-zero deltas, which would otherwise permanently kill blur.
        if (newAmt > 0.0001f)
            g_MotionBlurAmount = newAmt;
    }

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
        // Fallback motion source: view-matrix delta. This is not as accurate as engine motion vectors,
        // but it's reliable across builds and avoids depending on game internals that may not be called.
        static bool s_hasLast = false;
        static D3DXMATRIX s_last{};

        g_LastViewMatrix = *reinterpret_cast<const D3DXMATRIX*>(matrix);
        g_HasLastViewMatrix = true;

        if (s_hasLast)
        {
            // View matrix translation can be near-stationary or not directly represent camera motion.
            // Use a cheap full-matrix delta heuristic so "moving/turning camera" produces a nonzero amount.
            const float* a = (const float*)&g_LastViewMatrix;
            const float* b = (const float*)&s_last;
            float sumAbs = 0.0f;
            for (int i = 0; i < 16; ++i)
                sumAbs += std::fabs(a[i] - b[i]);

            const float dx = g_LastViewMatrix._41 - s_last._41;
            const float dy = g_LastViewMatrix._42 - s_last._42;
            const float dz = g_LastViewMatrix._43 - s_last._43;
            const float transMag = std::sqrt(dx * dx + dy * dy + dz * dz);

            const float mag = sumAbs + transMag;
            if (std::isfinite(mag))
            {
                // Direction in "screen-ish" space; only used as a blur direction hint.
                g_MotionVec[0] = dx;
                g_MotionVec[1] = dy;
                g_MotionVec[2] = dz;
                g_MotionVec[3] = 0.0f;

                // Heuristic scaling: matrix deltas are tiny, so scale aggressively.
                // In MW, the view matrix deltas can be extremely small. We only need a stable,
                // nonzero scalar to drive our blend; so use a very high gain.
                const float newAmt = min(0.65f, mag * 5000.0f);
                if (newAmt > 0.0001f)
                    g_MotionBlurAmount = newAmt;
            }
        }

        s_last = g_LastViewMatrix;
        s_hasLast = true;

        // Mark that we saw a VIEW set this frame.
        g_LastMotionUpdateTick = GetTickCount();

        static DWORD s_lastLogTick = 0;
        const DWORD nowTick = GetTickCount();
        if (nowTick - s_lastLogTick >= 1000)
        {
            s_lastLogTick = nowTick;
            printf_s("[MotionVec] hkSetTransform VIEW amount=%.6f\n", g_MotionBlurAmount);
        }
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
    g_RenderTargetManager.g_SceneCopyTex = g_RenderTargetManager.g_SceneCopyTex; // (no-op, just clarity)
    g_RenderTargetManager.g_LastSceneFullFrame = eFrameCounter;
}

static void CaptureSceneColor(IDirect3DDevice9* device)
{
    if (!device) return;
    if (!g_RenderTargetManager.g_LastSceneFullSurface || !g_RenderTargetManager.g_SceneColorSurface)
        return;
    static uint32_t s_lastCopyFrame = 0;
    if (s_lastCopyFrame == eFrameCounter)
        return;

    // Guard against re-entrancy: StretchRect can trigger internal state changes that call SetRenderTarget.
    static thread_local bool s_inCapture = false;
    if (s_inCapture)
        return;
    s_inCapture = true;

    const HRESULT hr = device->StretchRect(
        g_RenderTargetManager.g_LastSceneFullSurface,
        nullptr,
        g_RenderTargetManager.g_SceneColorSurface,
        nullptr,
        D3DTEXF_NONE
    );
    s_inCapture = false;
    if (FAILED(hr))
        return;
    s_lastCopyFrame = eFrameCounter;
    g_RenderTargetManager.g_SceneColorFrame = eFrameCounter;
}

static void UpdateMotionBlurFromEView()
{
    // PC MW: EViewCurrent pointer (WeatherMod / game global). This is more reliable than the X360Stuff port address.
    // Note: DXVK builds still keep this global; it is game-side, not D3D-side.
    static void** eViewCurrentPtr = reinterpret_cast<void**>(0x009196B4); // eViews_0 / EViewCurrent
    void* eView = (eViewCurrentPtr && !IsBadReadPtr(eViewCurrentPtr, sizeof(void*))) ? *eViewCurrentPtr : nullptr;

    // Fallback to the old X360Stuff-derived pointer if the PC global isn't valid for some reason.
    if (!eView || reinterpret_cast<uintptr_t>(eView) < 0x10000)
    {
        static void** legacy = reinterpret_cast<void**>(0x00A0D4D8);
        if (legacy && !IsBadReadPtr(legacy, sizeof(void*)) && *legacy && reinterpret_cast<uintptr_t>(*legacy) >=
            0x10000)
            eView = *legacy;
    }

    if (!eView || reinterpret_cast<uintptr_t>(eView) < 0x10000)
        return;

    auto view = static_cast<char*>(eView);
    void* viewData = *reinterpret_cast<void**>(view + 0xB0);
    if (!viewData || IsBadReadPtr(viewData, 0x220))
    {
        return;
    }

    auto vd = static_cast<char*>(viewData);

    // Scale factors (from X360Stuff decomp constants)
    static float* motionBlurScale = reinterpret_cast<float*>(0x1002F180);
    static float* velocityScale = reinterpret_cast<float*>(0x1002A784);
    const float scale = motionBlurScale ? *motionBlurScale : 1.0f;
    const float vscale = velocityScale ? *velocityScale : 1.0f;

    // Position delta and velocity
    const float dx = (*reinterpret_cast<float*>(vd + 0x130) - *reinterpret_cast<float*>(vd + 0x50)) * scale;
    const float dy = (*reinterpret_cast<float*>(vd + 0x134) - *reinterpret_cast<float*>(vd + 0x54)) * scale;
    const float dz = (*reinterpret_cast<float*>(vd + 0x138) - *reinterpret_cast<float*>(vd + 0x58)) * scale;

    const float vx = *reinterpret_cast<float*>(vd + 0x200) * vscale;
    const float vy = *reinterpret_cast<float*>(vd + 0x204) * vscale;
    const float vz = *reinterpret_cast<float*>(vd + 0x208) * vscale;

    float motionVec[4] = {dx + vx, dy + vy, dz + vz, 0.0f};
    const float posMag = std::sqrt(dx * dx + dy * dy + dz * dz);
    const float velMag = std::sqrt(vx * vx + vy * vy + vz * vz);
    float mag = (posMag > velMag) ? posMag : velMag;

    if (!std::isfinite(mag) || mag <= 0.0001f)
    {
        return;
    }

    // Transform by view matrix (viewData points at a matrix in X360Stuff).
    D3DXVECTOR4 vec(motionVec[0], motionVec[1], motionVec[2], motionVec[3]);
    D3DXMATRIX* viewMatrix = reinterpret_cast<D3DXMATRIX*>(viewData);
    D3DXVECTOR4 transformed{};
    D3DXVec4Transform(&transformed, &vec, viewMatrix);
    motionVec[0] = transformed.x;
    motionVec[1] = transformed.y;
    motionVec[2] = transformed.z;

    // Normalize
    const float invMag = 1.0f / mag;
    motionVec[0] *= invMag;
    motionVec[1] *= invMag;
    motionVec[2] *= invMag;

    // Magnitude scaling (X360Stuff)
    float magnitude = (mag - 15.0f) * 0.009090909f;
    if (mag <= 15.0f)
        magnitude = 0.0f;
    else if (mag >= 125.0f || magnitude > 0.65f)
        magnitude = 0.65f;
    magnitude *= 0.02f;

    static void** visualLookParams = reinterpret_cast<void**>(0x00982AF0);
    float intensity = 1.0f;
    if (visualLookParams && *visualLookParams)
    {
        auto params = static_cast<char*>(*visualLookParams);
        intensity = *reinterpret_cast<float*>(params + 0x178);
    }

    const float finalScale = (intensity * 0.75f + 1.2f) * magnitude;
    const float aspect = (g_LastBackBufferH != 0)
                             ? (static_cast<float>(g_LastBackBufferW) / static_cast<float>(g_LastBackBufferH))
                             : 1.0f;

    float outVec[4]{};
    outVec[0] = finalScale * motionVec[0] / aspect;
    outVec[1] = finalScale * motionVec[1];
    outVec[2] = finalScale * motionVec[2];
    outVec[3] = 0.0f;

    if (magnitude < 0.0f)
        magnitude = 0.0f;
    else if (magnitude > 1.0f)
        magnitude = 1.0f;

    // Commit only on success so failures don't stomp other motion sources.
    g_MotionVec[0] = outVec[0];
    g_MotionVec[1] = outVec[1];
    g_MotionVec[2] = outVec[2];
    g_MotionVec[3] = outVec[3];
    g_MotionBlurAmount = magnitude;
    g_LastMotionUpdateTick = GetTickCount();

    static DWORD s_lastDbg = 0;
    const DWORD now = GetTickCount();
    if (now - s_lastDbg >= 1000)
    {
        s_lastDbg = now;
        printf_s("[MotionVec] eView mag=%.3f pos=%.3f vel=%.3f finalAmt=%.4f dx=%.3f vx=%.3f\n",
                 mag, posMag, velMag, magnitude, dx, vx);
    }
}


static void EnsureDeviceVtableHooks(IDirect3DDevice9* dev)
{
    if (!dev) 
        return;
    if (g_RenderTargetManager.g_DeviceResetInProgress)
        return;
    void** vtable = *reinterpret_cast<void***>(dev);
    if (!vtable) 
        return;

    DWORD oldProtect = 0;

    // Ensure Reset (index 16) is hooked (ShaderLoader reloads often trigger Reset).
    if (vtable[16] != reinterpret_cast<void*>(&hkReset))
    {
        g_RenderTargetManager.oReset = reinterpret_cast<Reset_t>(vtable[16]);
        VirtualProtect(&vtable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[16] = reinterpret_cast<void*>(&hkReset);
        VirtualProtect(&vtable[16], sizeof(void*), oldProtect, &oldProtect);
    }

    // Do NOT hook Present here. Present is commonly hooked by other mods; swapping it risks recursion and perf issues.

    // Do NOT hook EndScene here. Hooking EndScene is high-risk in mod stacks (perf + reload stability).

    // Ensure SetTransform (index 44) stays hooked. Other mods can hot-swap vtables after us.
    if (vtable[44] != reinterpret_cast<void*>(&hkSetTransform))
    {
        g_RenderTargetManager.oSetTransform =
            reinterpret_cast<RenderTargetManager::SetTransform_t>(vtable[44]);
        VirtualProtect(&vtable[44], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[44] = reinterpret_cast<void*>(&hkSetTransform);
        VirtualProtect(&vtable[44], sizeof(void*), oldProtect, &oldProtect);
    }

    // Ensure SetRenderTarget (index 37) stays hooked too (scene RT locking relies on it).
    if (vtable[37] != reinterpret_cast<void*>(&hkSetRenderTarget))
    {
        g_RenderTargetManager.oSetRenderTarget =
            reinterpret_cast<RenderTargetManager::SetRenderTarget_t>(vtable[37]);
        VirtualProtect(&vtable[37], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[37] = reinterpret_cast<void*>(&hkSetRenderTarget);
        VirtualProtect(&vtable[37], sizeof(void*), oldProtect, &oldProtect);
    }

    // Ensure SetDepthStencilSurface (index 39) stays hooked so we can force INTZ during the scene pass.
    if (vtable[39] != reinterpret_cast<void*>(&hkSetDepthStencilSurface))
    {
        g_RenderTargetManager.oSetDepthStencilSurface =
            reinterpret_cast<RenderTargetManager::SetDepthStencilSurface_t>(vtable[39]);
        VirtualProtect(&vtable[39], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[39] = reinterpret_cast<void*>(&hkSetDepthStencilSurface);
        VirtualProtect(&vtable[39], sizeof(void*), oldProtect, &oldProtect);
    }
}

HRESULT WINAPI hkSetDepthStencilSurface(LPDIRECT3DDEVICE9 device, IDirect3DSurface9* pNewZStencil)
{
    // Track the last depth surface the game requested (for debugging / potential restore paths).
    if (pNewZStencil && !IsBadReadPtr(pNewZStencil, sizeof(void*)) && !IsBadReadPtr(
        *(void**)pNewZStencil, sizeof(void*)))
    {
        SAFE_RELEASE(g_RenderTargetManager.g_LastGameDepthSurface);
        g_RenderTargetManager.g_LastGameDepthSurface = pNewZStencil;
        g_RenderTargetManager.g_LastGameDepthSurface->AddRef();
    }

    // Force INTZ depth for the game: keep it always-on once created.
    // Minimal option 1: reuse the game's existing GAINMAP, only fix HEIGHTMAP_TEXTURE.
    // This means depth must actually be written during normal rendering.
    if (device && g_RenderTargetManager.g_IntzDepthSurface)
    {
        return g_RenderTargetManager.oSetDepthStencilSurface
                   ? g_RenderTargetManager.oSetDepthStencilSurface(device, g_RenderTargetManager.g_IntzDepthSurface)
                   : D3D_OK;
    }

    return g_RenderTargetManager.oSetDepthStencilSurface
               ? g_RenderTargetManager.oSetDepthStencilSurface(device, pNewZStencil)
               : D3D_OK;
}


static void RebindVisualTreatmentIfChanged()
{
    // ShaderLoader hot-reload swaps effect pointers without calling ApplyGraphicsSettings reliably.
    // Poll the live IVisualTreatment slot and re-bind our textures/params if the effect changed.
    // Do NOT gate on ShaderLoader detection. Some setups rename/pack the module and detection becomes unreliable.
    // This function is safe enough to run per-frame; it exits early if VT object/effect isn't available.
    if (g_RenderTargetManager.g_DeviceResetInProgress)
        return;
    if (!g_pVisualTreatment || IsBadReadPtr(g_pVisualTreatment, sizeof(void*)))
        return;
    if (!*g_pVisualTreatment)
        return;

    void* vtObject = *g_pVisualTreatment;
    ID3DXEffect* fx = FindVtEffectFromObject(vtObject);
    if (!fx || !IsFxReadable(fx))
        return;

    static ID3DXEffect* s_lastFx = nullptr;
    static uint32_t s_lastFrame = 0;
    const bool shouldRebindThisFrame = (eFrameCounter != s_lastFrame);
    if (shouldRebindThisFrame)
        s_lastFrame = eFrameCounter;

    // With ShaderLoader, the effect pointer can stay the same while internal parameters/handles are reloaded.
    // Rebind at least once per frame so our textures/params win.
    const bool forceRebind = shouldRebindThisFrame;

    if (fx != s_lastFx || g_RenderTargetManager.g_PendingVTRebind || forceRebind)
    {
        s_lastFx = fx;
        g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"] = fx;
        g_RenderTargetManager.g_LastReloadedFx = fx;
        g_LastVtFxObserved = fx;
        ReloadBlurBindings(fx, "IDI_VISUALTREATMENT_FX");
        g_RenderTargetManager.g_PendingVTRebind = false;
    }
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

    // IMPORTANT: When ShaderLoader is present, keep this hook minimal.
    // ShaderLoader's hot-reload path is sensitive to re-entrancy and extra D3D calls here have
    // repeatedly correlated with 0xc0000409 fastfails during reload.
    // We keep our blur pipeline running via:
    // - hkSetRenderTarget scene lock + ReloadBlurBindings
    // - sub_6D3B80 blur override
    if (!g_HasShaderLoader)
    {
        IDirect3DDevice9* dev = g_Device;
        if (dev && !g_RenderTargetManager.g_DeviceResetInProgress)
        {
            if (FAILED(dev->TestCooperativeLevel()))
                return;

            EnsureDeviceVtableHooks(dev);

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

            CopyScene(dev);
            MotionBlurPass::CustomMotionBlurHook();
            RebindVisualTreatmentIfChanged();
        }
    }

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
    // No SEH here (causes C2712 in MSVC when unwinding is required).
    // Keep it conservative: readable + COM-ish.
    if (!fx) return false;
    if (!IsFxReadable(fx)) return false;
    return IsProbablyCOMObject(fx);
}

void ReloadBlurBindings(ID3DXEffect* fx, const std::string& name = "")
{
    if (!IsValidFx(fx))
        return;

    if (g_RenderTargetManager.g_DeviceResetInProgress)
        return;

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

    // If we don't have the full scene, don't enable blur. The fallback copies can include UI/front-end.
    if (!g_RenderTargetManager.g_LastSceneFullTex)
        blurTex = nullptr;

    // --- bind blur + amount ---
    if (!blurTex || !hBlur)
    {
        if (hBlur) fx->SetTexture(hBlur, nullptr);
        if (hAmt) fx->SetFloat(hAmt, 0.0f);
    }
    else
    {
        fx->SetTexture(hBlur, blurTex);

        // if you want to drive it globally, keep g_MotionBlurAmount authoritative:
        // Debug override: hold Shift to force a visible blur blend and validate shader path.
        float amt = g_MotionBlurAmount;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            amt = 0.15f;
        // If history exists but amount is stuck at 0 due to missing motion source, allow forcing via F7.
        // This is a diagnostic to confirm the custom blur path is visible.
        static bool s_force = false;
        static SHORT s_prev = 0;
        SHORT cur = GetAsyncKeyState(VK_F7);
        if ((cur & 0x8000) && !(s_prev & 0x8000))
            s_force = !s_force;
        s_prev = cur;
        if (s_force)
            amt = 0.15f;
        if (hAmt) fx->SetFloat(hAmt, amt);
    }

    // Do NOT stomp GAINMAP (MISCMAP2) for VT; we reuse whatever the game binds.
    // Only bind HeightMapTexture to INTZ to enable depth sampling (option 1).
    if (hBlurVector)
        fx->SetVector(hBlurVector, reinterpret_cast<const D3DXVECTOR4*>(g_MotionVec));
    if (hBlurParams)
    {
        const D3DXVECTOR4 params(g_MotionBlurAmount, 1.0f, 0.0f, 0.0f);
        fx->SetVector(hBlurParams, &params);
    }

    // X360Stuff VT expects HeightMapTexture to be a shader-readable INTZ depth texture.
    // Bind it if available; otherwise leave game-provided binding as-is.
    if (name == "IDI_VISUALTREATMENT_FX" && g_RenderTargetManager.g_IntzDepthTex)
    {
        if (auto h = fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE"))
            fx->SetTexture(h, g_RenderTargetManager.g_IntzDepthTex);
        else if (auto h = fx->GetParameterByName(nullptr, "HeightMapTexture"))
            fx->SetTexture(h, g_RenderTargetManager.g_IntzDepthTex);
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
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP1_TEXTURE"))
            fx->SetTexture(
                h, g_RenderTargetManager.g_ExposureTex);
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP2_TEXTURE"))
            fx->SetTexture(
                h, g_RenderTargetManager.g_VignetteTex);
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP3_TEXTURE"))
            fx->SetTexture(
                h, g_RenderTargetManager.g_BloomLUTTex);
        if (auto h = fx->GetParameterByName(nullptr, "MISCMAP4_TEXTURE"))
            fx->SetTexture(
                h, g_RenderTargetManager.g_DofTex);
        if (auto h = fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE"))
            fx->SetTexture(
                h, g_RenderTargetManager.g_DepthTex);
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

        // Capture the vignette mask actually used by VT.
        //
        // NGG disasm shows VT binds UVESVIGNETTE into a param named "VIGNETTETEX". This is the
        // correct mask that fades radial/motion blur and prevents fullscreen haze. Using MISCMAP2
        // (GAINMAP) here is unreliable because many effects reuse MISCMAP2 for unrelated control.
        //
        // We store it in g_VTGainMapTex for now (naming legacy), and MotionBlurPass will consume it
        // as MOTIONBLUR_MASK_TEXTURE.
        {
            IDirect3DBaseTexture9* gainBase = nullptr;
            // Only accept the actual VT vignette texture. Do NOT fall back to MISCMAP2/GAINMAP here:
            // those are often bound to unrelated control maps and are frequently ~white, which causes
            // fullscreen haze in our composite.
            D3DXHANDLE hGain = fx->GetParameterByName(nullptr, "VIGNETTETEX");
            if (!hGain) hGain = fx->GetParameterByName(nullptr, "UVESVIGNETTE");
            if (hGain)
                fx->GetTexture(hGain, &gainBase);

            IDirect3DTexture9* gainTex = nullptr;
            if (gainBase)
            {
                gainBase->QueryInterface(__uuidof(IDirect3DTexture9), (void**)&gainTex);
                gainBase->Release();
            }

            if (gainTex && gainTex != g_RenderTargetManager.g_VTGainMapTex)
            {
                SAFE_RELEASE(g_RenderTargetManager.g_VTGainMapTex);
                g_RenderTargetManager.g_VTGainMapTex = gainTex; // keep ref
            }
            else if (gainTex)
            {
                gainTex->Release();
            }
            else
            {
                // Ensure we don't keep a stale/incorrect mask across reloads.
                SAFE_RELEASE(g_RenderTargetManager.g_VTGainMapTex);
            }
        }

        // Bind optional motion blur mask (BlurMask.png) if the shader supports it (MW360Tweaks style).
        // This should reduce blur on cars/center and prevents a uniform "tight blur on everything".
        if (auto h = fx->GetParameterByName(nullptr, "MotionBlurMask"))
            fx->SetTexture(h, g_RenderTargetManager.g_MotionBlurMaskTex);
        else if (auto h2 = fx->GetParameterByName(nullptr, "MOTIONBLUR_MASK_TEXTURE"))
            fx->SetTexture(h2, g_RenderTargetManager.g_MotionBlurMaskTex);

        // Capture the depth texture actually used by VT (if any). This is more reliable on PC builds
        // than our own g_DepthTex which may never be populated.
        {
            IDirect3DBaseTexture9* depthBase = nullptr;
            D3DXHANDLE hDepth = fx->GetParameterByName(nullptr, "HEIGHTMAP_TEXTURE");
            if (!hDepth) hDepth = fx->GetParameterByName(nullptr, "DOFTexture");
            if (!hDepth) hDepth = fx->GetParameterByName(nullptr, "DEPTHBUFFER_TEXTURE");
            if (hDepth)
                fx->GetTexture(hDepth, &depthBase);

            IDirect3DTexture9* depthTex = nullptr;
            if (depthBase)
            {
                depthBase->QueryInterface(__uuidof(IDirect3DTexture9), (void**)&depthTex);
                depthBase->Release();
            }

            if (depthTex && depthTex != g_RenderTargetManager.g_VTDepthTex)
            {
                SAFE_RELEASE(g_RenderTargetManager.g_VTDepthTex);
                g_RenderTargetManager.g_VTDepthTex = depthTex; // keep ref
            }
            else if (depthTex)
            {
                depthTex->Release();
            }
        }
    }

    // Commit is fine (and cheap) - but don't spam logs every call
    fx->CommitChanges();

    // No periodic logging here; this runs at gameplay framerate and logging destroys FPS.
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
    // We may see a valid device pointer here, but do NOT patch vtables from inside D3DX effect creation.
    // ShaderLoader hot-reload/Reset paths can call into D3DX while the device is transitioning; vtable writes
    // here have caused instability (0xc0000409) in practice. DeferredHookThread handles vtable installation.
    if (device)
        SetGameDevice(device);

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
                    const char* errText = (const char*)(*outErrors)->GetBufferPointer();
                    printf_s("[XNFS] ? FX override compile failed: %s\n",
                             errText ? errText : "(null)");
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
                g_LastVtFxObserved = *outEffect;
                printf_s("[XNFS] ✅ Shader created and tracked (effect): %s (%p)\n",
                         pResource, (void*)*outEffect);
            }

            // Rebind our textures if blur history is ready (safe no-op otherwise).
            if (g_RenderTargetManager.g_CurrentBlurTex)
                ReloadBlurBindings(*outEffect, "IDI_VISUALTREATMENT_FX");
        }
        return hr;
    }

    if (isOverbrightFx)
    {
        auto& existingFx = g_RenderTargetManager.g_ActiveEffects[pResource];

        if (existingFx && existingFx != *outEffect)
        {
            printf_s("[XNFS] 🔁 Replacing previous effect (%p) for %s\n", existingFx, pResource);
            if (IsValidShaderPointer(existingFx))
                existingFx->Release();
            existingFx = nullptr;
        }

        existingFx = *outEffect;
        (*outEffect)->AddRef(); // Retain since we're storing it
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

HRESULT WINAPI hkSetRenderTarget(LPDIRECT3DDEVICE9 device, DWORD index, IDirect3DSurface9* renderTarget)
{
    // Re-entrancy guard: avoid recursion when we call StretchRect during scene capture.
    static thread_local bool s_inHook = false;
    if (s_inHook)
        return g_RenderTargetManager.oSetRenderTarget
                   ? g_RenderTargetManager.oSetRenderTarget(device, index, renderTarget)
                   : D3D_OK;
    s_inHook = true;

    if (device && index == 0)
    {
        static uint32_t lastRtLogFrame = 0;
        static uint32_t lastRtFrame = 0;
        static IDirect3DSurface9* bestSceneRt = nullptr;
        static bool sawSmallRtThisFrame = false;
        if (eFrameCounter != lastRtFrame)
        {
            lastRtFrame = eFrameCounter;
            g_CustomMotionBlurRanThisFrame = false;
            g_SceneTargetThisFrame = nullptr;
            g_SceneTargetBestArea = 0;
            SAFE_RELEASE(bestSceneRt);
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
                renderTarget == g_RenderTargetManager.g_BlurHistorySurfA ||
                renderTarget == g_RenderTargetManager.g_BlurHistorySurfB);

        // Never consider the blur destination RT (DoMotionBlur output) as a scene-color candidate.
        const bool isBlurDst = (g_BlurDstSurfaceThisFrame && g_BlurDstFrame == eFrameCounter && renderTarget ==
            g_BlurDstSurfaceThisFrame);

        if (renderTarget)
        {
            D3DSURFACE_DESC sd{};
            if (SUCCEEDED(renderTarget->GetDesc(&sd)))
            {
                if (g_KnownRenderTargets.insert(renderTarget).second)
                {
                    // printf_s("[XNFS] ? RT0 set: %ux%u fmt=%u ptr=%p\n",
                    //          sd.Width, sd.Height, sd.Format, renderTarget);
                }
            }
        }

        if (renderTarget && renderTarget != g_RenderTargetManager.g_BackBufferSurface && !isOurBlurSurface && !
            isBlurDst)
        {
            D3DSURFACE_DESC sd{};
            if (SUCCEEDED(renderTarget->GetDesc(&sd)) && bbDesc.Width && bbDesc.Height)
            {
                // Pick the "best" scene RT for blur source this frame.
                // Many setups render the main scene at a higher resolution than the backbuffer
                // (e.g. 2560x1440 with a 1920x1080 backbuffer). If we only accept exact backbuffer size,
                // we often end up selecting a post-process RT that already contains blur.
                const uint64_t bbArea = uint64_t(bbDesc.Width) * uint64_t(bbDesc.Height);
                const uint64_t area = uint64_t(sd.Width) * uint64_t(sd.Height);
                const float bbAspect = float(bbDesc.Width) / float(bbDesc.Height);
                const float rtAspect = float(sd.Width) / float(sd.Height);
                const float aspectRatio = (bbAspect > 0.0f) ? (rtAspect / bbAspect) : 1.0f;

                const bool aspectOk = (aspectRatio > 0.90f && aspectRatio < 1.10f);
                const bool areaOk = (area >= bbArea && area <= bbArea * 4ULL);

                if (aspectOk && areaOk && area > g_SceneTargetBestArea)
                {
                    g_SceneTargetBestArea = area;
                    SAFE_RELEASE(bestSceneRt);
                    bestSceneRt = renderTarget;
                    bestSceneRt->AddRef();
                }
                if (sd.Width < bbDesc.Width || sd.Height < bbDesc.Height)
                    sawSmallRtThisFrame = true;

                // Lock in the scene RT.
                // Prefer waiting until we see smaller RTs (post stack) to avoid selecting flare/lighting RTs,
                // but if we still don't have any scene RT yet, lock to the best candidate immediately.
                if (bestSceneRt && (sawSmallRtThisFrame || !g_RenderTargetManager.g_LastSceneFullTex))
                {
                    g_RenderTargetManager.g_LastSceneFullFrame = eFrameCounter;
                    if (g_RenderTargetManager.g_LastSceneFullSurface != bestSceneRt)
                    {
                        SAFE_RELEASE(g_RenderTargetManager.g_LastSceneFullSurface);
                        g_RenderTargetManager.g_LastSceneFullSurface = bestSceneRt;
                        g_RenderTargetManager.g_LastSceneFullSurface->AddRef();
                    }

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

                    if (eFrameCounter != lastRtLogFrame)
                    {
                        lastRtLogFrame = eFrameCounter;
                        printf_s("[XNFS] ? Scene RT locked: %ux%u fmt=%u usage=%u ptr=%p\n",
                                 sd.Width, sd.Height, sd.Format, sd.Usage, (void*)bestSceneRt);
                    }

                    // Keep our dedicated scene-color copy up to date for blur.
                    CaptureSceneColor(device);

                    static uint32_t lastRebindFrame = 0;
                    if (lastRebindFrame != eFrameCounter)
                    {
                        lastRebindFrame = eFrameCounter;
                        auto it = g_RenderTargetManager.g_ActiveEffects.find("IDI_VISUALTREATMENT_FX");
                        if (it != g_RenderTargetManager.g_ActiveEffects.end() && IsValidEffectObject(it->second))
                            ReloadBlurBindings(it->second, "IDI_VISUALTREATMENT_FX");
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

    HRESULT hr = g_RenderTargetManager.oSetRenderTarget
                     ? g_RenderTargetManager.oSetRenderTarget(device, index, renderTarget)
                     : D3D_OK;
    s_inHook = false;
    return hr;
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND hwnd,
                             const RGNDATA* dirty)
{
    // Some mod stacks never route through our "HookedPresent" trampoline, and others already own Present.
    // This function must stay side-effect free to avoid recursion, stack overflows, and shader reload crashes.
    SetGameDevice(device);
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

// Forward decl (used by SafeApplyGraphicsSettingsMain)
bool IsProbablyValidManager(void* manager);

static void SafeApplyGraphicsSettingsMain(void* manager)
{
    // No SEH here. Validate manager before calling game code.
    if (!manager || !IsProbablyValidManager(manager))
        return;
    ApplyGraphicsManagerMainOriginal(manager);
    printf_s("[HotReload] ✅ Applied GraphicsSettings\n");
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
    // Re-apply our bindings to the *active* VT effect instance.
    ReloadBlurBindings(fx, "IDI_VISUALTREATMENT_FX");
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
    if (IsFxReadable(fx) && IsProbablyCOMObject(fx))
    {
        vtbl = *(void***)fx;
        printf_s("[Debug:%s] fx = %p, vtable = %p, vtable[0] = %p\n", label, fx, vtbl, vtbl ? vtbl[0] : nullptr);
    }
    else
    {
        printf_s("[Debug:%s] ❌ fx unreadable or not COM-like: %p\n", label, fx);
    }
}

void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject)
{
    LogApplyGraphicsSettingsCall(manager, vtObject, 1);

    // Never block the game's original ApplyGraphicsSettings. ShaderLoader and the engine rely on it.

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

    const bool validVtObject = IsValidThis(vtObject);

    // ShaderLoader interop:
    // Do not patch vtObject effect slots or call effect reset methods here.
    // ShaderLoader will swap/recreate effects; we only re-bind our textures/params.
    if (g_HasShaderLoader && validVtObject && !g_WaitingForReset)
    {
        ID3DXEffect* fx = nullptr;
        void* slotPtr = (char*)vtObject + 0x18C;
        if (!IsBadReadPtr(slotPtr, sizeof(void*)))
            fx = *reinterpret_cast<ID3DXEffect**>(slotPtr);

        if (fx && IsValidEffectObject(fx))
        {
            g_RenderTargetManager.g_ActiveEffects["IDI_VISUALTREATMENT_FX"] = fx;
            g_RenderTargetManager.g_LastReloadedFx = fx;
            ReloadBlurBindings(fx, "IDI_VISUALTREATMENT_FX");
        }

        LogApplyGraphicsSettingsCall(manager, vtObject, 2);
        if (ApplyGraphicsSettingsOriginal)
            ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
        return; // we intentionally skip our non-ShaderLoader hot-reload path
    }

    if (!validVtObject)
    {
        // Still call original at the end.
        LogApplyGraphicsSettingsCall(manager, vtObject, 2);
        if (ApplyGraphicsSettingsOriginal)
            ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
        return;
    }

    if (g_WaitingForReset)
    {
        LogApplyGraphicsSettingsCall(manager, vtObject, 2);
        if (ApplyGraphicsSettingsOriginal)
            ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
        return;
    }

    // Apply internal hot-reload if requested: compile on this thread, then patch vtObject+0x18C.
    if (g_PendingVisualTreatmentRecompile.exchange(false))
    {
        if (g_RenderTargetManager.g_DeviceResetInProgress || !g_Device ||
            FAILED(g_Device->TestCooperativeLevel()))
        {
            // Defer until the device is usable again.
            g_PendingVisualTreatmentRecompile.store(true);
            return;
        }

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (d3dx && g_Device)
        {
            auto createFromFile = reinterpret_cast<D3DXCreateEffectFromFileAFn>(
                GetProcAddress(d3dx, "D3DXCreateEffectFromFileA"));
            if (createFromFile)
            {
                ID3DXEffect* newFx = nullptr;
                ID3DXBuffer* errors = nullptr;
                const HRESULT hr = createFromFile(g_Device, "fx\\visualtreatment.fx",
                                                  nullptr, nullptr, 0, nullptr, &newFx, &errors);
                if (FAILED(hr) || !newFx)
                {
                    if (errors)
                    {
                        const char* errText = (const char*)errors->GetBufferPointer();
                        printf_s("[HotReload] ❌ visualtreatment.fx compile failed: %s\n",
                                 errText ? errText : "(null)");
                        errors->Release();
                    }
                    else
                    {
                        printf_s("[HotReload] ❌ visualtreatment.fx compile failed: 0x%08X\n", (unsigned)hr);
                    }
                }
                else
                {
                    if (errors) errors->Release();
                    printf_s("[HotReload] ✅ Applying VT swap (%p) to vtObject=%p\n", (void*)newFx, vtObject);
                    TryPatchSlotIfWritable(vtObject, 0x18C, newFx);
                    ReloadBlurBindings(newFx, "IDI_VISUALTREATMENT_FX");
                    newFx->Release();
                }
            }
        }
    }

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
            // Patch the live IVisualTreatment effect slot directly.
            // Do not gate this on g_SlotRetainedFx: that creates a chicken-and-egg where reload never applies.
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

    LogApplyGraphicsSettingsCall(manager, vtObject, 2);

    if (ApplyGraphicsSettingsOriginal)
        ApplyGraphicsSettingsOriginal(manager, nullptr, vtObject);
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
    // Intentionally disabled: do not hook D3DX* in this mod.
    // ShaderLoader loads after us and relies on those entrypoints; hook chaining breaks reload.
    return;
    if (g_D3DXHooksInstalled)
        return;

    HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
    if (!d3dx)
        return;

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
        printf_s("[XNFS] ⚠️ g_CurrentBlurTex null — skipped effect rebinding (will rebind on next EndScene)\n");
    }
}

void OnDeviceLost()
{
    g_RenderTargetManager.OnDeviceLost(); // Delegate to RenderTargetManager's cleanup
    // Drop any per-frame tracked surfaces; they become invalid across device loss/reload.
    SafeReleaseSurfaceTracked(g_BlurDstSurfaceThisFrame);
    g_BlurDstFrame = 0;
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

    // If this ever fires, we'd otherwise crash by calling a null function pointer (EIP=0).
    if (!g_RenderTargetManager.oReset)
    {
        printf_s("[XNFS] ❌ hkReset called but oReset is null (device=%p)\n", (void*)device);
        g_RenderTargetManager.g_DeviceResetInProgress = false;
        return D3DERR_INVALIDCALL;
    }

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

    // Release resources before attempting Reset.
    OnDeviceLost();

    printf_s(
        "[XNFS] Reset start: coop=0x%08X bb=%ux%u fmt=%u win=%d msaa=%u\n",
        coop,
        params->BackBufferWidth,
        params->BackBufferHeight,
        params->BackBufferFormat,
        params->Windowed,
        params->MultiSampleType
    );

    // In windowed mode, always force UNKNOWN. Some reload/reset paths pass a stale explicit format.
    if (params->Windowed)
    {
        if (params->BackBufferFormat != D3DFMT_UNKNOWN)
        {
            printf_s(
                "[XNFS] Forcing BackBufferFormat to UNKNOWN for windowed reset (was %u)\n",
                params->BackBufferFormat
            );
        }
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
    if (!device) 
        return oEndScene(device);

    if (!g_Device) 
        SetGameDevice(device);

    // Keep EndScene extremely lightweight. EndScene runs at full framerate and is a common integration point
    // for other mods; heavy work here tanks FPS and increases crash risk under DXVK.
    //
    // Motion blur work happens inside the engine blur pass hook (XNFS_Sub6D3B80_Hook), not here.
    EnsureDeviceVtableHooks(device);

    // Control vanilla DoMotionBlur toggle per-frame:
    // - When our override is active, disable vanilla to avoid fullscreen blur stacking.
    // - When inactive, allow vanilla (so the game stays functional).
    // Vanilla blur control is handled in XNFS_Sub6D3B80_Hook (guaranteed to run on the blur path).

    return oEndScene(device);
}

HRESULT WINAPI hkPresent(IDirect3DDevice9* dev,
                         const RECT* src, const RECT* dst,
                         HWND wnd, const RGNDATA* dirty)
{
    if (!dev)
        return D3DERR_INVALIDCALL;

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

    // Intentionally keep hkPresent minimal. ShaderLoader and other mods hook Present; we do our
    // per-frame work in RenderDispatchHook to avoid breaking reload.

    // Call the stable "original" present if we have it. Other mods may hot-swap vtable entries
    // (e.g. on shader reload), which can invalidate a one-time captured pointer.
    PresentFn next = RealPresent ? RealPresent : oPresent;
    if (!next)
    {
        printf_s("[XNFS] ❌ hkPresent: next present is null\n");
        return D3DERR_INVALIDCALL;
    }
    return next(dev, src, dst, wnd, dirty);
}


DWORD WINAPI DeferredHookThread(LPVOID)
{
    while (!g_Device)
        Sleep(10);

    // ShaderLoader may load after us; re-check before deciding to install any D3DX hooks.
    if (!g_HasShaderLoader)
        g_HasShaderLoader = DetectShaderLoader();

    if (!g_Device) return E_FAIL;
    void** vtable = *(void***)g_Device;
    if (!vtable)
        return E_FAIL;

    // Hook Reset only. Other hooks are installed via safer callsite/vtable paths to avoid shader reload crashes.
    g_RenderTargetManager.oReset = (Reset_t)vtable[16];
    DWORD oldProtect;
    VirtualProtect(&vtable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    vtable[16] = (void*)&hkReset;
    VirtualProtect(&vtable[16], sizeof(void*), oldProtect, &oldProtect);
    printf_s("[Init] Hooked IDirect3DDevice9::Reset (deferred)\n");

    // No D3DX hooks (ShaderLoader compatibility).
    return 0;
}

// Disable heavy EndScene work by default (perf + stability). Flip to 1 only for targeted debugging.
#ifndef XNFS_ENABLE_HEAVY_ENDSCENE
#define XNFS_ENABLE_HEAVY_ENDSCENE 0
#endif

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_RenderTargetManager.g_hModule = hModule;
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        printf_s("[Init] Shader override DLL loaded.\n");

        // Do not log "ShaderLoader detected: NO" here; many setups load ShaderLoader after us.
        // We'll log only when we detect it at runtime (see DetectShaderLoaderLate()).
        g_HasShaderLoader = DetectShaderLoader();

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (d3dx)
        {
            // NOTE: Avoid hooking D3DXCreateEffectFromResourceA directly here.
            // ShaderLoader hooks this path and hot-reloads effects; chaining multiple hooks has
            // proven unstable (0xc0000409) during reload. We only hook file/effect creation.
            void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
            g_RenderTargetManager.RealCreateFromResource = (RenderTargetManager::D3DXCreateEffectFromResourceAFn)addr;

            // Do not hook D3DXCreateEffectFromFileA/D3DXCreateEffect at all.
            // ShaderLoader owns effect hot-reload and hook chaining is known to break it.

            // Intentionally do NOT hook IVisualTreatment::Get (0x006DFAF0).
            // It is unstable in MW due to invalid vtables during reload/transition and can crash the game.

            injector::MakeJMP(0x00750B10, RenderDispatchHook, true);
            PatchMotionBlurFix();
            InstallSub6D3B80BlurHook();
            // Do NOT hook 0x006DBB20 (DoMotionBlur) via JMP.
            // Short-circuiting DoMotionBlur prevents the engine from reaching the sub_6D3B80 callsites we patch.
            // InstallDoMotionBlurHook();
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
            if (!g_HasShaderLoader)
                CreateThread(nullptr, 0, HotReloadThread, nullptr, 0, nullptr);
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
