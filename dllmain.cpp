#include "dllmain.h"
#include <algorithm>
#include <atomic>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9effect.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include "includes/injector/injector.hpp"
#include <d3dx9effect.h>
#include <iostream>
#include <MinHook.h>
#include <mutex>
#include <unordered_set>

#if _DEBUG
#include "Log.h"
#define printf_s(...) asi_log::Log(__VA_ARGS__)
#endif

// -------------------- GLOBALS --------------------

LPDIRECT3DDEVICE9 g_Device = nullptr;

std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
std::unordered_set<std::string> g_FxOverrides;
static std::atomic<int> g_HookCallCount{0};
std::atomic<bool> g_TriggerApplyGraphicsSettings = false;
static std::unordered_set<std::string> g_CreatedShaderNames;
bool g_TryReloadVisualTreatment = false;
bool g_TryReloadVisualTreatmentTriggered = false;
bool g_TryReloadViaSub6D7500 = false;
bool g_PresentHooked = false;
bool g_FrontendReady = false;
void* actionQueue = nullptr;

ID3DXEffect* g_LastVisualTreatmentFx = nullptr;
static std::unordered_map<std::string, ID3DXEffect*> g_LastCompiledShaders;
static std::unordered_map<std::string, int> g_LastShaderIndices;

struct QueuedPatch
{
    int slot;
    ID3DXEffect* fx;
    int framesRemaining = 2; // safe delay
};

std::vector<QueuedPatch> g_PendingPatches;

std::mutex g_PatchMutex;

std::atomic<bool> g_PausePresent{false};
std::atomic<bool> g_PresentIsWaiting{false};

typedef void (__cdecl*ApplyGraphicsSettingsFn)();
ApplyGraphicsSettingsFn ApplyGraphicsSettings = (ApplyGraphicsSettingsFn)0x004EA0B0;

typedef void (__cdecl*ApplySettingsFn)();
ApplySettingsFn ApplySettings = (ApplySettingsFn)0x004EA0B0;

typedef void* (__cdecl*GetActionQueueFn)(int, int, int);
static GetActionQueueFn GetActionQueue = (GetActionQueueFn)0x004E42A0;

typedef void (__thiscall*EnableFn)(void*, bool);

void ActionQueue_Enable(void* queue, bool enable)
{
    auto fn = (void (__thiscall*)(void*, bool))0x00644060;
    fn(queue, enable);
}

// Declare both trampolines at global scope
typedef int (__fastcall*Sub6C6080Fn)(void*);
Sub6C6080Fn RealSub6C6080 = nullptr;

typedef void (*Sub6D7500Fn)();
Sub6D7500Fn RealSub6D7500 = nullptr;

typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);
PresentFn RealPresent = nullptr;

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*, LPD3DXINCLUDE,
    DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

// -------------------- HELPERS --------------------

// Converts string to uppercase
std::string ToUpper(const std::string& str)
{
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i)
        result[i] = (char)toupper(result[i]);
    return result;
}

bool IsValidCString(const char* str)
{
    __try
    {
        if (!str || IsBadStringPtrA(str, 512))
            return false;
        volatile char c = *str; // forces a read
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

// -------------------- INCLUDE HANDLER --------------------

class FXIncludeHandler : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE, LPCSTR fileName, LPCVOID, LPCVOID* ppData, UINT* pBytes) override
    {
        std::string fullPath = "fx/" + std::string(fileName);
        printf_s("[Include] Trying to open: %s\n", fullPath.c_str());

        FILE* f = fopen(fullPath.c_str(), "rb");
        if (!f)
        {
            printf_s("[Include] Failed to open: %s\n", fullPath.c_str());
            return E_FAIL;
        }

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);
        BYTE* buf = new BYTE[len];
        fread(buf, 1, len, f);
        fclose(f);

        *ppData = buf;
        *pBytes = (UINT)len;
        printf_s("[Include] Opened: %s\n", fullPath.c_str());
        return S_OK;
    }

    STDMETHOD(Close)(LPCVOID pData) override
    {
        delete[] (BYTE*)pData;
        return S_OK;
    }
};

bool CompileAndDumpShader(const std::string& key, const std::string& fxPath)
{
    printf_s("[Compile] Compiling FX effect: %s => %s\n", fxPath.c_str(), key.c_str());

    ID3DXEffectCompiler* pCompiler = nullptr;
    ID3DXBuffer* pErrors = nullptr;

    HRESULT hr = D3DXCreateEffectCompilerFromFile(
        fxPath.c_str(),
        nullptr, // macros
        nullptr, // include
        0, // flags
        &pCompiler,
        &pErrors
    );

    if (FAILED(hr))
    {
        if (pErrors)
        {
            printf_s("[Compile] Error: %s\n", (char*)pErrors->GetBufferPointer());
            pErrors->Release();
        }
        return false;
    }

    ID3DXBuffer* pCompiledEffect = nullptr;
    hr = pCompiler->CompileEffect(0, &pCompiledEffect, &pErrors);
    pCompiler->Release();

    if (FAILED(hr))
    {
        if (pErrors)
        {
            printf_s("[Compile] Error: %s\n", (char*)pErrors->GetBufferPointer());
            pErrors->Release();
        }
        return false;
    }

    // Write to game root folder IDI_*.FX
    std::string outPath = key;
    FILE* f = fopen(outPath.c_str(), "wb");
    if (!f)
    {
        printf_s("[Compile] Failed to open output: %s\n", outPath.c_str());
        pCompiledEffect->Release();
        return false;
    }

    fwrite(pCompiledEffect->GetBufferPointer(), 1, pCompiledEffect->GetBufferSize(), f);
    fclose(f);
    pCompiledEffect->Release();

    printf_s("[Compile] Written compiled FX: %s\n", outPath.c_str());
    return true;
}

// -------------------- SHADER OVERRIDE LOADER --------------------
void LoadShaderOverrides()
{
    DWORD attrs = GetFileAttributesA("fx");
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        printf_s("[Error] fx/ folder does not exist or is inaccessible.\n");
        return;
    }

    printf_s("[Init] fx/ folder found, scanning for shaders...\n");

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("fx\\*.fx", &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf_s("[Init] No .fx files found in fx/ folder.\n");
        return;
    }

    do
    {
        std::string fileName = findData.cFileName;
        std::string name = fileName.substr(0, fileName.find_last_of('.'));
        std::replace(name.begin(), name.end(), '-', '_'); // normalize dashes
        std::string key = "IDI_" + ToUpper(name) + "_FX";
        std::string fullPath = "fx/" + fileName;
        g_FxOverrides.insert(key); // ✅ REQUIRED for HookedCreateFromResource

        CompileAndDumpShader(key, fullPath);

        printf_s("[Init] Compiling and caching %s as %s\n", fileName.c_str(), key.c_str());

        FILE* f = fopen(fullPath.c_str(), "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> data(len);
        fread(data.data(), 1, len, f);
        fclose(f);

        ID3DXEffect* fx = nullptr;
        ID3DXBuffer* errors = nullptr;
        FXIncludeHandler includeHandler;

        HRESULT hr = D3DXCreateEffect(g_Device, data.data(), (UINT)len, nullptr, &includeHandler,
                                      D3DXSHADER_DEBUG, nullptr, &fx, &errors);

        if (FAILED(hr))
        {
            printf_s("[Init] Failed to compile %s\n", fileName.c_str());
            if (errors)
            {
                printf_s("[Init] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        g_ShaderOverridePaths[key] = fullPath;
        g_ShaderBuffers[key] = data; // stores raw HLSL code
    }
    while (FindNextFileA(hFind, &findData));

    printf_s("[Debug] g_FxOverrides contains:\n");
    for (const auto& k : g_FxOverrides)
        printf_s("  - %s\n", k.c_str());

    FindClose(hFind);
}

// -------------------- HOOK HANDLER --------------------
bool IsValidShaderPointer(ID3DXEffect* fx)
{
    if (!fx) return false;
    void** vtable = *(void***)fx;
    return vtable && !IsBadCodePtr((FARPROC)vtable[0]) && !IsBadCodePtr((FARPROC)vtable[1]);
}

void ApplyQueuedShaderPatches()
{
    std::lock_guard<std::mutex> lock(g_PatchMutex);
    static ID3DXEffect** table = (ID3DXEffect**)0x0093DE78;

    std::vector<QueuedPatch> stillPending;

    for (auto& patch : g_PendingPatches)
    {
        if (--patch.framesRemaining > 0)
        {
            stillPending.push_back(patch);
            continue;
        }

        if (!IsValidShaderPointer(patch.fx))
        {
            printf_s("[Patch] ❌ fx invalid for slot %d, skipping\n", patch.slot);
            continue;
        }

        if (table[patch.slot] && IsValidShaderPointer(table[patch.slot]))
        {
            printf_s("[Patch] 🔁 Released old fx in slot %d (%p)\n", patch.slot, table[patch.slot]);
            table[patch.slot]->Release();
        }

        // patch.fx->AddRef(); // assign to table
        table[patch.slot] = patch.fx;

        printf_s("[Patch] ✅ Wrote shader to slot %d (fx=%p)\n", patch.slot, patch.fx);
    }

    g_PendingPatches = std::move(stillPending);
}

void* SafeGetActionQueue()
{
    void* ptr = nullptr;
    __try
    {
        ptr = GetActionQueue(1, 0, 0);
        if (ptr)
            printf_s("[XNFS-ShaderLoader-MW] [Debug] SafeGetActionQueue() = %p\n", ptr);
        else
            printf_s("[XNFS-ShaderLoader-MW] [Debug] GetActionQueue returned null\n");
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf_s("[XNFS-ShaderLoader-MW] ❌ Exception in SafeGetActionQueue()\n");
    }
    return ptr;
}



// MUST NOT use STL types in this function!
const char* SafeGetShaderTableEntryChecked(int index)
{
    __try
    {
        uintptr_t base = 0x008F9B60;
        uintptr_t offset = index * 0x10;
        char* ptr = *(char**)(base + offset);
        if (!ptr || (uintptr_t)ptr < 0x1000) // avoid nullptr or low memory
            return nullptr;

        volatile char c = ptr[0]; // probe safely
        return ptr;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return nullptr;
    }
}

std::vector<int> LookupShaderSlotsFromResource(const std::string& resourceName)
{
    std::vector<int> results;
    std::string base;
    if (resourceName.find("IDI_") == 0 && resourceName.rfind("_FX") == resourceName.length() - 3)
        base = resourceName.substr(4, resourceName.length() - 7); // Strip IDI_ and _FX
    else
        return results;

    for (int i = 0; i < 64; ++i)
    {
        const char* entry = SafeGetShaderTableEntryChecked(i);
        if (!entry || IsBadStringPtrA(entry, 128))
            continue;

        std::string name(entry);
        size_t dot = name.find('.');
        if (dot != std::string::npos)
            name = name.substr(0, dot);

        std::string a = base, b = name;
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);

        if (a == b)
            results.push_back(i);
    }

    return results;
}

int __fastcall HookedSub6C6080(void* thisptr) 
{
    int shaderIndex = *(int*)((char*)thisptr + 0x4);
    printf_s("[XNFS-ShaderLoader-MW] [Hook] sub_6C6080(this=%p) — shaderIndex = %d\n", thisptr, shaderIndex);

    if (shaderIndex >= 0 && shaderIndex < 64)
    {
        const char** table = (const char**)0x008F9B60;
        const char* name = table[shaderIndex];
        if (IsValidCString(name))
            printf_s(" └─ Resource: %s\n", name);
        else
            printf_s(" └─ Invalid resource string pointer\n");
    }

    void* fx = *(void**)((char*)thisptr + 0x48);
    if (IsValidShaderPointer((ID3DXEffect*)fx))
    {
        printf_s(" └─ fx @+0x48 = %p [valid]\n", fx);

        ID3DXEffect* effect = (ID3DXEffect*)fx;
        D3DXHANDLE tech = effect->GetCurrentTechnique();
        if (tech)
        {
            D3DXTECHNIQUE_DESC desc;
            if (SUCCEEDED(effect->GetTechniqueDesc(tech, &desc)))
                printf_s(" └─ Technique: %s\n", desc.Name);
        }
    }
    else
    {
        printf_s(" └─ fx @+0x48 = %p [invalid]\n", fx);
    }

    if (!RealSub6C6080)
    {
        printf_s("❌ RealSub6C6080 is null!\n");
        return 0;
    }

    int result = RealSub6C6080(thisptr);
    printf_s("[XNFS-ShaderLoader-MW] [Hook] sub_6C6080 result = 0x%08X (%d)\n", result, result);
    return result;
}


void* FindFxWrapperByShaderIndex(int shaderIndex)
{
    const uintptr_t start = 0x01000000;
    const uintptr_t end   = 0x20000000;

    for (uintptr_t addr = start; addr < end; addr += 4)
    {
        __try
        {
            int value = *(int*)(addr + 0x4);
            if (value == shaderIndex)
            {
                void* fx = *(void**)(addr + 0x48);
                if (IsValidShaderPointer((ID3DXEffect*)fx))
                {
                    printf_s("[FindFxWrapper] ✅ Found fxWrapper at %p for index %d\n", (void*)addr, shaderIndex);
                    return (void*)addr;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            continue;
        }
    }

    printf_s("[FindFxWrapper] ❌ Could not find wrapper for shader index %d\n", shaderIndex);
    return nullptr;
}

void RecompileAndReloadAll()
{
    for (const auto& key : g_FxOverrides)
    {
        auto it = g_ShaderOverridePaths.find(key);
        if (it == g_ShaderOverridePaths.end())
        {
            printf_s("[HotReload] ❌ No path found for %s\n", key.c_str());
            continue;
        }

        const std::string& fxPath = it->second;
        if (!CompileAndDumpShader(key, fxPath))
        {
            printf_s("[HotReload] ❌ Failed to recompile %s\n", fxPath.c_str());
            continue;
        }

        FILE* f = fopen(key.c_str(), "rb");
        if (!f)
        {
            printf_s("[HotReload] ❌ Failed to reopen compiled FX: %s\n", key.c_str());
            continue;
        }

        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        fseek(f, 0, SEEK_SET);

        std::vector<char> buffer(len);
        fread(buffer.data(), 1, len, f);
        fclose(f);

        FXIncludeHandler includeHandler;
        LPD3DXEFFECT fx = nullptr;
        LPD3DXBUFFER errors = nullptr;

        HRESULT hr = D3DXCreateEffect(
            g_Device, buffer.data(), (UINT)len,
            nullptr, &includeHandler,
            D3DXSHADER_DEBUG, nullptr, &fx, &errors);

        if (FAILED(hr) || !fx || !IsValidShaderPointer(fx))
        {
            printf_s("[HotReload] ❌ Invalid fx pointer after compile: %p (hr=0x%08X)\n", fx, hr);
            if (errors)
            {
                printf_s("[HotReload] Error: %s\n", (char*)errors->GetBufferPointer());
                errors->Release();
            }
            continue;
        }

        printf_s("[HotReload] ✅ Reloaded shader: %s (fx=%p)\n", key.c_str(), fx);

        g_LastCompiledShaders[key] = fx;
        fx->AddRef();

        ForceReplaceShaderIntoSlots(key, fx);
        printf_s("[HotReload] ✅ Force-replaced effect in all slots for %s\n", key.c_str());

        // Normalize shader key to match internal resource names
        std::string shaderResourceName = key;

        // Strip path
        size_t lastSlash = shaderResourceName.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            shaderResourceName = shaderResourceName.substr(lastSlash + 1);

        // Strip extension
        size_t dot = shaderResourceName.rfind(".fx");
        if (dot != std::string::npos)
            shaderResourceName = shaderResourceName.substr(0, dot);

        // Uppercase
        for (char& c : shaderResourceName)
            c = static_cast<char>(toupper(c));

        // Final name
        shaderResourceName = "IDI_" + shaderResourceName + "_FX";

        // Find the slot and record it for fallback
        auto slots = LookupShaderSlotsFromResource(shaderResourceName);
        if (!slots.empty())
        {
            g_LastShaderIndices[shaderResourceName] = slots.front();
            printf_s("[HotReload] ✅ Fallback recorded index %d for %s\n", slots.front(), shaderResourceName.c_str());
        }

        printf_s("[HotReload] Triggering ApplyGraphicsSettings()\n");
        g_TriggerApplyGraphicsSettings = true;
        printf_s("[HotReload] Will apply graphics settings at next Present\n");
    }
}

void ReloadShaderFxWrapper(const std::string& shaderKey)
{
    auto it = g_LastCompiledShaders.find(shaderKey);
    if (it == g_LastCompiledShaders.end() || !IsValidShaderPointer(it->second))
    {
        printf_s("[HotReload] ❌ No known compiled shader for key: %s\n", shaderKey.c_str());
        g_TriggerApplyGraphicsSettings = true;
        return;
    }

    ID3DXEffect* fx = it->second;
    void** shaderTable = (void**)0x0093DE78;
    int shaderIndex = -1;

    for (int i = 0; i < 64; i++)
    {
        if (shaderTable[i] == fx)
        {
            shaderIndex = i;
            printf_s("[HotReload] ✅ Found fx in shader table slot %d\n", i);
            break;
        }
    }

    std::string shaderResourceName = shaderKey;
    size_t lastSlash = shaderResourceName.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        shaderResourceName = shaderResourceName.substr(lastSlash + 1);

    size_t dot = shaderResourceName.rfind(".fx");
    if (dot != std::string::npos)
        shaderResourceName = shaderResourceName.substr(0, dot);

    for (char& c : shaderResourceName)
        c = static_cast<char>(toupper(c));

    shaderResourceName = "IDI_" + shaderResourceName + "_FX";

    if (shaderIndex == -1)
    {
        auto fallback = g_LastShaderIndices.find(shaderResourceName);
        if (fallback != g_LastShaderIndices.end())
        {
            shaderIndex = fallback->second;
            printf_s("[HotReload] ⚠️ Using recorded fallback shader index %d for %s\n", shaderIndex, shaderResourceName.c_str());
        }
        else
        {
            auto slots = LookupShaderSlotsFromResource(shaderResourceName);
            if (!slots.empty())
            {
                shaderIndex = slots.front();
                printf_s("[HotReload] ⚠️ Using string-table fallback index %d for %s\n", shaderIndex, shaderResourceName.c_str());
            }
            else
            {
                printf_s("[HotReload] ❌ Could not locate compiled fx in shader table or via fallback\n");
                return;
            }
        }
    }

    void* fxWrapper = FindFxWrapperByShaderIndex(shaderIndex);
    if (!fxWrapper)
        return;

    void** shaderSlot = (void**)((char*)fxWrapper + 0x48);
    *shaderSlot = fx;
    fx->AddRef();

    printf_s("[HotReload] ✅ Assigned compiled shader to fxWrapper->+0x48: %p\n", fx);

    int result = HookedSub6C6080(fxWrapper);
    printf_s("[HotReload] ✅ sub_6C6080 result = 0x%08X (%d)\n", result, result);
}

DWORD WINAPI DeferredReloadThread(LPVOID)
{
    Sleep(500); // wait half a second
    printf_s("[HotReload] ⚡ Calling sub_6D7500() from thread\n");
    ReloadShaderFxWrapper("IDI_VISUALTREATMENT_FX");
    return 0;
}

void HookedSub6D7500()
{
    static bool inProgress = false;
    if (inProgress)
    {
        printf_s("[Hook] ?? Reentry detected in sub_6D7500, skipping\n");
        return;
    }

    inProgress = true;
    printf_s("[Hook] ? HookedSub6D7500 running\n");

    // Null the shader slot before proceeding
    void* fxWrapper = *(void**)0x00982AF0;
    if (fxWrapper)
    {
        void** slot = (void**)((char*)fxWrapper + 0x48);
        if (*slot)
        {
            printf_s("[HotReload] ⚠️ Nulling fxWrapper->+0x48 (%p)\n", *slot);
            *slot = nullptr;
        }
        else
        {
            printf_s("[HotReload] ℹ️ Shader slot already null\n");
        }
    }

    // Call the real dispatcher
    if (RealSub6D7500)
        RealSub6D7500();

    inProgress = false;
}

// Utility: Shader creation tracker
bool ShaderWasCreated(const std::string& name)
{
    return g_CreatedShaderNames.find(name) != g_CreatedShaderNames.end();
}

HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                             const RECT* src, const RECT* dest,
                             HWND hwnd, const RGNDATA* dirty)
{
    static int presentCount = 0;
    static bool pendingTrigger = false;

    static bool feFxSeen = false;
    static bool feMaskSeen = false;
    static int delayFrames = 0;

    presentCount++;

    printf_s("[Hook] Present #%d — pendingTrigger=%d g_FrontendReady=%d\n", presentCount, pendingTrigger,
             g_FrontendReady);

    if (!g_FrontendReady)
    {
        if (ShaderWasCreated("IDI_FE_FX")) feFxSeen = true;
        if (ShaderWasCreated("IDI_FE_MASK_FX")) feMaskSeen = true;

        if (feFxSeen && feMaskSeen)
        {
            if (delayFrames == 0)
            {
                delayFrames = 10; // wait ~10 frames for frontend stability
                printf_s("[XNFS-ShaderLoader-MW] [Hook] Frontend shaders loaded — delaying %d frames\n", delayFrames);
            }
            else
            {
                delayFrames--;
                if (delayFrames == 0)
                {
                    g_FrontendReady = true;
                    printf_s("[XNFS-ShaderLoader-MW] [Hook] ✅ Frontend is now fully ready\n");
                }
            }
        }
    }

    ApplyQueuedShaderPatches();

    printf_s("[Debug] Created shaders so far:\n");
    for (const auto& name : g_CreatedShaderNames)
        printf_s("  • %s\n", name.c_str());

    if (g_TriggerApplyGraphicsSettings)
    {
        g_TriggerApplyGraphicsSettings = false;
        printf_s("[XNFS-ShaderLoader-MW] [Hook] Applying graphics settings (via Present)\n");
        ApplyGraphicsSettings();

        delayFrames = 5; // Wait a few frames before trying Enable
        pendingTrigger = true;
    }

    if (pendingTrigger && delayFrames > 0)
    {
        delayFrames--;
        printf_s("[XNFS-ShaderLoader-MW] [Hook] Waiting %d more frames before Enable...\n", delayFrames);
    }

    printf_s("[Debug] Reload check — g_TryReloadViaSub6D7500 = %d, g_TryReloadVisualTreatmentTriggered = %d\n",
             g_TryReloadViaSub6D7500, g_TryReloadVisualTreatmentTriggered);

    if (g_FrontendReady && delayFrames == 0 && g_TryReloadViaSub6D7500 && !g_TryReloadVisualTreatmentTriggered)
    {
        printf_s("[Present] ✅ ReloadVisualTreatmentFxWrapper() conditions met — frontend ready, delay done\n");
        ReloadShaderFxWrapper("IDI_VISUALTREATMENT_FX");
        g_TryReloadVisualTreatmentTriggered = true;
    }

    if (g_FrontendReady && pendingTrigger && delayFrames == 0)
    {
        printf_s("[HotReload] ⚡ Forcing sub_6D7500() to reinitialize shaders\n");
        using FnSub6D7500 = void(*)();
        ((FnSub6D7500)0x006D7500)();

        void* queue = SafeGetActionQueue();
        if (queue)
        {
            ActionQueue_Enable(queue, true);
            printf_s("[XNFS-ShaderLoader-MW] [Hook] ✅ Called ActionQueue::Enable(true)\n");
            pendingTrigger = false;
        }
        else
        {
            printf_s("[XNFS-ShaderLoader-MW] [Hook] ❌ SafeGetActionQueue() still null\n");
        }
    }

    return RealPresent(device, src, dest, hwnd, dirty);
}

void InstallPresentHookFromDevice(IDirect3DDevice9* device)
{
    if (g_PresentHooked || !device)
        return;

    void** vtable = *(void***)device;
    if (vtable && vtable[17])
    {
        RealPresent = (PresentFn)vtable[17];
        DWORD oldProtect;
        VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[17] = (void*)&HookedPresent;
        VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[Init] ✅ Hooked IDirect3DDevice9::Present\n");
        g_PresentHooked = true;
    }
}

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
    printf_s("[Hook] D3DXCreateEffectFromResourceA CALLED — pResource = %s\n",
             IsValidCString(pResource) ? pResource : "(null)");

    if (device == nullptr)
    {
        printf_s("[Hook] Warning: D3DXCreateEffectFromResourceA called with null device\n");
    }
    else if (!g_Device)
    {
        g_Device = device;
        LoadShaderOverrides();

        // Hook Present now that g_Device is valid
        InstallPresentHookFromDevice(device);
    }

    std::string safeResourceName = "(invalid)";
    if (IsValidCString(pResource))
    {
        printf_s("[D3D] D3DXCreateEffectFromResourceA(%s)\n", pResource);
        safeResourceName = std::string(pResource);
    }

    printf_s("[Hook] D3DXCreateEffectFromResourceA called #%d — pResource = %s\n", g_HookCallCount++,
             safeResourceName.c_str());

    if (g_FxOverrides.count(safeResourceName))
    {
        std::string path = std::string(safeResourceName); // assume key starts with IDI_
        FILE* f = fopen(safeResourceName.c_str(), "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<char> buffer(len);
            fread(buffer.data(), 1, len, f);
            fclose(f);

            FXIncludeHandler includeHandler;
            HRESULT hr = D3DXCreateEffect(device, buffer.data(), (UINT)len, defines, &includeHandler, flags, pool,
                                          outEffect, outErrors);
            if (SUCCEEDED(hr) && *outEffect)
            {
                printf_s("[Hook] Loaded compiled FX override for %s\n", safeResourceName.c_str());
                g_CreatedShaderNames.insert(safeResourceName); // ← track created
                return S_OK;
            }

            printf_s("[Hook] Failed to create effect from compiled override for %s\n", safeResourceName.c_str());
        }
        else
        {
            printf_s("[Hook] Failed to open compiled file: %s\n", path.c_str());
        }
    }

    // Fallback: load compiled shader from game root if it exists and wasn't compiled from .fx
    if (!g_FxOverrides.count(safeResourceName) && strncmp(pResource, "IDI_", 4) == 0)
    {
        FILE* f = fopen(safeResourceName.c_str(), "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<char> buffer(len);
            fread(buffer.data(), 1, len, f);
            fclose(f);

            FXIncludeHandler includeHandler;
            HRESULT hr = D3DXCreateEffect(device, buffer.data(), (UINT)len,
                                          defines, &includeHandler, flags, pool, outEffect, outErrors);
            if (SUCCEEDED(hr) && *outEffect)
            {
                printf_s("[Hook] Loaded fallback compiled shader: %s\n", safeResourceName.c_str());
                g_CreatedShaderNames.insert(pResource); // ← track fallback
                return S_OK;
            }

            printf_s("[Hook] Failed to load fallback compiled shader: %s\n", safeResourceName.c_str());
        }
    }


    return RealCreateFromResource(device, hModule, pResource, defines, include, flags, pool, outEffect, outErrors);
}

bool SafePatchShaderTable(int slot, ID3DXEffect* fx)
{
    if (slot < 0 || slot >= 64 || !IsValidShaderPointer(fx))
        return false;

    {
        std::lock_guard<std::mutex> lock(g_PatchMutex);
        fx->AddRef(); // Defer patch safely
        g_PendingPatches.push_back({slot, fx, 2});
    }

    printf_s("[Patch] ⏳ Deferred shader patch queued: slot %d → fx=%p\n", slot, fx);
    return true;
}

void PauseGameThread()
{
    g_PausePresent = true;

    // Wait up to 250ms for Present to acknowledge pause
    const auto start = std::chrono::steady_clock::now();
    while (!g_PresentIsWaiting &&
        std::chrono::steady_clock::now() - start < std::chrono::milliseconds(250))
    {
        Sleep(10);
    }

    if (g_PresentIsWaiting)
        printf_s("[Pause] Game Present thread paused.\n");
    else
        printf_s("[Pause] ⚠️ Timeout waiting for Present to stall — continuing anyway.\n");
}

void ResumeGameThread()
{
    g_PausePresent = false;
    printf_s("[Pause] Game Present thread resumed.\n");
}

void ForceReplaceShaderIntoSlots(const std::string& resourceKey, ID3DXEffect* fx)
{
    PauseGameThread();
    auto slots = LookupShaderSlotsFromResource(resourceKey);
    if (!slots.empty())
    {
        for (int slot : slots)
        {
            fx->AddRef();
            SafePatchShaderTable(slot, fx);
            printf_s("[HotReload] ✅ Patched slot %d with new effect for %s\n", slot, resourceKey.c_str());
        }
    }
    else
    {
        printf_s("[HotReload] ⚠️ No slots found for %s\n", resourceKey.c_str());
    }
    ResumeGameThread();

    g_TriggerApplyGraphicsSettings = true;
    printf_s("[HotReload] 🔁 Deferring ActionQueue::Enable to next Present\n");
}

DWORD WINAPI DeferredHookThread(LPVOID)
{
    while (!g_Device)
        Sleep(10);

    void** vtable = *(void***)g_Device;
    if (vtable)
    {
        RealPresent = (PresentFn)vtable[17];
        DWORD oldProtect;
        VirtualProtect(&vtable[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[17] = (void*)&HookedPresent;
        VirtualProtect(&vtable[17], sizeof(void*), oldProtect, &oldProtect);
        printf_s("[Init] Hooked IDirect3DDevice9::Present (deferred)\n");
    }
    return 0;
}

DWORD WINAPI HotkeyThread(LPVOID)
{
    while (true)
    {
        static HANDLE hReloadThread = nullptr;

        if (GetAsyncKeyState(VK_F2) & 1)
        {
            printf_s("[HotkeyThread] F2 pressed → Recompiling FX overrides...\n");

            RecompileAndReloadAll();

            g_TryReloadVisualTreatment = true;
            g_TryReloadViaSub6D7500 = true;
            g_TryReloadVisualTreatmentTriggered = false;

            // Prevent thread spam
            if (!hReloadThread || WaitForSingleObject(hReloadThread, 0) == WAIT_OBJECT_0)
            {
                hReloadThread = CreateThread(nullptr, 0, DeferredReloadThread, nullptr, 0, nullptr);
            }
        }

        Sleep(100);
    }

    return 0;
}

// -------------------- DllMain --------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        printf_s("[Init] Shader override DLL loaded.\n");

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (d3dx)
        {
            void* addr = GetProcAddress(d3dx, "D3DXCreateEffectFromResourceA");
            RealCreateFromResource = (D3DXCreateEffectFromResourceAFn)addr;
            injector::MakeCALL(0x006C60D2, HookedCreateFromResource, true);
            printf_s("[Init] Hooked D3DXCreateEffectFromResourceA\n");
        }

        MH_Initialize();
        // Hook CreateFXFromSlot (sub_6C6080)
        if (MH_CreateHook((LPVOID)0x006C6080, HookedSub6C6080, (void**)&RealSub6C6080) != MH_OK)
            printf_s("❌ Failed to create hook for sub_6C6080\n");
        if (MH_EnableHook((LPVOID)0x006C6080) != MH_OK)
            printf_s("❌ Failed to enable hook for sub_6C6080\n");

        // Hook shader reload dispatcher (sub_6D7500)
        if (MH_CreateHook((LPVOID)0x006D7500, HookedSub6D7500, (void**)&RealSub6D7500) != MH_OK)
            printf_s("❌ Failed to create hook for sub_6D7500\n");
        if (MH_EnableHook((LPVOID)0x006D7500) != MH_OK)
            printf_s("❌ Failed to enable hook for sub_6D7500\n");

        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HotkeyThread, nullptr, 0, nullptr);
        CreateThread(nullptr, 0, DeferredHookThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
