#pragma once
#include <d3d9.h>

class globals
{
public:
    
};

inline IDirect3DTexture9* g_MotionBlurTexA = nullptr;
inline IDirect3DTexture9* g_MotionBlurTexB = nullptr;
inline IDirect3DTexture9* g_CurrentBlurTex = nullptr;
inline IDirect3DSurface9* g_CurrentBlurSurface = nullptr;
inline bool g_UseTexA = true;
inline UINT g_Width = 0;
inline UINT g_Height = 0;