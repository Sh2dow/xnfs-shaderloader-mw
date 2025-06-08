#pragma once
#include <unordered_set>

#include "FxWrapper.h"
#include "Hooks.h"
#include "RenderTargetManager.h"

struct QueuedPatch;
inline RenderTargetManager g_RenderTargetManager;

inline ApplyGraphicsSettingsFn ApplyGraphicsSettingsOriginal = nullptr;
inline ApplyGraphicsManagerMain_t ApplyGraphicsManagerMainOriginal = nullptr;
inline IVisualTreatment_ResetFn IVisualTreatment_Reset = (IVisualTreatment_ResetFn)Reset_16IVisualTreatment_ADDRESS;

inline const int ShaderTableSize = 62;
inline static std::shared_ptr<FxWrapper>* g_ShaderTable = (std::shared_ptr<FxWrapper>*)SHADER_TABLE_ADDRESS;
// NFS MW shader g_ShaderTable
inline std::mutex g_ShaderTableLock = std::mutex();
inline std::unordered_map<std::string, std::shared_ptr<FxWrapper>> g_DebugLiveEffects;
inline std::unordered_set<ID3DXEffect*> g_AlreadyInjectedFxThisFrame;
// Map from shader resource name to fallback slot
inline std::unordered_map<std::string, int> g_DynamicFallbackSlots;

inline std::unordered_map<std::string, std::shared_ptr<FxWrapper>> g_ActiveEffects;
inline bool g_WaitingForReset = false;
inline bool g_ApplyGraphicsSeenThisFrame = false;
inline bool g_CallApplyGraphicsManagerNextFrame = false;
inline int g_ApplyGraphicsTriggerDelay = 0;
inline bool g_ResumeGameThreadNextPresent = false;
inline bool g_EnableShaderTableDump = false;
inline void* g_LiveVisualTreatmentObject = nullptr;
inline bool g_PendingVisualReset = false;

inline void* g_ApplyGraphicsManagerThis;
inline void* g_ApplyGraphicsSettingsThis;

inline void* lastPatchedThis;

inline std::shared_ptr<FxWrapper> g_SlotRetainedFx[64];
inline std::shared_ptr<FxWrapper> g_LastReloadedFx;

static bool compiled = false;
inline void* g_ThisCandidates[3] = {};
inline int g_ThisCount = 0;
inline void* g_LatestEView;
inline unsigned g_FrameId;
inline unsigned g_LastApplySeenFrame;
inline void** g_pVisualTreatment = (void**)pVisualTreatmentPlat_ADDRESS;
inline std::pair<void*, int> lastKey;
inline int repeatCount;

inline struct Vertex
{
    float x, y, z, rhw;
    float u, v;
} screenQuadVerts[4];

struct QueuedPatch
{
    int slot = -1;
    std::shared_ptr<FxWrapper> fx;
    int framesRemaining = 2; // safe delay
    std::string resourceName;
};

inline static std::unordered_map<std::string, std::vector<char>> g_ShaderBuffers;
inline static std::vector<QueuedPatch> g_PendingPatches;

inline static std::mutex g_PatchMutex;
inline static std::unordered_map<std::string, std::string> g_ShaderOverridePaths;
inline static std::unordered_set<std::string> g_FxOverrides;

inline static std::atomic<int> g_HookCallCount{0};
inline std::atomic<bool> g_TriggerApplyGraphicsSettings = false;

inline static std::atomic<bool> g_PausePresent{false};
inline static std::atomic<bool> g_PresentIsWaiting{false};


inline static PDIRECT3DTEXTURE9 g_MyBlurTexture;
inline static PDIRECT3DSURFACE9 g_MyBlurSurface;
