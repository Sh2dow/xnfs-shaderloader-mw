#pragma once

#include <d3d9.h>

namespace ExposureStandalone
{
    struct ExposureConfig
    {
        float middleGray = 0.18f;
        float minAdaptedLuminance = 0.001f;
        float sampleIntervalSeconds = 0.25f;
        float rateFast = 0.06f;
        float rateSlow = 0.02f;
        float rateDown = 0.015f;
    };

    struct ExposureState
    {
        float currentFrameLuminance = 0.75f;
        float adaptedLuminance = 0.75f;
        float exposure = 0.75f;
        float luminanceTimer = 0.0f;
        bool forceSnap = false;
    };

    class ExposureSampler
    {
    public:
        bool Init(IDirect3DDevice9* device);
        void Shutdown();

        // Samples average luminance from the given backbuffer surface.
        bool SampleLuminance(IDirect3DSurface9* backBufferSurface);

        // Runs adaptation/exposure update using the last sampled luminance.
        void UpdateExposure(float deltaTimeSeconds, float exposureBias);

        float GetExposure() const { return m_state.exposure; }
        float GetAdaptedLuminance() const { return m_state.adaptedLuminance; }

    private:
        float ComputeExposure(float adaptedLuminance, float exposureBias) const;
        void AdaptLuminance(float deltaTimeSeconds);

        IDirect3DDevice9* m_device = nullptr;
        IDirect3DTexture9* m_luminanceRT = nullptr;
        IDirect3DSurface9* m_luminanceSurface = nullptr;
        IDirect3DSurface9* m_readbackSurface = nullptr;

        ExposureConfig m_cfg = {};
        ExposureState m_state = {};
    };
}
