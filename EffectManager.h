#pragma once

#include <d3d9.h>
#include <d3dx9effect.h>

class EffectManager
{

public:
    static bool TrySetTechnique(ID3DXEffect* fx, const char* const* names, size_t count);
    static bool EnsureCustomBlurEffect(IDirect3DDevice9* device);
};
