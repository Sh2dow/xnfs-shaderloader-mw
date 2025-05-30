#pragma once
#include <d3d9.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "FxWrapper.h"
// Define a cast macro that allows old FxWrapper* to be used with minimal refactoring

class ShaderManager
{
public:
    static void LoadOverrides();
    static bool SafePatchShaderTable(int slot, FxWrapper* fx);
    static void ApplyQueuedShaderPatches();
    static void PauseGameThread();
    static void ResumeGameThread();

    static void DumpShaderTable();

    static std::unordered_set<std::string> g_FxOverrides;
    static std::unordered_map<std::string, std::string> g_ShaderOverridePaths;

    typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

    static PresentFn RealPresent;

private:
    static FxWrapper* g_LastReloadedFx;

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

extern bool TryApplyGraphicsSettingsSafely();
extern void CompileShaderOverride();
extern void ForceReplaceShaderIntoSlots(const std::string& resourceKey, FxWrapper* fx);
extern void ReleaseAllRetainedShaders();
extern void ReleaseAllActiveEffects();
extern bool RecompileAndReloadAll();
extern void ScanIVisualTreatment();
extern void PrintFxAtOffsets(void* obj);
extern bool ReplaceShaderSlot(BYTE* object, int offset, FxWrapper* newFx);
extern void ClearMatchingShaders(BYTE* object, FxWrapper* newFx);
extern void ReleaseMotionBlurTexture();

extern bool compiled;
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

inline D3DXCreateEffectFromResourceAFn RealCreateFromResource = nullptr;

typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

extern HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                                    const RECT* src, const RECT* dest,
                                    HWND hwnd, const RGNDATA* dirty);

// -------------------- NFSMW-RenderTarget block --------------------

typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
extern Reset_t oReset;
extern HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params);


// -------------------- NFSMW-RenderTarget block End --------------------
