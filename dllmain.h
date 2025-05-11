#include <algorithm>
#include <atomic>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include "includes/injector/injector.hpp"
#include <d3dx9effect.h>
#include <iostream>
#include <MinHook.h>
#include <mutex>
#include <unordered_set>

#if _DEBUG
#include "Log.h"
#define printf_s(...) asi_log::Log(__VA_ARGS__)
#endif

void* FindFxWrapperByShaderIndex(int shaderIndex);
void RecompileAndReloadAll();
void ReloadShaderFxWrapper(const std::string& shaderKey);
void ForceReplaceShaderIntoSlots(const std::_List_const_iterator<std::_List_val<std::_List_simple_types<std::string>>>::value_type& key, LPD3DXEFFECT fx);
