#include "Globals.h"

RenderTargetManager g_RenderTargetManager;

bool g_CustomMotionBlurRanThisFrame = false;
uint32_t g_CustomMotionBlurLastFrame = 0;
bool g_PendingCustomBlur = false;

std::unordered_set<void*> g_KnownRenderTargets;
IDirect3DSurface9* g_SceneTargetThisFrame = nullptr;
float g_MotionBlurAmount = 0.0f;
bool g_MotionBlurHistoryReady = false;
bool g_InEndSceneBlur = false;
uint32_t g_SceneTargetBestArea = 0;
uint32_t g_LastBackBufferW = 0;
uint32_t g_LastBackBufferH = 0;
float g_MotionVec[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float g_LastCameraPos[3] = {0.0f, 0.0f, 0.0f};
bool g_HasLastCameraPos = false;
bool g_HasLastViewMatrix = false;

HRESULT (WINAPI* oPresent)(
    IDirect3DDevice9*,
    const RECT*,
    const RECT*,
    HWND,
    const RGNDATA*
) = nullptr;
