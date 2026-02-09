#include "stdafx.h"
#include "ExposureStandalone.h"

namespace ExposureStandalone
{
    static float ClampMin(float value, float minValue)
    {
        return (value < minValue) ? minValue : value;
    }

    bool ExposureSampler::Init(IDirect3DDevice9* device)
    {
        if (!device)
            return false;

        m_device = device;
        m_device->AddRef();

        HRESULT hr = m_device->CreateTexture(
            1,
            1,
            1,
            D3DUSAGE_RENDERTARGET,
            D3DFMT_A8R8G8B8,
            D3DPOOL_DEFAULT,
            &m_luminanceRT,
            nullptr);

        if (FAILED(hr) || !m_luminanceRT)
            return false;

        hr = m_luminanceRT->GetSurfaceLevel(0, &m_luminanceSurface);
        if (FAILED(hr) || !m_luminanceSurface)
            return false;

        hr = m_device->CreateOffscreenPlainSurface(
            1,
            1,
            D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM,
            &m_readbackSurface,
            nullptr);

        return SUCCEEDED(hr) && m_readbackSurface != nullptr;
    }

    void ExposureSampler::Shutdown()
    {
        if (m_readbackSurface)
        {
            m_readbackSurface->Release();
            m_readbackSurface = nullptr;
        }
        if (m_luminanceSurface)
        {
            m_luminanceSurface->Release();
            m_luminanceSurface = nullptr;
        }
        if (m_luminanceRT)
        {
            m_luminanceRT->Release();
            m_luminanceRT = nullptr;
        }
        if (m_device)
        {
            m_device->Release();
            m_device = nullptr;
        }
    }

    bool ExposureSampler::SampleLuminance(IDirect3DSurface9* backBufferSurface)
    {
        if (!m_device || !backBufferSurface || !m_luminanceSurface || !m_readbackSurface)
            return false;

        HRESULT hr = m_device->StretchRect(
            backBufferSurface,
            nullptr,
            m_luminanceSurface,
            nullptr,
            D3DTEXF_LINEAR);

        if (FAILED(hr))
            return false;

        hr = m_device->GetRenderTargetData(m_luminanceSurface, m_readbackSurface);
        if (FAILED(hr))
            return false;

        D3DLOCKED_RECT locked = {};
        hr = m_readbackSurface->LockRect(&locked, nullptr, D3DLOCK_READONLY);
        if (FAILED(hr) || !locked.pBits)
            return false;

        const unsigned int argb = *reinterpret_cast<const unsigned int*>(locked.pBits);
        const float r = static_cast<float>((argb >> 16) & 0xFFu) * (1.0f / 255.0f);
        const float g = static_cast<float>((argb >> 8) & 0xFFu) * (1.0f / 255.0f);
        const float b = static_cast<float>((argb >> 0) & 0xFFu) * (1.0f / 255.0f);

        m_readbackSurface->UnlockRect();

        const float lum = 0.6125 * r + 0.5154 * g + 0.0721 * b;
        m_state.currentFrameLuminance = ClampMin(lum, m_cfg.minAdaptedLuminance);
        return true;
    }

    void ExposureSampler::UpdateExposure(float deltaTimeSeconds, float exposureBias)
    {
        if (deltaTimeSeconds <= 0.0f)
            deltaTimeSeconds = 1.0f / 60.0f;

        m_state.luminanceTimer += deltaTimeSeconds;

        if (m_state.luminanceTimer >= m_cfg.sampleIntervalSeconds)
            m_state.luminanceTimer -= m_cfg.sampleIntervalSeconds;

        AdaptLuminance(deltaTimeSeconds);
        m_state.exposure = ComputeExposure(m_state.adaptedLuminance, exposureBias);
    }

    float ExposureSampler::ComputeExposure(float adaptedLuminance, float exposureBias) const
    {
        const float denom = ClampMin(adaptedLuminance, m_cfg.minAdaptedLuminance);
        return ((m_cfg.middleGray * 2.0f) / denom) * (exposureBias + 1.0f);
    }

    void ExposureSampler::AdaptLuminance(float deltaTimeSeconds)
    {
        float tau = 0.0333333f / deltaTimeSeconds;
        if (tau < 1.0f)
            tau = 1.0f;

        if (m_state.forceSnap)
        {
            m_state.adaptedLuminance = m_state.currentFrameLuminance;
            m_state.forceSnap = false;
            return;
        }

        float rate = m_cfg.rateFast;
        if (m_state.adaptedLuminance >= 0.06f)
        {
            rate = m_cfg.rateSlow;
            if (m_state.currentFrameLuminance - m_state.adaptedLuminance <= 0.0f)
                rate = m_cfg.rateDown;
        }

        const float alpha = rate / tau;
        m_state.adaptedLuminance =
            (1.0f - alpha) * m_state.adaptedLuminance +
            alpha * m_state.currentFrameLuminance;
    }
}
