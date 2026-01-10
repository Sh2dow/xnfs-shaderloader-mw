#pragma once

#include <d3d9.h>

class MotionBlurPass
{
public:
    static void __cdecl CustomMotionBlurHook();
    static void __cdecl RenderBlurPass(IDirect3DDevice9* device);
    static void __cdecl RenderBlurCompositePass(IDirect3DDevice9* device);
};
