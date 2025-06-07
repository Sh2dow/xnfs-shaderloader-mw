#pragma once

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }
#include <unordered_map>

class RenderTargetManager
{
public:
    HMODULE g_hModule = nullptr;
    bool g_DeviceResetInProgress = false;
    int g_ApplyDelayCounter = 0;
    bool g_ApplyScheduled = false;
    int g_ApplyGraphicsTriggerDelay = 0;

    std::unordered_map<std::string, LPD3DXEFFECT> g_ActiveEffects;

    void* g_ApplyGraphicsManagerThis = nullptr;

    ID3DXEffect* g_LastReloadedFx = nullptr;

    IDirect3DTexture9* g_MotionBlurTexA = nullptr;
    IDirect3DTexture9* g_MotionBlurTexB = nullptr;
    IDirect3DTexture9* g_CurrentBlurTex = nullptr;
    IDirect3DSurface9* g_CurrentBlurSurface = nullptr;

    IDirect3DTexture9* g_GainMapTex = nullptr;
    IDirect3DTexture9* g_VignetteTex = nullptr;
    IDirect3DTexture9* g_BloomTex = nullptr;
    IDirect3DTexture9* g_DofTex = nullptr;
    IDirect3DTexture9* g_LinearDepthTex = nullptr;
    IDirect3DTexture9* g_ExposureTex = nullptr;
    IDirect3DTexture9* g_BloomLUTTex = nullptr;
    IDirect3DTexture9* g_DepthTex = nullptr;

    bool g_UseTexA = true;
    UINT g_Width = 0;
    UINT g_Height = 0;


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

    HRESULT WINAPI HookedCreateFromResource(
        LPDIRECT3DDEVICE9 device,
        HMODULE hModule,
        LPCSTR pResource,
        const D3DXMACRO* defines,
        LPD3DXINCLUDE include,
        DWORD flags,
        LPD3DXEFFECTPOOL pool,
        LPD3DXEFFECT* outEffect,
        LPD3DXBUFFER* outErrors);


    typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

    // -------------------- NFSMW-RenderTarget block --------------------
    struct Vertex
    {
        float x, y, z, rhw;
        float u, v;
    } screenQuadVerts[4];

    LPDIRECT3DTEXTURE9 g_MyBlurTexture = nullptr;
    LPDIRECT3DSURFACE9 g_MyBlurSurface = nullptr;

    typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
    Reset_t oReset = nullptr;
    HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params);
    HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 device);
    HRESULT WINAPI HookedPresent(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND hwnd,
                                        const RGNDATA* dirty);


    void OnDeviceLost();

    bool OnDeviceReset(LPDIRECT3DDEVICE9 device);

    void Swap();
};
