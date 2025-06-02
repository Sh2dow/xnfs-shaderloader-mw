#pragma once
#include <d3dx9effect.h>

class Validators
{
    
public:
};

inline extern bool IsValidCodePtr(void* ptr);
extern bool IsValidThis(void* ptr);
extern bool IsValidShaderPointer_SEH(ID3DXEffect* fx, void*** outVtable, DWORD* outProtect);
extern bool IsValidShaderPointer(ID3DXEffect* fx);
extern bool IsD3D9ExAvailable();
extern bool IsSafeToReload();
inline bool IsInCutsceneOrFrontend();