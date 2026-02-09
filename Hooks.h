#pragma once
#include <atomic>
#include <d3dx9effect.h>
#include <vector>

namespace std {
    template <>
    struct hash<std::pair<void*, int>> {
        size_t operator()(const std::pair<void*, int>& p) const {
            return hash<void*>()(p.first) ^ (hash<int>()(p.second) << 1);
        }
    };
}

// typedef void(__thiscall* ApplyGraphicsSettingsFn)(void* thisptr);
typedef void(__fastcall* ApplyGraphicsSettingsFn)(void* ecx, void* edx, void* arg1);
extern ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal;  // ✅ extern = DECLARATION ONLY

// NOTE: This is a game function (not a C++ member) that takes the manager "this" as a stack argument.
// Using __thiscall here can trigger __RTC_CheckEsp failures (0xC0000409) due to stack imbalance.
typedef int (__stdcall* ApplyGraphicsManagerMain_t)(void* thisptr);
extern ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal;

extern int g_ApplyGraphicsTriggerDelay;
extern void* g_ApplyGraphicsSettingsThis;
void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject);
extern std::atomic<bool> g_TriggerApplyGraphicsSettings;

using IVisualTreatment_ResetFn = void(__thiscall*)(void* thisPtr);
// ✅ Header declaration only:
extern IVisualTreatment_ResetFn IVisualTreatment_Reset;
extern ID3DXEffect* g_LastReloadedFx;
