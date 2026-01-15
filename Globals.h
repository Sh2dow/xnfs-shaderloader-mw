#pragma once

#include <cstdint>
#include <unordered_set>
#include <d3d9.h>

#include "RenderTargetManager.h"

extern RenderTargetManager g_RenderTargetManager;

extern bool g_CustomMotionBlurRanThisFrame;
extern uint32_t g_CustomMotionBlurLastFrame;

extern std::unordered_set<void*> g_KnownRenderTargets;
extern IDirect3DSurface9* g_SceneTargetThisFrame;
extern float g_MotionBlurAmount;
extern bool g_MotionBlurHistoryReady;
extern bool g_InEndSceneBlur;
extern uint32_t g_SceneTargetBestArea;
extern uint32_t g_LastBackBufferW;
extern uint32_t g_LastBackBufferH;
extern bool g_PendingCustomBlur;
extern float g_MotionVec[4];
extern float g_LastCameraPos[3];
extern bool g_HasLastCameraPos;
extern bool g_HasLastViewMatrix;
