#include "Validators.h"
#include "Hooks.h"
#include <unordered_map>
#include <windows.h>
#define printf_s(...) asi_log::Log(__VA_ARGS__)

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
