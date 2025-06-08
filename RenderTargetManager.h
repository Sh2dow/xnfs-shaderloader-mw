#pragma once
#include <unordered_map>
#include "FxWrapper.h"
#include "Hooks.h"

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

class RenderTargetManager
{
public:
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


    typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

    // -------------------- NFSMW-RenderTarget block --------------------

    // HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params);
    // HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 device);
    // HRESULT WINAPI HookedPresent(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND hwnd,
    //                                     const RGNDATA* dirty);

    void RenderTargetManager::ReloadBlurBindings(ID3DXEffect* fx, const std::string& name = "");

    void RenderTargetManager::RenderBlurPass(IDirect3DDevice9* device);
    
    void OnDeviceLost();

    bool OnDeviceReset(LPDIRECT3DDEVICE9 device);

    void Swap();
};
