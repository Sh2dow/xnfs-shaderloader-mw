#include "EffectManager.h"

#include <cstring>
#include <cstdio>
#include <windows.h>

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

float4 ps_composite(VS_OUT IN) : COLOR
{
    float2 uv = IN.tex01.xy;
    float4 curr = tex2D(DIFFUSEMAP_SAMPLER, uv);

    // Our MOTIONBLUR_TEXTURE is a *directionally blurred version of the current frame*.
    // Do NOT treat it as previous-frame history (temporal blend = global haze).
    float4 blurred = tex2D(MOTIONBLUR_SAMPLER, uv);

    // Debug modes (set from code via MotionBlurScale):
    // - > 2.5: visualize depth texture (DEPTHBUFFER_SAMPLER) as grayscale
    // - > 1.5: visualize abs(curr-blurred) as grayscale
    // - > 0.5: visualize blurred.a (history alpha) as grayscale
    if (MotionBlurScale > 2.5f)
    {
        float d = tex2D(DEPTHBUFFER_SAMPLER, uv).x;
        d = saturate(d);
        return float4(d, d, d, 1.0f);
    }
    if (MotionBlurScale > 1.5f)
    {
        float3 d = abs(curr.rgb - blurred.rgb);
        float v = saturate(dot(d, float3(0.3333, 0.3333, 0.3333)) * 4.0f);
        return float4(v, v, v, 1.0f);
    }
    if (MotionBlurScale > 0.5f)
    {
        float d = saturate(blurred.a);
        return float4(d, d, d, 1.0f);
    }

    // Amount cap: keep it subtle.
    float amt = saturate(MotionBlurBlend);
    amt = min(amt * 0.65, 0.20);

    // Edge mask (X360 vignette intent): blur mostly at edges.
    float2 center = float2(0.5, 0.55);
    float2 d = uv - center;
    float r = length(d);
    float edge = smoothstep(0.15, 0.55, r);
    edge *= edge;

    return lerp(curr, blurred, amt * edge);
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

technique composite
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader = compile ps_2_0 ps_composite();
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

    auto fileExists = [](const char* p) -> bool
    {
        if (!p || !*p) return false;
        DWORD attr = GetFileAttributesA(p);
        return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    };

    // Prefer file-based FX so ShaderLoader hot-reload can replace the shader without us hooking D3DX.
    // If missing or compile fails, fall back to the embedded string.
    {
        using D3DXCreateEffectFromFileAFn = HRESULT (WINAPI*)(
            LPDIRECT3DDEVICE9,
            LPCSTR,
            const D3DXMACRO*,
            LPD3DXINCLUDE,
            DWORD,
            LPD3DXEFFECTPOOL,
            LPD3DXEFFECT*,
            LPD3DXBUFFER*);

        HMODULE d3dx = GetModuleHandleA("d3dx9_43.dll");
        if (!d3dx)
            d3dx = LoadLibraryA("d3dx9_43.dll");
        if (d3dx)
        {
            auto createFromFile = reinterpret_cast<D3DXCreateEffectFromFileAFn>(
                GetProcAddress(d3dx, "D3DXCreateEffectFromFileA"));
            if (createFromFile)
            {
                // Prefer a shader next to our ASI (SCRIPTS folder) so edits actually take effect.
                // Fallback to the game's fx\motionblur.fx.
                char fxPathBuf[MAX_PATH]{};
                const char* fxPath = nullptr;

                if (g_RenderTargetManager.g_hModule)
                {
                    char modPath[MAX_PATH]{};
                    if (GetModuleFileNameA(g_RenderTargetManager.g_hModule, modPath, MAX_PATH))
                    {
                        // strip filename
                        for (int i = (int)strlen(modPath) - 1; i >= 0; --i)
                        {
                            if (modPath[i] == '\\' || modPath[i] == '/')
                            {
                                modPath[i] = '\0';
                                break;
                            }
                        }
                        // <moddir>\fx\motionblur.fx
                        sprintf_s(fxPathBuf, "%s\\fx\\motionblur.fx", modPath);
                        if (fileExists(fxPathBuf))
                            fxPath = fxPathBuf;
                    }
                }
                if (!fxPath && fileExists("fx\\motionblur.fx"))
                    fxPath = "fx\\motionblur.fx";

                static bool s_loggedFxPath = false;
                if (!s_loggedFxPath)
                {
                    s_loggedFxPath = true;
                    if (fxPath)
                        printf_s("[XNFS] ? motionblur.fx selected: %s\n", fxPath);
                    else
                        printf_s("[XNFS] ? motionblur.fx not found (expected <ASI dir>\\fx\\motionblur.fx or fx\\motionblur.fx)\n");
                }

                if (fxPath && fileExists(fxPath))
                {
                    ID3DXBuffer* errors = nullptr;
                    HRESULT hrFile = createFromFile(
                        device, fxPath, nullptr, nullptr,
                        D3DXSHADER_SKIPOPTIMIZATION, nullptr,
                        &g_RenderTargetManager.g_CustomBlurEffect, &errors);
                    if (SUCCEEDED(hrFile) && g_RenderTargetManager.g_CustomBlurEffect)
                    {
                        printf_s("[XNFS] ✅ Loaded custom blur FX from file: %s (%p)\n",
                                 fxPath, g_RenderTargetManager.g_CustomBlurEffect);
                        if (errors) errors->Release();
                        return true;
                    }
                    if (errors)
                    {
                        const char* errText = (const char*)errors->GetBufferPointer();
                        printf_s("[XNFS] ⚠️ motionblur.fx compile failed: %s\n", errText ? errText : "(null)");
                        errors->Release();
                    }
                }
            }
        }
    }

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
            const char* errText = (const char*)errors->GetBufferPointer();
            printf_s("[XNFS] ? Custom blur FX compile failed: %s\n", errText ? errText : "(null)");
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
