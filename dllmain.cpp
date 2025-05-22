#include "Hooks.h"
#include "ShaderManager.h"
#include "dllmain.h"
#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <cstdio>
#include <MinHook.h>

#include "includes/injector/injector.hpp"
#include <mutex>

#if _DEBUG
#include "Log.h"
#define printf_s(...) asi_log::Log(__VA_ARGS__)
#endif

// -------------------- GLOBALS --------------------
LPDIRECT3DDEVICE9 ShaderManager::g_Device = nullptr;
D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

int g_ApplyDelayCounter = 0;
bool g_ApplyScheduled = false;

DWORD WINAPI HotkeyThread(LPVOID)
{
    while (true)
    {
        if (GetAsyncKeyState(VK_F2) & 1) // Press F2
        {
            printf_s("[HotkeyThread] F2 pressed → Recompiling FX overrides...\n");
            ReleaseAllRetainedShaders();
            RecompileAndReloadAll();

            // ✅ Trigger ApplyGraphicsManagerMain on next Present
            g_ApplyDelayCounter = 4; // delay for 4 Present()s
            g_ApplyScheduled = true;
        }

        // while (g_ApplyGraphicsSettingsThis == nullptr)
        {
            Sleep(50);
        }
    }
}

DWORD WINAPI DeferredHookThread(LPVOID)
{
    while (!ShaderManager::g_Device)
        Sleep(10);

    void** vtable = *(void***)ShaderManager::g_Device;
    if (vtable)
    {
        ShaderManager::RealPresent = (PresentFn)vtable[17];
        DWORD oldProtect;
        VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[17] = (void*)&HookedPresent;
        VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[Init] Hooked IDirect3DDevice9::Present (deferred)\n");
    }
    return 0;
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

