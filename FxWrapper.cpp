#include "FxWrapper.h"
#include <stdio.h>
#include "Log.h"
#include "Validators.h"
#define printf_s(...) asi_log::Log(__VA_ARGS__)

void FxWrapper::ClearMotionBlurTexture() const
{
    if (!m_fx || IsBadReadPtr(m_fx, sizeof(void*)))
    {
        printf_s("[FxWrapper] ❌ Invalid m_fx pointer when clearing MISCMAP3_TEXTURE\n");
        return;
    }

    HRESULT hr = m_fx->SetTexture("MISCMAP3_TEXTURE", nullptr);
    if (FAILED(hr))
    {
        printf_s("[FxWrapper] ⚠️ Failed to clear MISCMAP3_TEXTURE (0x%08X)\n", hr);
    }
    else
    {
        printf_s("[FxWrapper] 🧹 Cleared MISCMAP3_TEXTURE from m_fx: %p\n", m_fx);
    }
}

void FxWrapper::ReloadHandles()
{
    if (!m_fx)
        return;

    hMiscMap3 = m_fx->GetParameterByName(nullptr, "MISCMAP3_TEXTURE");
    hBlurParams = m_fx->GetParameterByName(nullptr, "BlurParams");

    printf_s("[FxWrapper] 🔁 Refreshed handles for %s (%p)\n", name.c_str(), m_fx);
    if (!hMiscMap3) printf_s("[FxWrapper] ⚠️ hMiscMap3 not found!\n");
    if (!hBlurParams) printf_s("[FxWrapper] ⚠️ hBlurParams not found!\n");
}

bool FxWrapper::IsValid() const
{
    if (!m_fx)
        return false;

    // Optional: Validate underlying pointer
    void* vtable = *(void**)m_fx;
    return IsValidCodePtr(vtable); // ensures m_fx is a valid ID3DXEffect*
}
