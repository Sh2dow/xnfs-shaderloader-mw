#include "Hooks.h"
#include "ShaderManager.h"
#include "dllmain.h"
#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include "includes/injector/injector.hpp"
#include <mutex>
#include "FxWrapper.h"

// -------------------- GLOBALS --------------------
LPDIRECT3DDEVICE9 ShaderManager::g_Device = nullptr;
D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

int g_ApplyDelayCounter = 0;
bool g_ApplyScheduled = false;


DWORD WINAPI DeferredHookThread(LPVOID)
{
    while (!ShaderManager::g_Device)
        Sleep(10);

    void** vtable = *(void***)ShaderManager::g_Device;
    if (vtable)
    {
        // Hook Present
        ShaderManager::RealPresent = (PresentFn)vtable[17];
        DWORD oldProtect;
        VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[17] = (void*)&HookedPresent;
        VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[Init] Hooked IDirect3DDevice9::Present (deferred)\n");

        // Hook Reset
        oReset = (Reset_t)vtable[16];
        VirtualProtect(&vtable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[16] = (void*)&hkReset;
        VirtualProtect(&vtable[16], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[Init] Hooked IDirect3DDevice9::Reset (deferred)\n");

    }
    return 0;
}

DWORD WINAPI HotkeyThread(LPVOID)
{
    while (true)
    {
        if (GetAsyncKeyState(VK_F2) & 1)
        {
            printf_s("[HotkeyThread] F2 pressed → Recompiling FX overrides...\n");

            // Now trigger recompilation
            RecompileAndReloadAll();

            // Schedule ApplyGraphicsSettings
            g_ApplyDelayCounter = 4;
            g_ApplyScheduled = true;
            g_TriggerApplyGraphicsSettings = true;
            g_ApplyGraphicsTriggerDelay = 2;
        }

        Sleep(100);
    }
}

// -------------------- DllMain --------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        printf_s("[Init] Shader override DLL loaded.\n");

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (d3dx)
        {
            void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
            RealCreateFromResource = (D3DXCreateEffectFromResourceAFn)addr;
            injector::MakeCALL(0x006C60D2, HookedCreateFromResource, true);

            ApplyGraphicsManagerMainOriginal = (decltype(ApplyGraphicsManagerMainOriginal))0x004F17F0;
            printf_s("[Init] ApplyGraphicsManagerMainOriginal set to 0x004F17F0\n");

            ApplyGraphicsSettingsOriginal = reinterpret_cast<ApplyGraphicsSettingsFn>(0x004EA0D0);
            injector::MakeCALL(0x004F186E, HookApplyGraphicsSettings, true);

            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HotkeyThread, nullptr, 0, nullptr);
            CreateThread(nullptr, 0, DeferredHookThread, nullptr, 0, nullptr);

            printf_s("[Init] Hooked D3DXCreateEffectFromResourceA\n");
        }
    }
    return TRUE;
}
