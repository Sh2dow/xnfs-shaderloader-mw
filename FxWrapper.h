#pragma once
#include <d3dx9effect.h>
#include <memory>
#include <string>

class FxWrapper
{
public:
    FxWrapper(ID3DXEffect* fx) : m_fx(fx)
    {
        if (m_fx) m_fx->AddRef();
    }

    ~FxWrapper()
    {
        if (m_fx)
        {
            printf_s("[FxWrapper] ~FxWrapper() releasing m_fx = %p\n", m_fx);
            m_fx->Release();
            m_fx = nullptr;
        }
        else
        {
            printf_s("[FxWrapper] ~FxWrapper() called with null m_fx!\n");
        }
    }

    // Implement AddRef and Release
    ULONG AddRef()
    {
        return m_fx ? m_fx->AddRef() : 0;
    }

    ULONG Release()
    {
        return m_fx ? m_fx->Release() : 0;
    }

    HRESULT CloneEffect(IDirect3DDevice9* pDevice, std::shared_ptr<FxWrapper>* ppWrapper)
    {
        if (!m_fx || !ppWrapper) return E_FAIL;

        ID3DXEffect* cloned = nullptr;
        HRESULT hr = m_fx->CloneEffect(pDevice, &cloned);
        if (FAILED(hr) || !cloned)
            return hr;

        *ppWrapper = std::make_shared<FxWrapper>(cloned);
        return S_OK;
    }

    ID3DXEffect* GetEffect() const
    {
        return m_fx; // No need to go through SafeGetEffect
    }

    void OnLostDevice() const
    {
        if (m_fx) m_fx->OnLostDevice();
    }

    void OnResetDevice() const
    {
        if (m_fx) m_fx->OnResetDevice();
    }

    ID3DXEffect* operator->() const
    {
        return m_fx;
    }

    void ClearMotionBlurTexture() const;
    void ReloadHandles();
    bool IsValid() const;

    std::string name;

    // Cached handles
    D3DXHANDLE hMiscMap3 = nullptr;
    D3DXHANDLE hBlurParams = nullptr;

private:
    ID3DXEffect* m_fx = nullptr;
};

inline std::shared_ptr<FxWrapper> FromRaw(ID3DXEffect* fx)
{
    return fx ? std::make_shared<FxWrapper>(fx) : nullptr;
}
