#pragma once
#include "FxWrapper.h"

class Validators
{
    
public:
};

inline extern bool IsValidCodePtr(void* ptr);
extern bool IsValidThis(void* ptr);
extern bool IsValidShaderPointer_SEH(std::shared_ptr<FxWrapper> fx, void*** outVtable, DWORD* outProtect);
extern bool IsValidShaderPointer(std::shared_ptr<FxWrapper> fx);
extern bool IsD3D9ExAvailable();
extern bool IsSafeToReload();
inline bool IsInCutsceneOrFrontend();