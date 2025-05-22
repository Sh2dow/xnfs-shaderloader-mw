#pragma once
#include <atomic>
#include <d3dx9effect.h>
#include <vector>

// typedef void(__thiscall* ApplyGraphicsSettingsFn)(void* thisptr);
typedef void(__fastcall* ApplyGraphicsSettingsFn)(void* ecx, void* edx, void* arg1);
extern ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal;  // ✅ extern = DECLARATION ONLY

typedef int (__thiscall* ApplyGraphicsManagerMain_t)(void* thisptr);
extern ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal;

extern void* g_ApplyGraphicsSettingsThis;
void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject);
void __stdcall LogApplyGraphicsSettingsThis(void* thisptr);
void TryApplyGraphicsManagerMain();
extern std::atomic<bool> g_TriggerApplyGraphicsSettings;

using IVisualTreatment_ResetFn = void(__thiscall*)(void* thisPtr);
// ✅ Header declaration only:
extern IVisualTreatment_ResetFn IVisualTreatment_Reset;
extern ID3DXEffect* g_LastReloadedFx;
