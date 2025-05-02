// NFSMW Shader Compiler & Loader
// Compiles and/or loads shaders outside the executable
// Put HLSL effect .fx files in the fx folder in the game directory
// Or put compiled shader objects with resource names next to executable

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

char* ErrorString;

bool bConsoleExists(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return false;

	return true;
}

// custom print methods which include console attachment checks...
int __cdecl cusprintf(const char* Format, ...)
{
	va_list ArgList;
	int Result = 0;

	if (bConsoleExists())
	{
		__crt_va_start(ArgList, Format);
		Result = vprintf(Format, ArgList);
		__crt_va_end(ArgList);
	}

	return Result;
}

int __cdecl cus_puts(char* buf)
{
	if (bConsoleExists())
		return puts(buf);
	return 0;
}

bool CheckIfFileExists(const char* FileName)
{
	FILE *fin = fopen(FileName, "rb");
	if (fin == NULL)
	{
		char secondarypath[MAX_PATH];
		strcpy(secondarypath, "fx\\");
		strcat(secondarypath, FileName);
		fin = fopen(secondarypath, "rb");
		if (fin == NULL)
			return 0;
		strcpy((char*)FileName, secondarypath);
	}
	fclose(fin);
	return 1;
}

bool WriteFileFromMemory(const char* FileName, const void* buffer, long size)
{
	FILE *fout = fopen(FileName, "wb");
	if (fout == NULL)
		return 0;

	fwrite(buffer, 1, size, fout);

	fclose(fout);
	return 1;
}

HRESULT __stdcall D3DXCreateEffectFromResourceHook(LPDIRECT3DDEVICE9 pDevice,
	HMODULE           hSrcModule,
	LPCTSTR           pSrcResource,
	const D3DXMACRO         *pDefines,
	LPD3DXINCLUDE     pInclude,
	DWORD             Flags,
	LPD3DXEFFECTPOOL  pPool,
	LPD3DXEFFECT      *ppEffect,
	LPD3DXBUFFER      *ppCompilationErrors
)
{
    HRESULT result = E_FAIL;
    ID3DXEffectCompiler* pEffectCompiler = nullptr;
    ID3DXBuffer* pEffectBuffer = nullptr;
    ID3DXBuffer* pBuffer = nullptr;
    char FilenameBuf[MAX_PATH];

    // Resolve shader filename from table
    char* LastDot;
    char* FxFilePath = nullptr;
    __try {
        __asm {
            mov eax, CurrentShaderNum
            lea ecx, [eax + eax * 8]
            shl ecx, 4
            mov eax, dword ptr ds : [0x008F9B60 + ecx]
            mov FxFilePath, eax
        }
        if (!FxFilePath || IsBadStringPtrA(FxFilePath, 64)) {
            cusprintf("Shader filename pointer is invalid or null.\n");
            return E_FAIL;
        }

        strcpy(FilenameBuf, FxFilePath);
        LastDot = strrchr(FilenameBuf, '.');
        if (LastDot) {
            LastDot[1] = 'f';
            LastDot[2] = 'x';
            LastDot[3] = '\0';
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        cusprintf("Invalid shader index: %u\n", CurrentShaderNum);
        return E_FAIL;
    }

	// Try loading precompiled .fxo
	char fxoFilename[MAX_PATH];
	strcpy(fxoFilename, FilenameBuf);
	char* dot = strrchr(fxoFilename, '.');
	if (dot) {
		dot[1] = 'f'; dot[2] = 'x'; dot[3] = 'o'; dot[4] = '\0';

		if (GetFileAttributesA(fxoFilename) != INVALID_FILE_ATTRIBUTES) {
			cusprintf("Loading compiled shader %s\n", fxoFilename);
			result = D3DXCreateEffectFromFile(pDevice, fxoFilename, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
			if (SUCCEEDED(result))
				return result;
			else
				cusprintf("Failed to load compiled shader. HRESULT: %X\n", result);
		}
	}

    // Try compiling from .fx file
    if (GetFileAttributesA(FilenameBuf) != INVALID_FILE_ATTRIBUTES) {
        result = D3DXCreateEffectCompilerFromFile(FilenameBuf, nullptr, nullptr, 0, &pEffectCompiler, &pBuffer);
        if (SUCCEEDED(result)) {
            cusprintf("Compiling shader %s\n", FilenameBuf);
            result = pEffectCompiler->CompileEffect(0, &pEffectBuffer, &pBuffer);
            if (!SUCCEEDED(result)) {
            cusprintf("HRESULT: %X", result);
            if (pBuffer) {
                SIZE_T len = pBuffer->GetBufferSize();
                const BYTE* raw = (const BYTE*)pBuffer->GetBufferPointer();
                if (len >= 2 && raw[0] == 0xFF && raw[1] == 0xFE) {
                    const wchar_t* wstr = (const wchar_t*)raw;
                    MessageBoxW(NULL, wstr, L"Shader Compilation Error", MB_ICONERROR);
                    WideCharToMultiByte(CP_ACP, 0, wstr, -1, FilenameBuf, sizeof(FilenameBuf), NULL, NULL);
                    cus_puts(FilenameBuf);
                } else {
                    char* tempError = new char[len + 1];
                    memcpy(tempError, raw, len);
                    tempError[len] = '\0';
                    cus_puts(tempError);
                    MessageBoxA(NULL, tempError, "Shader Compilation Error", MB_ICONERROR);
                    delete[] tempError;
                }
            } else {
                MessageBoxA(NULL, "Unknown shader compile error — no buffer!", "Shader Error", MB_ICONERROR);
            }
            return result;
            }

            cusprintf("Compilation successful!\n");

            if (pEffectBuffer && pEffectBuffer->GetBufferPointer() && pEffectBuffer->GetBufferSize()) {
                if (pBuffer) {
                    pBuffer->Release();
                    pBuffer = nullptr;
                }
                result = D3DXCreateEffect(
                    pDevice,
                    pEffectBuffer->GetBufferPointer(),
                    pEffectBuffer->GetBufferSize(),
                    pDefines,
                    pInclude,
                    Flags,
                    pPool,
                    ppEffect,
                    &pBuffer
                );
                if (!SUCCEEDED(result)) {
                    cusprintf("Effect creation failed: HRESULT: %X\n", result);
                    if (pBuffer) cus_puts((char*)pBuffer->GetBufferPointer());
                }
                return result;
            } else {
                cusprintf("Invalid compiled shader buffer.\n");
                return E_FAIL;
            }
        } else {
            cusprintf("Error compiling shader: HRESULT: %X\n", result);
            if (pBuffer) cus_puts((char*)pBuffer->GetBufferPointer());
        }
    }

    // Fallback to file-based load
    if (GetFileAttributesA(pSrcResource) != INVALID_FILE_ATTRIBUTES) {
        return D3DXCreateEffectFromFile(pDevice, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    // Fallback to original resource load
    return D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
}

int Init()
{
	injector::MakeCALL(0x006C60D2, D3DXCreateEffectFromResourceHook, true);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		if (bConsoleExists())
		{
			freopen("CON", "w", stdout);
			freopen("CON", "w", stderr);
		}
		Init();
	}
	return TRUE;
}
