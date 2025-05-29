#include "Validators.h"

#include <stdio.h>
#include <unordered_map>
#include <windows.h>

#include "FxWrapper.h"


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
        void* vtable = *(void**)ptr;
        if (!vtable)
            return false;

        void* fn = *((void**)vtable); // vtable[0]
        return fn != nullptr && !IsBadCodePtr((FARPROC)fn);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

// bool IsValidThis(void* ptr)
// {
//     if (!ptr || (uintptr_t)ptr < 0x10000)
//         return false;
//
//     MEMORY_BASIC_INFORMATION mbi = {};
//     if (!VirtualQuery(ptr, &mbi, sizeof(mbi)))
//         return false;
//
//     // Accept readable and writable (RW or RWX)
//     if (!(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)))
//         return false;
//
//     // Ensure it's committed and not a guard or reserved region
//     if (!(mbi.State & MEM_COMMIT) || (mbi.Protect & PAGE_GUARD))
//         return false;
//
//     return true;
// }

// inline bool IsValidThis(void* ptr)
// {
//     return ptr != nullptr && IsBadReadPtr(ptr, 4) == FALSE;
// }

bool IsValidShaderPointer_SEH(ID3DXEffect* fx, void*** outVtable, DWORD* outProtect)
{
    __try
    {
        if (IsBadReadPtr(fx, sizeof(void*)))
            return false;

        void** vtable = *(void***)fx;
        if (!vtable || IsBadReadPtr(vtable, sizeof(void*)))
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

bool IsValidShaderPointer(FxWrapper* m_fx)
{
    ID3DXEffect* fx = m_fx->GetEffect();
    if (!fx || (uintptr_t)fx < 0x10000)
        return false;

    if (((uintptr_t)fx & 0xFFF00000) == 0x3F000000)
        return false;

    void** vtable = nullptr;
    DWORD prot = 0;
    if (!IsValidShaderPointer_SEH(fx, &vtable, &prot))
        return false;

    bool valid = !IsBadCodePtr((FARPROC)vtable[0]);

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

bool IsD3D9ExAvailable()
{
    OSVERSIONINFOEX osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);

    return (osvi.dwMajorVersion > 6) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 0); // Vista+
}
