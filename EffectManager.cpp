#include "EffectManager.h"

#include <cstring>
#include <cstdio>

#include "Globals.h"

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

static const char* kCustomBlurFx = R"FX(
texture DIFFUSEMAP_TEXTURE;
texture MOTIONBLUR_TEXTURE;
texture DEPTHBUFFER_TEXTURE;
sampler2D DIFFUSEMAP_SAMPLER = sampler_state
{
    Texture = <DIFFUSEMAP_TEXTURE>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler2D MOTIONBLUR_SAMPLER = sampler_state
{
    Texture = <MOTIONBLUR_TEXTURE>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler2D DEPTHBUFFER_SAMPLER = sampler_state
{
    Texture = <DEPTHBUFFER_TEXTURE>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

float2 BlurTexelSize;
float MotionBlurBlend;
float2 MotionVec;
float MotionBlurScale;

struct VS_IN
{
    float4 position : POSITION;
    float4 tex0     : TEXCOORD0;
    float4 tex1     : TEXCOORD1;
    float4 tex2     : TEXCOORD2;
    float4 tex3     : TEXCOORD3;
    float4 tex4     : TEXCOORD4;
    float4 tex5     : TEXCOORD5;
    float4 tex6     : TEXCOORD6;
    float4 tex7     : TEXCOORD7;
};

struct VS_OUT
{
    float4 position : POSITION;
    float4 tex01    : TEXCOORD0;
    float4 tex23    : TEXCOORD1;
    float4 tex45    : TEXCOORD2;
    float4 tex67    : TEXCOORD3;
};

VS_OUT vs_main(VS_IN IN)
{
    VS_OUT OUT;
    OUT.position = IN.position;
    OUT.tex01 = float4(IN.tex0.xy, IN.tex1.xy);
    OUT.tex23 = float4(IN.tex2.xy, IN.tex3.xy);
    OUT.tex45 = float4(IN.tex4.xy, IN.tex5.xy);
    OUT.tex67 = float4(IN.tex6.xy, IN.tex7.xy);
    return OUT;
}

float4 ps_blur(VS_OUT IN) : COLOR
{
    float2 uv0 = IN.tex01.xy;
    float2 uv1 = IN.tex01.zw;
    float2 uv2 = IN.tex23.xy;
    float2 uv3 = IN.tex23.zw;
    float2 uv4 = IN.tex45.xy;
    float2 uv5 = IN.tex45.zw;
    float2 uv6 = IN.tex67.xy;
    float2 uv7 = IN.tex67.zw;

    float4 c = tex2D(DIFFUSEMAP_SAMPLER, uv0) * (6.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv1) * (5.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv2) * (4.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv3) * (3.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv4) * (2.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv5) * (1.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv6) * (1.0 / 23.0);
    c += tex2D(DIFFUSEMAP_SAMPLER, uv7) * (1.0 / 23.0);
    c.a = tex2D(DEPTHBUFFER_SAMPLER, uv0).x;
    return c;
}

float4 ps_copy(VS_OUT IN) : COLOR
{
    float4 c = tex2D(DIFFUSEMAP_SAMPLER, IN.tex01.xy);
    c.a = MotionBlurBlend;
    return c;
}

float4 ps_temporal(VS_OUT IN) : COLOR
{
    float4 curr = tex2D(DIFFUSEMAP_SAMPLER, IN.tex01.xy);
    float4 prev = tex2D(MOTIONBLUR_SAMPLER, IN.tex01.xy);
    return lerp(curr, prev, MotionBlurBlend);
}

float4 ps_tint(VS_OUT IN) : COLOR
{
    return float4(1.0f, 0.0f, 1.0f, 1.0f);
}

technique blur
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader = compile ps_2_0 ps_blur();
    }
}

technique copy
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader = compile ps_2_0 ps_copy();
    }
}

technique temporal
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader = compile ps_2_0 ps_temporal();
    }
}

technique tint
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader = compile ps_2_0 ps_tint();
    }
}
)FX";

bool EffectManager::TrySetTechnique(ID3DXEffect* fx, const char* const* names, size_t count)
{
    if (!fx) return false;
    for (size_t i = 0; i < count; ++i)
    {
        D3DXHANDLE tech = fx->GetTechniqueByName(names[i]);
        if (tech && SUCCEEDED(fx->ValidateTechnique(tech)) && SUCCEEDED(fx->SetTechnique(tech)))
            return true;
    }
    return false;
}

bool EffectManager::EnsureCustomBlurEffect(IDirect3DDevice9* device)
{
    if (!device) return false;
    if (g_RenderTargetManager.g_CustomBlurEffect) return true;

    ID3DXBuffer* errors = nullptr;
    HRESULT hr = D3DXCreateEffect(
        device,
        kCustomBlurFx,
        (UINT)std::strlen(kCustomBlurFx),
        nullptr,
        nullptr,
        D3DXSHADER_SKIPOPTIMIZATION,
        nullptr,
        &g_RenderTargetManager.g_CustomBlurEffect,
        &errors);

    if (FAILED(hr))
    {
        if (errors)
        {
            printf_s("[XNFS] ? Custom blur FX compile failed: %s\n", (const char*)errors->GetBufferPointer());
            errors->Release();
        }
        else
        {
            printf_s("[XNFS] ? Custom blur FX compile failed: 0x%08X (errors=null)\n", hr);
        }
        return false;
    }

    printf_s("[XNFS] ? Custom blur FX compiled successfully (%p)\n", g_RenderTargetManager.g_CustomBlurEffect);
    SAFE_RELEASE(errors);
    return true;
}
