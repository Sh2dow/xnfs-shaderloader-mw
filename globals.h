#pragma once
#include <d3d9.h>

inline IDirect3DTexture9* g_MotionBlurTexA = nullptr;
inline IDirect3DTexture9* g_MotionBlurTexB = nullptr;
inline IDirect3DTexture9* g_CurrentBlurTex = nullptr;
inline IDirect3DSurface9* g_CurrentBlurSurface = nullptr;
inline bool g_UseTexA = true;
inline UINT g_Width = 0;
inline UINT g_Height = 0;


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

inline D3DXCreateEffectFromResourceAFn RealCreateFromResource;

extern HRESULT WINAPI HookedCreateFromResource(
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

typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
inline Reset_t oReset = nullptr;
inline HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params);
inline HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 device);
inline HRESULT WINAPI HookedPresent(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND hwnd,
                                    const RGNDATA* dirty);
