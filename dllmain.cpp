// NFSMW Shader Compiler & Loader (Trampoline Style)
#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "includes\injector\injector.hpp"
#include <D3D9.h>
#include <d3dx9effect.h>

unsigned int CurrentShaderNum;
ID3DXEffectCompiler* pEffectCompiler;
ID3DXBuffer* pBuffer, *pEffectBuffer;
char FilenameBuf[2048];

#define SafeRelease(p) { if(p) { (p)->Release(); (p)=nullptr; } }

static LPD3DXEFFECT g_LastValidEffect = nullptr;

int __cdecl cusprintf(const char* Format, ...)
{
    va_list ArgList;
    int Result;
    va_start(ArgList, Format);
    Result = vprintf(Format, ArgList);
    va_end(ArgList);
    return Result;
}

int __cdecl cus_puts(const char* buf)
{
    return puts(buf);
}

HRESULT __stdcall D3DXCreateEffectFromResourceHook(
    LPDIRECT3DDEVICE9 pDevice,
    HMODULE hSrcModule,
    LPCTSTR pSrcResource,
    CONST D3DXMACRO* pDefines,
    LPD3DXINCLUDE pInclude,
    DWORD Flags,
    LPD3DXEFFECTPOOL pPool,
    LPD3DXEFFECT* ppEffect,
    LPD3DXBUFFER* ppCompilationErrors)
{
    char* LastDot;
    char* FxFilePath = nullptr;

    DWORD shaderIndex;
    __asm mov shaderIndex, edx;
    CurrentShaderNum = shaderIndex;
    cusprintf("Shader index received: %u\n", shaderIndex);

    uintptr_t tableBase = 0x008F9BE8;
    uintptr_t entryOffset = shaderIndex * 0x36;

    cusprintf("Shader index received: %u\n", shaderIndex);
    cusprintf("Shader table base: 0x%X\n", tableBase);
    cusprintf("Shader entry offset: 0x%X\n", entryOffset);

    if (shaderIndex >= 64) {
        cusprintf("Shader index %u is out of range. Clamping to 0.\n", shaderIndex);
        shaderIndex = 0;
        CurrentShaderNum = 0;
        entryOffset = 0;
    }

    __try {
        FxFilePath = *(char**)(tableBase + entryOffset);
        if (!FxFilePath || IsBadReadPtr(FxFilePath, 4)) {
            throw 1;
        }
        cusprintf("Resolved shader path: %s\n", FxFilePath);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        char msg[128];
        sprintf(msg, "Invalid shader index or null pointer (idx=%u)", shaderIndex);
        MessageBoxA(NULL, msg, "Shader Loader Error", MB_ICONERROR);
        return E_FAIL;
    }

    if (strncmp(FxFilePath, "IDI_", 4) == 0 || strncmp(FxFilePath, "IDI\\", 4) == 0) {
        cusprintf("Skipping IDI_* shader: %s\n", FxFilePath);
        if (g_LastValidEffect && !IsBadReadPtr(g_LastValidEffect, sizeof(ID3DXEffect))) {
            *ppEffect = g_LastValidEffect;
            g_LastValidEffect->AddRef();
            return S_OK;
        }
        return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    strcpy(FilenameBuf, FxFilePath);
    LastDot = strrchr(FilenameBuf, '.');
    if (LastDot)
    {
        LastDot[1] = 'f';
        LastDot[2] = 'x';
        LastDot[3] = '\0';
    }

    cusprintf("Final FX path: %s\n", FilenameBuf);

    FILE* fcheck = fopen(FilenameBuf, "rb");
    if (!fcheck)
    {
        cusprintf("FX file not found: skipping shader load. Falling back to original resource.\n");
        if (g_LastValidEffect && !IsBadReadPtr(g_LastValidEffect, sizeof(ID3DXEffect))) {
            *ppEffect = g_LastValidEffect;
            g_LastValidEffect->AddRef();
            return S_OK;
        }
        return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }
    fseek(fcheck, 0, SEEK_END);
    long size = ftell(fcheck);
    fclose(fcheck);
    cusprintf("FX file size: %ld bytes\n", size);

    DWORD compileFlags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
    HRESULT result = D3DXCreateEffectCompilerFromFile(FilenameBuf, NULL, NULL, compileFlags, &pEffectCompiler, &pBuffer);
    cusprintf("D3DXCreateEffectCompilerFromFile returned: %X\n", result);

    if (FAILED(result)) {
        MessageBoxA(NULL, "Failed to create effect compiler", "Shader Compilation Error", MB_ICONERROR);
        if (pBuffer)
            cus_puts((char*)pBuffer->GetBufferPointer());
        SafeRelease(pEffectCompiler);
        SafeRelease(pBuffer);
        if (g_LastValidEffect && !IsBadReadPtr(g_LastValidEffect, sizeof(ID3DXEffect))) {
            *ppEffect = g_LastValidEffect;
            g_LastValidEffect->AddRef();
            return S_OK;
        }
        return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    cusprintf("Compiling shader %s\n", FilenameBuf);
    result = pEffectCompiler->CompileEffect(0, &pEffectBuffer, &pBuffer);
    if (FAILED(result)) {
        MessageBoxA(NULL, "Failed to compile effect", "Shader Compilation Error", MB_ICONERROR);
        if (pBuffer)
            cus_puts((char*)pBuffer->GetBufferPointer());
        SafeRelease(pEffectCompiler);
        SafeRelease(pBuffer);
        if (g_LastValidEffect && !IsBadReadPtr(g_LastValidEffect, sizeof(ID3DXEffect))) {
            *ppEffect = g_LastValidEffect;
            g_LastValidEffect->AddRef();
            return S_OK;
        }
        return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    SafeRelease(pBuffer);
    result = D3DXCreateEffect(pDevice, pEffectBuffer->GetBufferPointer(), pEffectBuffer->GetBufferSize(), pDefines, pInclude, Flags, pPool, ppEffect, &pBuffer);
    if (FAILED(result)) {
        MessageBoxA(NULL, "Failed to create effect from compiled buffer", "Shader Compilation Error", MB_ICONERROR);
        if (pBuffer)
            cus_puts((char*)pBuffer->GetBufferPointer());
        SafeRelease(pEffectCompiler);
        SafeRelease(pBuffer);
        if (g_LastValidEffect && !IsBadReadPtr(g_LastValidEffect, sizeof(ID3DXEffect))) {
            *ppEffect = g_LastValidEffect;
            g_LastValidEffect->AddRef();
            return S_OK;
        }
        return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    g_LastValidEffect = *ppEffect;
    g_LastValidEffect->AddRef();
    cusprintf("Shader compiled and created successfully.\n");
    SafeRelease(pEffectCompiler);
    SafeRelease(pBuffer);
    return result;
}

__declspec(naked) void ShaderHookStub()
{
    __asm {
        mov CurrentShaderNum, edx
        jmp D3DXCreateEffectFromResourceHook
    }
}

int Init()
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
        Init();
    }
    return TRUE;
}
