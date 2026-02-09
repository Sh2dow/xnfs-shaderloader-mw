#include "Validators.h"
#include "Hooks.h"
#include <unordered_map>
#include <windows.h>
// Use the shared logging macro from Hooks.h (keeps build consistent).

inline bool IsValidCodePtr(void* ptr)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (!ptr) return false;

    if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
    {
        DWORD protect = mbi.Protect;
        bool isExecutable =
            (protect & PAGE_EXECUTE) ||
            (protect & PAGE_EXECUTE_READ) ||
            (protect & PAGE_EXECUTE_READWRITE) ||
            (protect & PAGE_EXECUTE_WRITECOPY);

        return (mbi.State == MEM_COMMIT) && isExecutable;
    }

    return false;
}

bool IsValidThis(void* ptr)
{
    if (!ptr)
        return false;

    __try
    {
        if (IsBadReadPtr(ptr, sizeof(void*)))
            return false;

        void* vtable = *(void**)ptr;
        if (!vtable)
            return false;

        if (IsBadReadPtr(vtable, sizeof(void*)))
            return false;

        void* fn = *((void**)vtable); // vtable[0]
        return fn != nullptr && IsValidCodePtr(fn);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsValidShaderPointer_SEH(ID3DXEffect* fx, void*** outVtable, DWORD* outProtect)
{
    __try
    {
        if (IsBadReadPtr(fx, sizeof(void*)))
            return false;

        void** vtable = *(void***)fx;
        if (IsBadReadPtr(vtable, sizeof(void*)))
            return false;

        MEMORY_BASIC_INFORMATION mbi = {};
        if (!VirtualQuery(vtable, &mbi, sizeof(mbi)))
            return false;

        *outVtable = vtable;
        *outProtect = mbi.Protect;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx || (uintptr_t)fx < 0x10000)
        return false;

    if (((uintptr_t)fx & 0xFFF00000) == 0x3F000000)
        return false;

    void** vtable = nullptr;
    DWORD prot = 0;
    if (!IsValidShaderPointer_SEH(fx, &vtable, &prot))
        return false;

    MEMORY_BASIC_INFORMATION mbiFx = {};
    if (!VirtualQuery(fx, &mbiFx, sizeof(mbiFx)) || mbiFx.State != MEM_COMMIT)
        return false;

    auto isInModule = [](void* ptr, HMODULE mod) -> bool
    {
        if (!mod || !ptr)
            return false;
        MEMORY_BASIC_INFORMATION mbi = {};
        if (!VirtualQuery(ptr, &mbi, sizeof(mbi)))
            return false;
        return mbi.AllocationBase == mod;
    };

    HMODULE d3dxMod = GetModuleHandleA("d3dx9_43.dll");
    bool valid = false;
    if (d3dxMod)
    {
        if (isInModule(vtable, d3dxMod) && isInModule(vtable[0], d3dxMod))
        {
            valid = IsValidCodePtr(vtable[0]);
        }
        else
        {
            // Some builds place the vtable outside d3dx; fall back to executable check.
            valid = IsValidCodePtr(vtable[0]) && !IsBadCodePtr((FARPROC)vtable[0]);
        }
    }
    else
    {
        valid = IsValidCodePtr(vtable[0]) && !IsBadCodePtr((FARPROC)vtable[0]);
    }

    static std::unordered_map<uintptr_t, int> invalidVtableHits;
    if (!valid)
    {
        uintptr_t vtblAddr = reinterpret_cast<uintptr_t>(vtable);
        int& count = invalidVtableHits[vtblAddr];
        if (count++ < 1)
            printf_s("⚠️ Invalid vtable: fx=%p vtable=%p prot=0x%08X\n", fx, vtable, prot);
    }

    return valid;
}

bool IsValidEffectObject(ID3DXEffect* fx)
{
    if (!fx || reinterpret_cast<uintptr_t>(fx) < 0x10000)
        return false;

    __try
    {
        D3DXEFFECT_DESC desc = {};
        HRESULT hr = fx->GetDesc(&desc);
        if (FAILED(hr))
            return false;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsLikelyEffectPointer(ID3DXEffect* fx)
{
    if (!fx || reinterpret_cast<uintptr_t>(fx) < 0x10000)
        return false;

    __try
    {
        if (IsBadReadPtr(fx, sizeof(void*)))
            return false;
        void** vtable = *(void***)fx;
        if (!vtable || IsBadReadPtr(vtable, sizeof(void*)))
            return false;
        void* fn = vtable[0];
        MEMORY_BASIC_INFORMATION mbi1 = {};
        MEMORY_BASIC_INFORMATION mbi2 = {};
        if (!VirtualQuery(vtable, &mbi1, sizeof(mbi1)))
            return false;
        if (!VirtualQuery(fn, &mbi2, sizeof(mbi2)))
            return false;
        if (mbi1.State != MEM_COMMIT || mbi2.State != MEM_COMMIT)
            return false;
        return IsValidCodePtr(fn);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool ProbeEffectHandles(ID3DXEffect* fx)
{
    if (!fx || reinterpret_cast<uintptr_t>(fx) < 0x10000)
        return false;

    __try
    {
        D3DXHANDLE h = fx->GetParameterByName(nullptr, "DIFFUSEMAP_TEXTURE");
        if (h)
            return true;
        h = fx->GetParameterByName(nullptr, "DiffuseMap");
        if (h)
            return true;
        D3DXEFFECT_DESC desc = {};
        if (SUCCEEDED(fx->GetDesc(&desc)) && desc.Parameters > 0)
            return true;
        return false;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool IsD3D9ExAvailable()
{
    OSVERSIONINFOEX osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);

    return (osvi.dwMajorVersion > 6) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 0); // Vista+
}

// NFSMW_XenonEffects imports

bool IsSafeToReload()
{
    int gameflow = *(int*)GAMEFLOWSTATUS_ADDR;
    void* nis = *(void**)NISINSTANCE_ADDR;
    return gameflow >= 3 && nis == nullptr;
}

inline bool IsInCutsceneOrFrontend()
{
    int flow = *(int*)GAMEFLOWSTATUS_ADDR;
    void* nis = *(void**)NISINSTANCE_ADDR;
    return (flow < 3 || nis != nullptr);
}
