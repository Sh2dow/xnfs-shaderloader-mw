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

shared texture MOTIONBLUR_TEXTURE : MOTIONBLUR;
sampler2D MOTIONBLUR_SAMPLER = sampler_state
{
    ASSIGN_TEXTURE(MOTIONBLUR_TEXTURE)
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

struct VS_INPUT_MOTIONBLUR
{
    float4 position : POSITION;
    float4 tex0 : TEXCOORD0;
    float4 tex1 : TEXCOORD1;
    float4 tex2 : TEXCOORD2;
    float4 tex3 : TEXCOORD3;
    float4 tex4 : TEXCOORD4;
    float4 tex5 : TEXCOORD5;
    float4 tex6 : TEXCOORD6;
    float4 tex7 : TEXCOORD7;
};

struct VtoP_MOTIONBLUR
{
    float4 position : POSITION;
    float2 tex[8] : TEXCOORD0;
};

VtoP_MOTIONBLUR VS_MotionBlur(const VS_INPUT_MOTIONBLUR IN)
{
    VtoP_MOTIONBLUR OUT;
    OUT.position = IN.position;
    OUT.tex[0] = IN.tex0.xy;
    OUT.tex[1] = IN.tex1.xy;
    OUT.tex[2] = IN.tex2.xy;
    OUT.tex[3] = IN.tex3.xy;
    OUT.tex[4] = IN.tex4.xy;
    OUT.tex[5] = IN.tex5.xy;
    OUT.tex[6] = IN.tex6.xy;
    OUT.tex[7] = IN.tex7.xy;
    return OUT;
}

const float kWeights[8] = {6, 5, 4, 3, 2, 1, 1, 1};
float4 PS_MotionBlur(const VtoP_MOTIONBLUR IN) : COLOR
{
    float4 result = 0;
    for (int i = 0; i < 8; ++i)
    {
        float4 blur = tex2D(MOTIONBLUR_SAMPLER, IN.tex[i]);
        result += blur * (kWeights[i] / 23.0f);
    }
    return result;
}

VtoP VS_CompositeBlur(const VS_INPUT IN)
{
    VtoP OUT;
    OUT.position = IN.position;
    OUT.texcoord = IN.texcoord;
    OUT.texcoord1 = IN.texcoord1;
    OUT.color = IN.color * DiffuseColour;
    return OUT;
}

float4 PS_CompositeBlur(const VtoP IN) : COLOR
{
    float4 screen = tex2D(DIFFUSEMAP_SAMPLER, IN.texcoord.xy);
    float4 blur = tex2D(MOTIONBLUR_SAMPLER, IN.texcoord.xy);
    float senseOfSpeed = saturate(XNFS_MotionBlurAmount);
    float3 blended = lerp(screen.xyz, blur.xyz, senseOfSpeed);
    return float4(blended, screen.w);
}

float4 PS_VisualTreatment_Low1(VS_INPUT i) : COLOR
{
    // FULL RES SCENE
    float4 scene = tex2D(DIFFUSEMAP_SAMPLER, i.texcoord.xy);

    float4 spline = tex2D(MISCMAP3_SAMPLER, float2(scene.a, scene.r));
    float splineR = spline.r;

    float lum = dot(scene.rgb, LUMINANCE_VECTOR);

    float3 desat =
        scene.rgb * Desaturation_0.rgb +
        lum.xxx  * Desaturation_1.rgb;

    float blackBloom =
        splineR * BlackBloomIntensity_0.x +
        BlackBloomIntensity_1.x;

    return float4(desat * blackBloom, 1.0);
}

float4 PS_VisualTreatment_Low2(VS_INPUT i) : COLOR
{
    // Full-resolution scene
    float4 scene = tex2D(DIFFUSEMAP_SAMPLER, i.texcoord.xy);

    // Spline LUT (uses scene.a, scene.r like PC VT)
    float2 splineUV = float2(scene.a, scene.r);
    float splineG = tex2D(MISCMAP3_SAMPLER, splineUV).g;

    // Colour bloom contribution
    float bloom = ColourBloomIntensity * splineG;
    float3 add = scene.rgb * (ColourBloomTint.rgb * bloom);

    // Vignette comes from GAINMAP
    float vignette = tex2D(MISCMAP2_SAMPLER, i.texcoord.xy).r;

    float3 outRgb =
        scene.rgb +
        add +
        vignette * VisualEffectVignette;

    float4 blur = tex2D(MOTIONBLUR_SAMPLER, i.texcoord.xy);
    outRgb = lerp(outRgb, blur.rgb, saturate(XNFS_MotionBlurAmount));

    return float4(outRgb, 1.0);
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

    float4 blur = tex2D(MOTIONBLUR_SAMPLER, uv);
    outRgb = lerp(outRgb, blur.rgb, saturate(XNFS_MotionBlurAmount));

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

technique vt <int shader = 3;>
{
    pass p1
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 vertex_shader_passthru();
        PixelShader = compile ps_2_0 PS_VisualTreatment();
    }
}

technique visualtreatment_branching <int shader = 1;>
{
    pass p1
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 vertex_shader_passthru();
        PixelShader = compile ps_2_0 PS_VisualTreatment_Low1();
    }
    pass p2
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 vertex_shader_passthru();
        PixelShader = compile ps_2_0 PS_VisualTreatment_Low2();
    }
}

technique motionblur <int shader = 1;>
{
    pass p0
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 VS_MotionBlur();
        PixelShader = compile ps_2_0 PS_MotionBlur();
    }
}

technique composite_blur <int shader = 1;>
{
    pass p0
    {
        COMMON_VT_PASS_STATE
        VertexShader = compile vs_1_1 VS_CompositeBlur();
        PixelShader = compile ps_2_0 PS_CompositeBlur();
    }
}
