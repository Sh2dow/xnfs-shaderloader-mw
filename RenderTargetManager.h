#pragma once
#include "Hooks.h"

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }
#include <string>
#include <unordered_map>


class RenderTargetManager
{
public:
    HMODULE g_hModule = nullptr;
    bool g_DeviceResetInProgress = false;
    int g_ApplyDelayCounter = 0;
    bool g_ApplyScheduled = false;
    int g_ApplyGraphicsTriggerDelay = 0;

    uint32_t g_LastSceneFullFrame = 0;
    // Frame index when g_SceneColorTex was last updated from g_LastSceneFullSurface.
    // Used to avoid sampling stale/cleared scene captures (causes dark/blank output).
    uint32_t g_SceneColorFrame = 0;
    
    std::unordered_map<std::string, LPD3DXEFFECT> g_ActiveEffects;

    void* g_ApplyGraphicsManagerThis = nullptr;

    ID3DXEffect* g_LastReloadedFx = nullptr;
    ID3DXEffect* g_BlurEffect = nullptr;
    ID3DXEffect* g_CustomBlurEffect = nullptr;
    IDirect3DVertexDeclaration9* g_ScreenQuadDecl = nullptr;

    IDirect3DTexture9* g_MotionBlurTexA = nullptr;
    IDirect3DTexture9* g_MotionBlurTexB = nullptr;
    IDirect3DTexture9* g_CurrentBlurTex = nullptr;
    IDirect3DSurface9* g_CurrentBlurSurface = nullptr;
    IDirect3DSurface9* g_MotionBlurSurfaceA = nullptr;
    IDirect3DSurface9* g_MotionBlurSurfaceB = nullptr;

    IDirect3DTexture9* g_GainMapTex = nullptr;
    IDirect3DTexture9* g_VignetteTex = nullptr;
    IDirect3DTexture9* g_BloomTex = nullptr;
    IDirect3DTexture9* g_DofTex = nullptr;
    IDirect3DTexture9* g_LinearDepthTex = nullptr;
    IDirect3DTexture9* g_ExposureTex = nullptr;
    IDirect3DTexture9* g_BloomLUTTex = nullptr;
    IDirect3DTexture9* g_DepthTex = nullptr;
    // Captured from the live VisualTreatment effect (HEIGHTMAP/DOF). Used for depth-masked blur.
    IDirect3DTexture9* g_VTDepthTex = nullptr;
    // Captured from the live VisualTreatment effect (MISCMAP2 / GAINMAP / UVESVIGNETTE).
    // Reused as a blur mask to fade blur near the lower center (existing game behavior).
    IDirect3DTexture9* g_VTGainMapTex = nullptr;
    // Optional blur mask (e.g. from MW360Tweaks BlurMask.png). Used to keep cars/center sharper.
    IDirect3DTexture9* g_MotionBlurMaskTex = nullptr;
    // INTZ depth texture used as both depth-stencil and shader-readable depth map (X360Stuff-style).
    IDirect3DTexture9* g_IntzDepthTex = nullptr;
    IDirect3DSurface9* g_IntzDepthSurface = nullptr;
    IDirect3DSurface9* g_BackBufferSurface = nullptr;
    IDirect3DSurface9* g_LastSceneSurface = nullptr;
    IDirect3DSurface9* g_LastSceneFullSurface = nullptr;
    IDirect3DTexture9* g_LastSceneFullTex = nullptr;

    // Dedicated stable scene-color capture for our blur pipeline (copied from the chosen scene RT).
    IDirect3DTexture9* g_SceneColorTex = nullptr;
    IDirect3DSurface9* g_SceneColorSurface = nullptr;

    // scene copy (final backbuffer copy, optional)
    IDirect3DTexture9* g_SceneCopyTex     = nullptr;
    IDirect3DSurface9* g_SceneCopySurface = nullptr;

    // blur history ping-pong (what VT samples as "PREV/MOTIONBLUR")
    IDirect3DTexture9* g_BlurHistoryTexA  = nullptr;
    IDirect3DSurface9* g_BlurHistorySurfA = nullptr;
    IDirect3DTexture9* g_BlurHistoryTexB  = nullptr;
    IDirect3DSurface9* g_BlurHistorySurfB = nullptr;

    bool g_UseTexA = true;
    UINT g_Width = 0;
    UINT g_Height = 0;
    bool g_PendingVTRebind = false;
    IDirect3DTexture9* g_BackBufferTex = nullptr;


    typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
        LPDIRECT3DDEVICE9,
        HMODULE,
        LPCSTR,
        const D3DXMACRO*,
        LPD3DXINCLUDE,
        DWORD,
        LPD3DXEFFECTPOOL,
        LPD3DXEFFECT*,
        LPD3DXBUFFER*);

    D3DXCreateEffectFromResourceAFn RealCreateFromResource;


    typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

    // -------------------- NFSMW-RenderTarget block --------------------
    struct Vertex
    {
        float x, y, z, rhw;
        float u, v;
    } screenQuadVerts[4];

    struct QuadVertex8
    {
        float x, y, z, rhw;
        float t0[4];
        float t1[4];
        float t2[4];
        float t3[4];
        float t4[4];
        float t5[4];
        float t6[4];
        float t7[4];
    };

    LPDIRECT3DTEXTURE9 g_MyBlurTexture = nullptr;
    LPDIRECT3DSURFACE9 g_MyBlurSurface = nullptr;

    Reset_t oReset = nullptr;
    using SetRenderTarget_t = HRESULT (WINAPI*)(LPDIRECT3DDEVICE9, DWORD, IDirect3DSurface9*);
    SetRenderTarget_t oSetRenderTarget = nullptr;
    using SetTransform_t = HRESULT (WINAPI*)(LPDIRECT3DDEVICE9, D3DTRANSFORMSTATETYPE, const D3DMATRIX*);
    SetTransform_t oSetTransform = nullptr;
    using SetDepthStencilSurface_t = HRESULT (WINAPI*)(LPDIRECT3DDEVICE9, IDirect3DSurface9*);
    SetDepthStencilSurface_t oSetDepthStencilSurface = nullptr;

    // Tracks the last depth-stencil surface the game attempted to set (for restoration).
    IDirect3DSurface9* g_LastGameDepthSurface = nullptr;

    void OnDeviceLost();

    bool OnDeviceReset(LPDIRECT3DDEVICE9 device);

    void Swap();
};
