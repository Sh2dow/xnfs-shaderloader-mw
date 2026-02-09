#include "global.h"

float4 BlackBloomIntensity;
float ColourBloomIntensity = {0.15};
float DetailMapIntensity = {1};
float VisualEffectBrightness;
float VisualEffectRadialBlur;
float VisualEffectVignette = {1};
float RadialBlurOffset = {0.1};
float4 ColourBloomTint = {0.517, 0.8, 0.9, 1};
float4 Desaturation;
float4 Desaturation_0;
float4 Desaturation_1;
float4 BlackBloomIntensity_0;
float4 BlackBloomIntensity_1;

float4 DiffuseColour : DIFFUSECOLOUR;
float XNFS_MotionBlurAmount;
float2 XNFS_TexelSize;      // (1/width, 1/height)
float2 XNFS_BlurDir;        // e.g. (1,0) or from motion vectors
float  XNFS_BlurScale;      // strength
// ------------------------------------------------------------------
// Samplers
// ------------------------------------------------------------------
shared texture DIFFUSEMAP_TEXTURE : DiffuseMap;
sampler2D DIFFUSEMAP_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(DIFFUSEMAP_TEXTURE)
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture MISCMAP1_TEXTURE : DIFFUSETEX;
sampler2D MISCMAP1_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(MISCMAP1_TEXTURE)
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture MISCMAP2_TEXTURE : GAINMAP;
sampler2D MISCMAP2_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(MISCMAP2_TEXTURE)
    AddressU = CLAMP;
    AddressV = WRAP; // Use mirror for split screen so the vignette works
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

// Game-side vignette mask texture (UVESVIGNETTE). The engine loads this by name and binds it to a parameter
// typically called VIGNETTETEX in the original shader setup. This is the correct mask for radial/motion blur
// falloff; do not approximate it with GAINMAP (often near-white) or you'll get fullscreen haze.
shared texture VIGNETTETEX : VIGNETTETEX;
sampler2D VIGNETTETEX_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(VIGNETTETEX)
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture MISCMAP3_TEXTURE : SPLINE;
sampler2D MISCMAP3_SAMPLER = sampler_state // BLOOM_SAMPLER
{
    ASSIGN_TEXTURE(MISCMAP3_TEXTURE)
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture HEIGHTMAP_TEXTURE : HeightMapTexture;
sampler2D HEIGHTMAP_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(HEIGHTMAP_TEXTURE)
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture MOTIONBLUR;
sampler2D MOTIONBLUR_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(MOTIONBLUR)
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

struct VS_INPUT
{
    float4 position : POSITION;
    float4 color : COLOR;
    float4 texcoord : TEXCOORD;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
};

struct VtoP
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
};

VtoP vertex_shader_passthru(VS_INPUT IN)
{
    VtoP o;
    o.position = IN.position;
    o.texcoord = IN.texcoord;
    o.texcoord1 = IN.texcoord1;
    o.color = IN.color * DiffuseColour;
    // o.color = IN.color;
    return o;
}

float4 PS_VisualTreatment(float2 uv : TEXCOORD) : COLOR
{
    // MISCMAP1 is the packed control/source in vanilla
    float4 misc1 = tex2D(MISCMAP1_SAMPLER, uv);

    // vignette mask is misc1.r (temp0.y = temp1.x)
    float vignetteMask = misc1.r;

    // spline UV is (misc1.a, misc1.r) (temp0.x=temp1.w, temp0.y=temp1.x)
    float2 splineUV = float2(misc1.a, misc1.r);
    float4 spline = tex2D(MISCMAP3_SAMPLER, splineUV);

    // gain map used as multiplicative intensity at the end (temp0 = tex2D(MISCMAP2))
    float4 gain = tex2D(MISCMAP2_SAMPLER, uv);

    // Base scene: in your “fixed downscale” world, this must be DIFFUSEMAP full-res
    float3 scene = tex2D(DIFFUSEMAP_SAMPLER, uv).rgb;

    float lum = dot(scene, LUMINANCE_VECTOR);

    // desaturation (keep same structure as vanilla)
    float3 desat = Desaturation.xxx * scene + (lum * Desaturation.y).xxx;

    // black bloom scalar from spline.r (x)
    float blackBloom = spline.x * BlackBloomIntensity.x + BlackBloomIntensity.y;

    // colour bloom scalar from spline.g (y)
    float colourBloom = spline.y * ColourBloomIntensity;

    float3 outRgb = desat * blackBloom + scene * (ColourBloomTint.rgb * colourBloom);

    // ✅ vanilla vignette add (from misc1.r, not gain.r)
    outRgb += vignetteMask * VisualEffectVignette;

    // brightness
    outRgb *= VisualEffectBrightness;

    // ✅ vanilla final multiplier comes from gain.b (temp0.z)
    outRgb *= gain.b;

    // Temporal blur blend (prev-frame buffer)
    float4 blurSample = tex2D(MOTIONBLUR_SAMPLER, uv);
    // UVESVIGNETTE provides the intended radial/motion blur falloff mask.
    float4 uve = tex2D(VIGNETTETEX_SAMPLER, uv);
    float depth = tex2D(HEIGHTMAP_SAMPLER, uv).x;
    float zDist = (1 / (1 - depth));
    float blurDepth = saturate(-zDist / 300 + 1.2);
    float motionBlurMask = saturate(uve.x) * blurDepth * XNFS_MotionBlurAmount;
    float radialBlurMask = uve.w * XNFS_MotionBlurAmount;
    float blurAmount = saturate(motionBlurMask + radialBlurMask);
    outRgb = lerp(outRgb, blurSample.rgb, blurAmount);

    return float4(outRgb, 1.0);
}

// ------------------------------------------------------------------
// Common pass state macro
// ------------------------------------------------------------------
#define COMMON_VT_PASS_STATE \
    ColorWriteEnable = 7;    \
    AlphaTestEnable = 0;     \
    AlphaTestEnable = 0;     \
    ZFunc = 4;               \
    CullMode = 1;


technique vt
{
    pass p0
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 vertex_shader_passthru();
        PixelShader = compile ps_2_0 PS_VisualTreatment();
    }
}


technique visualtreatment_branching <int shader = 1;>
{
    pass p0s
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 vertex_shader_passthru();
        PixelShader = compile ps_2_0 PS_VisualTreatment();
    }
}
