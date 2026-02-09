#include "global.h"

// Standalone custom motion blur effect used by NFSMW-RenderTarget-MW.
// This is intentionally separate from VisualTreatment so ShaderLoader hot-reload works cleanly.

texture DIFFUSEMAP_TEXTURE;     // scene color (full scene)
texture MOTIONBLUR_TEXTURE;     // blurred scene or history (depending on pass)
texture DEPTHBUFFER_TEXTURE;    // INTZ or fallback; optional
texture MOTIONBLUR_MASK_TEXTURE; // optional mask (BlurMask.png)

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

sampler2D MOTIONBLUR_MASK_SAMPLER = sampler_state
{
    Texture = <MOTIONBLUR_MASK_TEXTURE>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

float2 BlurTexelSize;
float MotionBlurBlend;
float2 MotionVec;
float MotionBlurScale; // also used for debug modes (see ps_composite)

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

float4 ps_motionblur(VS_OUT IN) : COLOR
{
    // Directional taps provided by vertex shader coords (XNFS-style RenderBlurOverride).
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
    // Use alpha as a lightweight debug/telemetry channel (history alpha view).
    c.a = saturate(MotionBlurBlend);
    return c;
}

float4 ps_copy(VS_OUT IN) : COLOR
{
    float4 c = tex2D(DIFFUSEMAP_SAMPLER, IN.tex01.xy);
    c.a = MotionBlurBlend;
    return c;
}

float4 ps_composite(VS_OUT IN) : COLOR
{
    float2 uv = IN.tex01.xy;
    float4 curr = tex2D(DIFFUSEMAP_SAMPLER, uv);
    float4 blurred = tex2D(MOTIONBLUR_SAMPLER, uv);

    float amt = saturate(MotionBlurBlend);
    // Masking is critical: VT's UVESVIGNETTE (VIGNETTETEX) encodes where blur is allowed.
    // Take max(r,a) to support both PC VT (.x/.w) and single-channel masks (BlurMask.png).
    float4 mt = tex2D(MOTIONBLUR_MASK_SAMPLER, uv);
    float m = saturate(max(mt.r, mt.a));

    // Hard guarantee: keep the center sharp even if the mask is accidentally ~white.
    // Center is biased downward like MW's UVESVIGNETTE.
    float2 d = uv - float2(0.50f, 0.65f);
    float dist = length(d);
    float radial = smoothstep(0.60f, 0.95f, dist);

    // Make blur strongly peripheral by default.
    m = saturate((m - 0.80f) / 0.20f);
    m = m * m;
    m *= radial;

    // Keep it sane: raw amount is usually 1.0 in vanilla; we shape it down and let the mask do the work.
    // For visibility during bring-up, allow a slightly stronger max. Real tuning later.
    float w = min(amt * 0.35, 0.25) * m;
    return lerp(curr, blurred, w);
}

float4 ps_dbg_curr(VS_OUT IN) : COLOR
{
    float4 c = tex2D(DIFFUSEMAP_SAMPLER, IN.tex01.xy);
    c.a = 1.0f;
    return c;
}

float4 ps_dbg_blur(VS_OUT IN) : COLOR
{
    float4 b = tex2D(MOTIONBLUR_SAMPLER, IN.tex01.xy);
    b.a = 1.0f;
    return b;
}

float4 ps_dbg_mask(VS_OUT IN) : COLOR
{
    float4 mt = tex2D(MOTIONBLUR_MASK_SAMPLER, IN.tex01.xy);
    float m = saturate(max(mt.r, mt.a));
    return float4(m, m, m, 1.0f);
}

float4 ps_dbg_depth(VS_OUT IN) : COLOR
{
    float d = tex2D(DEPTHBUFFER_SAMPLER, IN.tex01.xy).x;
    d = saturate(d);
    return float4(d, d, d, 1.0f);
}

float4 ps_dbg_alpha(VS_OUT IN) : COLOR
{
    float d = saturate(tex2D(MOTIONBLUR_SAMPLER, IN.tex01.xy).a);
    return float4(d, d, d, 1.0f);
}

float4 ps_dbg_diff(VS_OUT IN) : COLOR
{
    float2 uv = IN.tex01.xy;
    float3 a = tex2D(DIFFUSEMAP_SAMPLER, uv).rgb;
    float3 b = tex2D(MOTIONBLUR_SAMPLER, uv).rgb;
    float v = saturate(dot(abs(a - b), float3(0.3333, 0.3333, 0.3333)) * 4.0f);
    return float4(v, v, v, 1.0f);
}

float4 ps_tint(VS_OUT IN) : COLOR
{
    // Magenta: proves we're drawing to the target surface.
    return float4(1.0, 0.0, 1.0, 1.0);
}

technique motionblur
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_motionblur();
    }
}

technique copy
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_copy();
    }
}

technique composite
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_composite();
    }
}

technique dbg_curr { pass p0 { VertexShader = compile vs_2_0 vs_main(); PixelShader = compile ps_2_0 ps_dbg_curr(); } }
technique dbg_blur { pass p0 { VertexShader = compile vs_2_0 vs_main(); PixelShader = compile ps_2_0 ps_dbg_blur(); } }
technique dbg_mask { pass p0 { VertexShader = compile vs_2_0 vs_main(); PixelShader = compile ps_2_0 ps_dbg_mask(); } }
technique dbg_depth { pass p0 { VertexShader = compile vs_2_0 vs_main(); PixelShader = compile ps_2_0 ps_dbg_depth(); } }
technique dbg_alpha { pass p0 { VertexShader = compile vs_2_0 vs_main(); PixelShader = compile ps_2_0 ps_dbg_alpha(); } }
technique dbg_diff { pass p0 { VertexShader = compile vs_2_0 vs_main(); PixelShader = compile ps_2_0 ps_dbg_diff(); } }

technique tint
{
    pass p0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_tint();
    }
}
