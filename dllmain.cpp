#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include "includes/injector/injector.hpp"
#include <cstdio>
#include <string>
#include <vector>
typedef HRESULT(WINAPI* D3DXCreateEffectFn)(
    IDirect3DDevice9*,
    LPCVOID,
    UINT,
    const D3DXMACRO*,
    LPD3DXINCLUDE,
    DWORD,
    LPD3DXEFFECTPOOL,
    LPD3DXEFFECT*,
    LPD3DXBUFFER*
);

D3DXCreateEffectFn RealCreateEffect = nullptr;

// 🔧 CRC32 calculation
DWORD CRC32(const void* data, size_t length)
{
    const BYTE* p = (const BYTE*)data;
    DWORD crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i)
    {
        crc ^= p[i];
        for (int j = 0; j < 8; ++j)
            crc = (crc >> 1) ^ (0xEDB88320 & (-(int)(crc & 1)));
    }
    return ~crc;
}

class FXIncludeHandler : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID* ppData, UINT* pBytes)
    {
        char fullPath[MAX_PATH];
        snprintf(fullPath, sizeof(fullPath), "fx/%s", pFileName);

        FILE* f = fopen(fullPath, "rb");
        if (!f) return E_FAIL;

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        BYTE* data = new BYTE[len];
        fread(data, 1, len, f);
        fclose(f);

        *ppData = data;
        *pBytes = (UINT)len;
        return S_OK;
    }

    STDMETHOD(Close)(LPCVOID pData)
    {
        delete[] (BYTE*)pData;
        return S_OK;
    }
};

typedef HRESULT(WINAPI* D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*, LPD3DXINCLUDE, DWORD,
    LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*
);

D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

HRESULT WINAPI HookedCreateFromResource(
    LPDIRECT3DDEVICE9 device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors)
{
    printf("[Hook] D3DXCreateEffectFromResourceA called — pResource = %s\n", pResource);

    if (pResource && strcmp(pResource, "IDI_VISUALTREATMENT_FX") == 0)
    {
        FILE* f = fopen("fx/visualtreatment.fx", "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<char> data(len);
            fread(data.data(), 1, len, f);
            fclose(f);

            printf("[Hook] Overriding visualtreatment.fx via pResource\n");

            FXIncludeHandler includeHandler;

            HRESULT hr = D3DXCreateEffect(device, data.data(), (UINT)data.size(),
                defines, &includeHandler, flags, pool, outEffect, outErrors);

            if (FAILED(hr) || !outEffect || !*outEffect)
            {
                printf("[Hook] Shader failed to compile or create — falling back to original resource.\n");
                return RealCreateFromResource(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
            }

            return hr;
        }
        else
        {
            printf("[Hook] Failed to open fx/visualtreatment.fx — falling back\n");
        }
    }

    return RealCreateFromResource(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        printf("[Init] Shader Resource Hook DLL Loaded\n");

        HMODULE d3dx = GetModuleHandleA("d3dx9_26.dll");
        if (d3dx)
        {
            void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
            RealCreateFromResource = (D3DXCreateEffectFromResourceAFn)addr;
            injector::MakeCALL(0x006C60D2, HookedCreateFromResource, true);
            printf("[Init] Hooked D3DXCreateEffectFromResourceA\n");
        }
    }
    return TRUE;
}

