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

bool bConsoleExists(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
}

int __cdecl cusprintf(const char* Format, ...)
{
    va_list ArgList;
    int Result = 0;
    if (bConsoleExists()) {
        __crt_va_start(ArgList, Format);
        Result = vprintf(Format, ArgList);
        __crt_va_end(ArgList);
    }
    return Result;
}

int __cdecl cus_puts(const char* buf)
{
    return bConsoleExists() ? puts(buf) : 0;
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

    __try {
        if (shaderIndex >= 64)
            throw 1;

        FxFilePath = *(char**)(tableBase + entryOffset);
        if (!FxFilePath || IsBadReadPtr(FxFilePath, 4))
            throw 2;

        cusprintf("Resolved shader path: %s\n", FxFilePath);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        char msg[128];
        sprintf(msg, "Invalid shader index or null pointer (idx=%u)", shaderIndex);
        MessageBoxA(NULL, msg, "Shader Loader Error", MB_ICONERROR);
        return E_FAIL;
    }

    strncpy(FilenameBuf, FxFilePath, sizeof(FilenameBuf) - 1);
    FilenameBuf[sizeof(FilenameBuf) - 1] = '\0';

    LastDot = strrchr(FilenameBuf, '.');
    if (LastDot)
    {
        LastDot[1] = 'f';
        LastDot[2] = 'x';
        LastDot[3] = '\0';
    }

    cusprintf("Final FX path: %s\n", FilenameBuf);
    DWORD fileAttr = GetFileAttributesA(FilenameBuf);
    cusprintf("File exists: %s\n", (fileAttr != INVALID_FILE_ATTRIBUTES) ? "YES" : "NO");

    if (fileAttr != INVALID_FILE_ATTRIBUTES)
    {
        HRESULT result = D3DXCreateEffectCompilerFromFile(FilenameBuf, NULL, NULL, 0, &pEffectCompiler, &pBuffer);
        if (SUCCEEDED(result))
        {
            cusprintf("Compiling shader %s\n", FilenameBuf);
            result = pEffectCompiler->CompileEffect(0, &pEffectBuffer, &pBuffer);
            if (!SUCCEEDED(result))
            {
                if (pBuffer)
                {
                    char* err = (char*)pBuffer->GetBufferPointer();
                    MessageBoxA(NULL, err, "Shader Compilation Error", MB_ICONERROR);
                    cus_puts(err);
                }
                cusprintf("HRESULT: %X\n", result);
                return result;
            }
            cusprintf("Compilation successful!\n");
            result = D3DXCreateEffect(pDevice, pEffectBuffer->GetBufferPointer(), pEffectBuffer->GetBufferSize(), pDefines, pInclude, Flags, pPool, ppEffect, &pBuffer);
            if (!SUCCEEDED(result))
            {
                cusprintf("Effect creation failed: HRESULT: %X\n", result);
                if (pBuffer)
                {
                    char* err = (char*)pBuffer->GetBufferPointer();
                    cus_puts(err);
                }
            }
            return result;
        }
        if (pBuffer)
        {
            char* err = (char*)pBuffer->GetBufferPointer();
            MessageBoxA(NULL, err, "Shader Compilation Error", MB_ICONERROR);
            cus_puts(err);
        }
        cusprintf("Error compiling shader: HRESULT: %X\n", result);
    }

    if (GetFileAttributesA(pSrcResource) != INVALID_FILE_ATTRIBUTES)
    {
        return D3DXCreateEffectFromFile(pDevice, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
}

__declspec(naked) void ShaderHookStub()
{
    __asm {
        mov CurrentShaderNum, edx
        push edx
        call D3DXCreateEffectFromResourceHook
        ret 0x24
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
        if (bConsoleExists()) {
            freopen("CON", "w", stdout);
            freopen("CON", "w", stderr);
        }
        Init();
    }
    return TRUE;
}
