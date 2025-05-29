#pragma once
#include "FxWrapper.h"

class Validators
{
    
public:
};

extern inline bool IsValidCodePtr(void* ptr);
extern bool IsValidShaderPointer_SEH(FxWrapper* fx, void*** outVtable, DWORD* outProtect);
extern bool IsValidShaderPointer(FxWrapper* fx);
extern bool IsValidThis(void* ptr);