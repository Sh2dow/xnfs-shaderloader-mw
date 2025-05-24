#pragma once
#include <d3d9.h>
#include <d3dx9effect.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

class ShaderManager
{
public:
    static void LoadOverrides();
    static bool SafePatchShaderTable(int slot, ID3DXEffect* fx);
    static void ApplyQueuedShaderPatches();
    static void PauseGameThread();
    static void ResumeGameThread();

    static void DumpShaderTable();
    static void LoadShaderOverrides();

    static LPDIRECT3DDEVICE9 g_Device;

    static std::unordered_set<std::string> g_FxOverrides;
    static std::unordered_map<std::string, std::string> g_ShaderOverridePaths;

    typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

    static PresentFn RealPresent;

private:
    // struct QueuedPatch
    // {
    //     int slot;
    //     ID3DXEffect* fx;
    //     int framesRemaining;
    // };

    static std::string ToUpper(const std::string& str);
    static bool CompileAndDumpShader(const std::string& key, const std::string& fxPath);
    static std::vector<int> LookupShaderSlotsFromResource(const std::string& resourceName);

    class FXIncludeHandler : public ID3DXInclude
    {
    public:
        STDMETHOD(Open)(D3DXINCLUDE_TYPE, LPCSTR, LPCVOID, LPCVOID* ppData, UINT* pBytes) override;
        STDMETHOD(Close)(LPCVOID pData) override;
    };
};

extern bool IsValidShaderPointer(ID3DXEffect* fx);
extern void ForceReplaceShaderIntoSlots(const std::string& resourceKey, ID3DXEffect* fx); 
extern void ReleaseAllRetainedShaders();
extern void RecompileAndReloadAll();
extern void ScanIVisualTreatment();
extern void PrintFxAtOffsets(void* obj);
extern bool ReplaceShaderSlot(BYTE* object, int offset, ID3DXEffect* newFx);
extern void ClearMatchingShaders(BYTE* object, ID3DXEffect* newFx);
extern bool IsValidThis(void* ptr);

extern void* g_ThisCandidates[3];

extern void* g_LatestEView;
extern unsigned g_FrameId;
extern unsigned g_LastApplySeenFrame;
extern void** g_pVisualTreatment;
extern void* g_ApplyGraphicsManagerThis;
extern std::pair<void*, int> lastKey;
extern int repeatCount;
extern int g_ThisCount;

typedef HRESULT (WINAPI*D3DXCreateEffectFromResourceAFn)(
    LPDIRECT3DDEVICE9, HMODULE, LPCSTR, const D3DXMACRO*,
    LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);

extern HRESULT WINAPI HookedCreateFromResource(
    LPDIRECT3DDEVICE9 device,
    HMODULE hModule,
    LPCSTR pResource,
    const D3DXMACRO* defines,
    LPD3DXINCLUDE include,
    DWORD flags,
    LPD3DXEFFECTPOOL pool,
    LPD3DXEFFECT* outEffect,
    LPD3DXBUFFER* outErrors);

extern D3DXCreateEffectFromResourceAFn RealCreateFromResource;

typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

extern HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                                    const RECT* src, const RECT* dest,
                                    HWND hwnd, const RGNDATA* dirty);
