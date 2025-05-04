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
int __cdecl cusprintf(const char* Format, ...)
{
    va_list ArgList;
    int Result;
    va_start(ArgList, Format);
    Result = vprintf(Format, ArgList);
    va_end(ArgList);
    return Result;
}

void DetectEffectBinding(LPD3DXEFFECT fx)
{
    for (uintptr_t addr = 0x00980000; addr < 0x00990000; addr += 4)
    {
        if (*(LPD3DXEFFECT*)addr == fx)
        {
            cusprintf("[Detect] Effect pointer %p stored at 0x%08X\n", fx, (unsigned int)addr);
        }
    }
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
    return D3DXCreateEffectFromResource(
        pDevice, hSrcModule, pSrcResource,
        pDefines, pInclude, Flags,
        pPool, ppEffect, ppCompilationErrors
    );
}

static void ShowSafeShaderErrorMessage(ID3DXBuffer* pErrBuf)
{
    if (!pErrBuf || !pErrBuf->GetBufferPointer() || pErrBuf->GetBufferSize() == 0)
    {
        cusprintf("Shader error: empty or null error buffer.");
        return;
    }

    const BYTE* raw = (const BYTE*)pErrBuf->GetBufferPointer();
    SIZE_T rawLen = pErrBuf->GetBufferSize();

    printf("Shader error raw buffer (%zu bytes):\n", rawLen);
    for (size_t i = 0; i < rawLen; ++i)
    {
        printf("%02X ", raw[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    cusprintf("Shader error: see console for raw dump.");
}

unsigned int CurrentShaderIndex;

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
    SYSTEMTIME st;
    GetSystemTime(&st);
    cusprintf("[Init] Shader hook fired at: %02d:%02d:%02d.%03d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    unsigned int idx = CurrentShaderIndex;
    cusprintf("[Hook] Shader index received: %u\n", idx);
    cusprintf("Table address: 0x%08X + %u * 4 = 0x%08X\n", 0x008F9BE8, idx, 0x008F9BE8 + idx * 4);

    if (!pSrcResource || IsBadReadPtr(pSrcResource, 4)) {
        cusprintf("[Hook] Invalid pSrcResource!\n");
        return E_FAIL;
    }

    if (idx >= kMaxShaderEntries)
    {
        cusprintf("Shader index out of bounds (%u), falling back.\n", idx);
        return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    const uintptr_t tableBase = 0x008F9BE8;
    uintptr_t entryOffset = idx * sizeof(char*);

    if ((tableBase + entryOffset) > 0x00900000) {
        cusprintf("[Hook] Calculated entry pointer is out-of-bounds!\n");
        return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    char** entryPtr = reinterpret_cast<char**>(tableBase + entryOffset);
    if (!entryPtr || IsBadReadPtr(entryPtr, sizeof(char*)) || !*entryPtr)
    {
        cusprintf("Invalid shader pointer for index %u, falling back.\n", idx);
        return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    const char* rawPath = *entryPtr;
    cusprintf("Resolved entryPtr: 0x%p\n", entryPtr);
    cusprintf("Raw shader name: %s\n", rawPath);

    if (strncmp(rawPath, "IDI_", 4) == 0 || strncmp(rawPath, "IDI\\", 4) == 0)
    {
        cusprintf("Loading IDI_* resource: %s\n", rawPath);
        HRESULT result = D3DXCreateEffectFromResource(pDevice, hSrcModule, rawPath, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
        if (SUCCEEDED(result))
        {
            LPD3DXEFFECT effect = *ppEffect;
            DetectEffectBinding(effect); // log only
        }
        return result;
    }

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
            return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
        }
        g_LastValidEffect = effect;
        *ppEffect = effect;
        return S_OK;
    }

    cusprintf("No override for %s, using original.\n", rawPath);
    return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
}

__declspec(naked) void ShaderHookStub()
{
    __asm {
        push eax
        mov eax, edx
        mov CurrentShaderIndex, eax
        pop eax
        jmp D3DXCreateEffectFromResourceHook
    }
}

int InitShaderHook()
{
    injector::MakeCALL(0x006C60D2, ShaderHookStub, true);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        InitShaderHook();
    }
    return TRUE;
}
