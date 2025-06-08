#pragma once
#include <d3d9.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "FxWrapper.h"
// Define a cast macro that allows old std::shared_ptr<FxWrapper> to be used with minimal refactoring

class ShaderManager
{
public:
    static bool SafePatchShaderTable(int slot, std::shared_ptr<FxWrapper> fx);

    static void LoadOverrides();
    static void ApplyQueuedShaderPatches();
    static void PauseGameThread();
    static void ResumeGameThread();
    static void DumpShaderTable();

    static std::unordered_set<std::string> g_FxOverrides;
    static std::unordered_map<std::string, std::string> g_ShaderOverridePaths;

    typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

    static PresentFn RealPresent;

private:
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

inline bool TryApplyGraphicsSettingsSafely();
inline bool CompileShaderOverrides();
inline void ForceReplaceShaderIntoSlots(const std::string& resourceName, std::shared_ptr<FxWrapper> fxWrapper);
inline void ReleaseAllRetainedShaders();
inline void ReleaseAllActiveEffects();
inline bool RecompileAndReloadAll();
inline void ScanIVisualTreatment();
inline void PrintFxAtOffsets(void* obj);
inline bool ReplaceShaderSlot(BYTE* object, int offset, const std::shared_ptr<FxWrapper>& newFx);
inline bool ReplaceShaderSlot_RawEffect(
    BYTE* baseObject,
    int offset,
    ID3DXEffect* newRawFx,
    int slotIndex);
inline void ClearMatchingShaders(BYTE* object, std::shared_ptr<FxWrapper> newFx);
inline void ReleaseMotionBlurTexture();

inline void* GetFirstIVisualTreatmentObject();
inline bool IsLikelyApplyGraphicsSettingsObject(void* candidate);

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

inline D3DXCreateEffectFromResourceAFn RealCreateFromResource;

typedef HRESULT (WINAPI*PresentFn)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);

extern HRESULT WINAPI HookedPresent(IDirect3DDevice9* device,
                                    const RECT* src, const RECT* dest,
                                    HWND hwnd, const RGNDATA* dirty);

// -------------------- NFSMW-RenderTarget block --------------------

typedef HRESULT (WINAPI*Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
inline Reset_t oReset;
extern HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* params);


// -------------------- NFSMW-RenderTarget block End --------------------
