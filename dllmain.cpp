
// NFSMW Shader Compiler & Loader (Trampoline Style)
#include "stdafx.h"
#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <string>
#include <vector>
#include <cstdio>
#include "includes/injector/injector.hpp"

#define SafeRelease(p) { if(p) { (p)->Release(); (p)=nullptr; } }

// Maximum number of shader entries expected
static const unsigned int kMaxShaderEntries = 512;

// Last successfully loaded effect for fallback
static LPD3DXEFFECT g_LastValidEffect = nullptr;

// Utility for logging
void cusprintf(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
}

// If override compile or resource load fails, fallback here
static HRESULT ReturnWithFallback(
    IDirect3DDevice9* pDevice,
    HMODULE hSrcModule,
    LPCSTR pSrcResource,
    const D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    ID3DXEffectPool* pPool,
    ID3DXEffect** ppEffect,
    ID3DXBuffer** ppCompilationErrors)
{
    if (g_LastValidEffect)
    {
        g_LastValidEffect->AddRef();
        *ppEffect = g_LastValidEffect;
        return S_OK;
    }
    // Call original loader
    return D3DXCreateEffectFromResource(
        pDevice, hSrcModule, pSrcResource,
        pDefines, pInclude, Flags,
        pPool, ppEffect, ppCompilationErrors
    );
}

static void ShowSafeShaderErrorMessage(ID3DXBuffer* pErrBuf)
{
    if (!pErrBuf || pErrBuf->GetBufferSize() == 0)
    {
        cusprintf("Shader error: empty or null error buffer.\n");
        return;
    }
    const char* msg = (const char*)pErrBuf->GetBufferPointer();
    cusprintf("Shader compile error:\n%s\n", msg);
}

// Hook stub placeholder (captures edx into CurrentShaderIndex then jumps)
unsigned int CurrentShaderIndex;

// Main hook function
HRESULT __stdcall D3DXCreateEffectFromResourceHook(
    IDirect3DDevice9* pDevice,
    HMODULE hSrcModule,
    LPCSTR pSrcResource,
    const D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    ID3DXEffectPool* pPool,
    ID3DXEffect** ppEffect,
    ID3DXBuffer** ppCompilationErrors)
{
    unsigned int idx = CurrentShaderIndex;
    cusprintf("[Hook] Shader index received: %u\n", idx);

    // 1) Out-of-bounds: fallback immediately
    if (idx >= kMaxShaderEntries)
    {
        cusprintf("Shader index out of bounds (%u), falling back.\n", idx);
        return ReturnWithFallback(
            pDevice, hSrcModule, pSrcResource,
            pDefines, pInclude, Flags,
            pPool, ppEffect, ppCompilationErrors
        );
    }

    // 2) Read the shader path pointer from the game table
    const uintptr_t tableBase = 0x008F9BE8;
    uintptr_t entryOffset = idx * sizeof(char*);
    char** entryPtr = reinterpret_cast<char**>(tableBase + entryOffset);
    if (!entryPtr || IsBadReadPtr(entryPtr, sizeof(char*)) || !*entryPtr)
    {
        cusprintf("Invalid shader pointer for index %u, falling back.\n", idx);
        return ReturnWithFallback(
            pDevice, hSrcModule, pSrcResource,
            pDefines, pInclude, Flags,
            pPool, ppEffect, ppCompilationErrors
        );
    }

    const char* rawPath = *entryPtr;
    cusprintf("Raw shader name: %s\n", rawPath);

    // 3) IDI_* resources: use game-provided compile
    if (strncmp(rawPath, "IDI_", 4) == 0 || strncmp(rawPath, "IDI\\", 4) == 0)
    {
        cusprintf("Loading IDI_* resource: %s\n", rawPath);
        return D3DXCreateEffectFromResource(
            pDevice, hSrcModule, rawPath,
            pDefines, pInclude, Flags,
            pPool, ppEffect, ppCompilationErrors
        );
    }

    // 4) Try fx/ override file
    char filenameBuf[256];
    strncpy(filenameBuf, rawPath, sizeof(filenameBuf)-1);
    char* dot = strrchr(filenameBuf, '.');
    if (dot) strcpy(dot, ".fx"); else strcat(filenameBuf, ".fx");
    std::string overridePath = std::string("fx/") + filenameBuf;

    FILE* f = fopen(overridePath.c_str(), "rb");
    if (f)
    {
        cusprintf("Overriding shader with file: %s\n", overridePath.c_str());
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> data(len);
        fread(data.data(), 1, len, f);
        fclose(f);

        ID3DXBuffer* errorBuf = nullptr;
        ID3DXEffect* effect = nullptr;
        HRESULT hr = D3DXCreateEffect(
            pDevice,
            data.data(), static_cast<UINT>(len),
            pDefines, pInclude, Flags,
            pPool, &effect, &errorBuf
        );
        if (FAILED(hr))
        {
            cusprintf("Override compile failed (0x%08X).\n", hr);
            ShowSafeShaderErrorMessage(errorBuf);
            SafeRelease(errorBuf);
            return ReturnWithFallback(
                pDevice, hSrcModule, pSrcResource,
                pDefines, pInclude, Flags,
                pPool, ppEffect, ppCompilationErrors
            );
        }
        // Success: cache and return
        g_LastValidEffect = effect;
        *ppEffect = effect;
        return S_OK;
    }

    // 5) Default fallback to original loader
    cusprintf("No override for %s, using original.\n", rawPath);
    return ReturnWithFallback(
        pDevice, hSrcModule, pSrcResource,
        pDefines, pInclude, Flags,
        pPool, ppEffect, ppCompilationErrors
    );
}

__declspec(naked) void ShaderHookStub()
{
    __asm {
        mov eax, edx               // edx holds shader index passed by compiler
        mov CurrentShaderIndex, eax
        jmp D3DXCreateEffectFromResourceHook
    }
}

// Initialization: insert hook
int InitShaderHook()
{
    // Make a trampoline to our stub
    injector::MakeCALL(0x006C60D2, ShaderHookStub);
    return 0;
}

// DLL entry
BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole(); freopen("CONOUT$","w", stdout);
        InitShaderHook();
    }
    return TRUE;
}
