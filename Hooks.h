#pragma once

typedef void(__fastcall* ApplyGraphicsSettingsFn)(void*, void*, void*);
extern ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal;  // ✅ extern = DECLARATION ONLY

extern void* g_ApplyGraphicsSettingsThis;
void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject);
void __stdcall LogApplyGraphicsSettingsThis(void* thisptr);
void TryApplyGraphicsSettings();
