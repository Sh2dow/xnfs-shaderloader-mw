#include "global.h"

float4 FilterWeights[4] : FILTERWEIGHTS;
texture FilterTexture0 : FILTERTEXTURE0; // 1
texture FilterTexture1 : FILTERTEXTURE1; // 2
texture FilterTexture2 : FILTERTEXTURE2; // 3
texture FilterTexture3 : FILTERTEXTURE3; // 4
float OverBrightOffset : OVERBRIGHTOFFSET : register(ps_1_1, c0);
float4 OverBrightGreyScale : OVERBRIGHTGREYSCALE : register(ps_1_1, c1);

float4 DiffuseColour : DIFFUSECOLOUR;

#define LEGACY_BLOOM 0
// ------------------------------------------------------------------
// Samplers
// ------------------------------------------------------------------
sampler2D filter_sampler0 = sampler_state
{
    Texture = <FilterTexture0>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler filter_sampler1 = sampler_state
{
    Texture = <FilterTexture1>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler filter_sampler2 = sampler_state
{
    Texture = <FilterTexture2>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler filter_sampler3 = sampler_state
{
    Texture = <FilterTexture3>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

struct VS_INPUT
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
    float4 texcoord3 : TEXCOORD3;
};

struct VtoP
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float2 texcoord1 : TEXCOORD1;
    float2 texcoord2 : TEXCOORD2;
    float2 texcoord3 : TEXCOORD3;
};

VtoP vertex_shader_passthru(VS_INPUT IN)
{
    VtoP o;
    o.position = IN.position;
    o.texcoord = IN.texcoord;
    o.texcoord1 = IN.texcoord1;
    o.texcoord2 = IN.texcoord2;
    o.texcoord3 = IN.texcoord3;
    return o;
}

float4 filter_PixelShader(float4 texcoord : TEXCOORD) : COLOR
{
    // Sample source buffer
    float3 color = tex2D(filter_sampler0, texcoord.xy).rgb;

    // Threshold in RGB (overbright extraction)
    float3 bright = max(color - OverBrightOffset.x, 0.0);

    float intensity = dot(bright, OverBrightGreyScale.rgb);

#if LEGACY_BLOOM
    // Luminance as intensity mask (NOT as color)

    // Legacy bloom: preserve hue, scale by intensity
    float3 bloom = bright * intensity;

    return float4(bloom, 0.0);
#else
    return float4(intensity.xxx, 0);
#endif
}


technique filter < int shader = 1;>
{
    pass pContrast
    {
        FogEnable = 0;
        AlphaTestEnable = Blend_State[0];
        AlphaRef = Blend_State[1];
        AlphaBlendEnable = Blend_State[2];
        SrcBlend = Blend_State[3];
        DestBlend = Blend_State[4];
        TexCoordIndex[0] = 0;
        TexCoordIndex[1] = 1;
        TexCoordIndex[2] = 2;
        CullMode = 1;
        ZEnable = 0;
        ZWriteEnable = 0;
        VertexShader = compile vs_1_1 vertex_shader_passthru();
        PixelShader = compile ps_2_0 filter_PixelShader();
    }
}
