// NFSMW Shader Compiler & Loader (Trampoline Style)
#include "stdafx.h"
#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <string>
#include <vector>
#include <cstdio>
#include <MinHook.h>

#include "includes/injector/injector.hpp"

#define SafeRelease(p) { if(p) { (p)->Release(); (p)=nullptr; } }

// Maximum number of shader entries expected
static const unsigned int kMaxShaderEntries = 256;

// Last successfully loaded effect for fallback
static LPD3DXEFFECT g_LastValidEffect = nullptr;
static const char* g_ResolvedShaderName = nullptr;

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
    SYSTEMTIME st;
    GetSystemTime(&st);
    cusprintf("[Init] Shader hook fired at: %02d:%02d:%02d.%03d\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    unsigned int idx = CurrentShaderIndex;
    const uintptr_t tableBase = 0x008F9BE8;
    const char* rawPath = nullptr;

    cusprintf("[Hook] Shader index received: %u\n", idx);
    cusprintf("Table address: 0x%08X + %u * 4 = 0x%08X\n", tableBase, idx, tableBase + idx * 4);

    DWORD* table = (DWORD*)0x008F9BE8;
    cusprintf("[Dump] Shader table @ 0x%p\n", table);
    for (int i = 0; i < 64; i += 4) {
        cusprintf("[Table +0x%02X] 0x%08X\n", i, *(DWORD*)((BYTE*)table + i));
    }

    // If our trampoline captured the real name
    if (g_ResolvedShaderName != nullptr)
    {
        rawPath = g_ResolvedShaderName;
        cusprintf("[Hook] Resolved shader name via trampoline: %s\n", rawPath);
        g_ResolvedShaderName = nullptr;
    }
    else
    {
        cusprintf("[Hook] No resolved shader name, fallback.\n");
            return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    cusprintf("Raw shader name: %s\n", rawPath);
    
    if (idx >= kMaxShaderEntries)
    {
        cusprintf("Shader index out of bounds (%u), falling back.\n", idx);
        return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    char** entryPtr = reinterpret_cast<char**>(tableBase + idx * sizeof(char*));
    cusprintf("[Hook] entryPtr @ 0x%p -> 0x%p\n", entryPtr, entryPtr ? *entryPtr : nullptr);

    if (IsBadReadPtr((void*)tableBase, sizeof(char*) * kMaxShaderEntries)) {
        cusprintf("[Hook] ERROR: Shader table memory is not readable at 0x%08X\n", tableBase);
        cusprintf("Invalid shader pointer for index %u, falling back.\n", idx);
        return ReturnWithFallback(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
    }

    DWORD entry = *(DWORD*)entryPtr;
    cusprintf("[Resolved entry] entryPtr @ 0x%p -> 0x%08X\n", entryPtr, entry);

    cusprintf("Resolved rawPath: 0x%p\n", rawPath);
    cusprintf("Raw shader name: %s\n", rawPath);

    if (strncmp(rawPath, "IDI_", 4) == 0 || strncmp(rawPath, "IDI\\", 4) == 0)
    {
        cusprintf("Loading IDI_* resource: %s\n", rawPath);
        HRESULT result = D3DXCreateEffectFromResource(pDevice, hSrcModule, rawPath, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
        if (SUCCEEDED(result))
        {
            LPD3DXEFFECT effect = *ppEffect;

            // ✅ Pointer validity check
            if (!effect || IsBadReadPtr(effect, sizeof(void*)))
            {
                cusprintf("[Hook] WARNING: Invalid effect pointer assigned!\n");
            }

            // ... proceed with knownSlots matching and fallback assignment

            bool bound = false;

            struct {
                const char* name;
                uintptr_t address;
            } static const knownSlots[] = {
                { "IDI_VISUALTREATMENT_FX", 0x00982AF0 },
                { "IDI_OVERBRIGHT_FX",      0x00982B00 },
                { "IDI_SHADOW_MAP_MESH_FX", 0x00982B10 },
                { "IDI_WORLDDEPTH_FX",      0x00982B40 },
                { "IDI_TREEDEPTH_FX",       0x00982B30 },
                { "IDI_CARDEPTH_FX",        0x00982B20 },
                { "IDI_HDR_FX",             0x00982B50 },
            };

            for (auto& slot : knownSlots)
            {
                if (_stricmp(rawPath, slot.name) == 0)
                {
                    *(LPD3DXEFFECT*)slot.address = effect;
                    cusprintf("[Hook] Bound to %s at 0x%08X\n", rawPath, (unsigned int)slot.address);
                    bound = true;
                    break;
                }
            }

            if (!bound)
            {
                if (idx < kMaxShaderEntries)
                {
                    uintptr_t shaderObjectsBase = 0x0093DE78;
                    uintptr_t slotAddr = shaderObjectsBase + idx * 4;
                    *(LPD3DXEFFECT*)slotAddr = effect;
                    cusprintf("[Hook] Assigned fallback slot at ShaderObjects[%u] +0x00 = 0x%08X\n", idx, (unsigned int)slotAddr);
                }
                else
                {
                    cusprintf("[Hook] Shader index %u out of bounds for fallback assignment.\n", idx);
                }
            }

            *ppEffect = effect;
            return S_OK;
        }
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

// Helper function to print return address (must not use std::string)
void __stdcall PrintReturnAddress(DWORD retAddr)
{
    cusprintf("[Hook] Return address: 0x%08X\n", retAddr);
}

extern "C" const char* g_ResolvedShaderName;
const void* g_ResolvedNameTrial_2C = nullptr;
const void* g_ResolvedNameTrial_28 = nullptr;
const void* g_ResolvedNameTrial_30 = nullptr;


using D3DXCreateEffectFn = HRESULT(WINAPI*)(
    IDirect3DDevice9*, HMODULE, LPCSTR,
    const D3DXMACRO*, LPD3DXINCLUDE, DWORD,
    ID3DXEffectPool*, ID3DXEffect**, ID3DXBuffer**);

D3DXCreateEffectFn OriginalD3DXCreateEffect = nullptr;

HRESULT WINAPI Hooked_CreateEffectFromResource(
    IDirect3DDevice9* device,
    HMODULE hModule,
    LPCSTR shaderName,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    ID3DXEffectPool* pool,
    ID3DXEffect** outEffect,
    ID3DXBuffer** outErrors)
{
    g_ResolvedShaderName = shaderName;
    printf("[Hook] Shader = %s\n", shaderName ? shaderName : "(null)");
    printf("[Hook] CurrentShaderIndex = %u\n", CurrentShaderIndex);
    return OriginalD3DXCreateEffect(device, hModule, shaderName, defines, include, flags, pool, outEffect, outErrors);
}

// Wrapper used to safely replace call at 0x006C60D2
HRESULT WINAPI ShaderWrapper(
    IDirect3DDevice9* device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    ID3DXEffectPool* pool,
    ID3DXEffect** outEffect,
    ID3DXBuffer** outErrors)
{
    g_ResolvedShaderName = pResource;
    printf("[Wrapper] Shader = %s\n", pResource ? pResource : "(null)");

    static D3DXCreateEffectFn Real = nullptr;
    if (!Real)
    {
        HMODULE d3dx = GetModuleHandleA("d3dx9_26.dll");
        if (d3dx)
        {
            Real = (D3DXCreateEffectFn)GetProcAddress(d3dx, "D3DXCreateEffectFromResource");
            printf("[Wrapper] Using D3DX export from d3dx9_26.dll: 0x%p\n", Real);
        }
    }

    if (!Real)
    {
        printf("[Wrapper] ERROR: No valid D3DXCreateEffectFromResource found!\n");
        return E_FAIL;
    }

    return Real(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

int InitShaderHook()
{
    // Replace call to D3DXCreateEffectFromResource with our wrapper
    injector::MakeCALL(0x006C60D2, ShaderWrapper, true);

    cusprintf("[Hook] Shader call replaced at 0x006C60D2\n");
    return 0;
}

DWORD WINAPI InitThread(LPVOID)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    printf("[Init] Shader Hook Thread Started\n");
    return InitShaderHook();
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
