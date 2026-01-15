#pragma once
#include <d3d9.h>

class MotionBlurPass
{
public:
    static void __cdecl CustomMotionBlurHook();
    static void __cdecl RenderBlurPass(
        IDirect3DDevice9* device,
        IDirect3DTexture9* srcTex,
        IDirect3DTexture9* dstTex,
        IDirect3DSurface9* dstSurface);
    static void __cdecl RenderBlurOverride(
        IDirect3DDevice9* device,
        IDirect3DBaseTexture9* src0,
        IDirect3DBaseTexture9* src1,
        IDirect3DSurface9* dstSurface);
};
