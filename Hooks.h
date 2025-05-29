#pragma once
#include <atomic>
#include <vector>
#include "FxWrapper.h"

#define FX(x) ((FxWrapper*)(x))

#if _DEBUG
#include "Log.h"
#define printf_s(...) asi_log::Log(__VA_ARGS__)
#endif

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

typedef int (__thiscall* ApplyGraphicsManagerMain_t)(void* thisptr);
extern ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal;

extern bool g_WaitingForReset;
extern int g_ApplyGraphicsTriggerDelay;
extern void* g_ApplyGraphicsSettingsThis;
void __fastcall HookApplyGraphicsSettings(void* manager, void*, void* vtObject);
extern std::atomic<bool> g_TriggerApplyGraphicsSettings;

using IVisualTreatment_ResetFn = void(__thiscall*)(void* thisPtr);
// ✅ Header declaration only:
extern IVisualTreatment_ResetFn IVisualTreatment_Reset;
extern FxWrapper* g_LastReloadedFx;
