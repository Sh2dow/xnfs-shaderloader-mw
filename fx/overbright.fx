//
// Standard Effect
//
#include "global.h"

float4x4 WorldView : WORLDVIEW;
float4x4 World : WORLDMAT;
shared float4 TextureOffset : TEXTUREOFFSET;
shared float4x4 TextureOffsetMatrix : TEXTUREOFFSETMATRIX;
float4 DiffuseColour : DIFFUSECOLOUR;
float OverBrightOffset : OVERBRIGHTOFFSET;

float g_fVignetteScale = 1.0f;

float Brightness = 2.0; // STANDARD_BRIGHTNESS;

int BlendStateValues[5] : BLENDSTATE;
float4 FilterWeights[4] : FILTERWEIGHTS;

texture FilterTexture0 : FILTERTEXTURE0;
texture FilterTexture1 : FILTERTEXTURE1;
texture FilterTexture2 : FILTERTEXTURE2;
texture FilterTexture3 : FILTERTEXTURE3;

float4 OverBrightGreyScale : OVERBRIGHTGREYSCALE;

// ------------------------------------------------------------------
// Samplers
// ------------------------------------------------------------------
shared texture DIFFUSEMAP_TEXTURE : DiffuseMap;
sampler DIFFUSEMAP_SAMPLER = sampler_state
{
    Texture = <DIFFUSEMAP_TEXTURE>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture OPACITYMAP_TEXTURE : OPACITYMAPTEXTURE;
sampler OPACITYMAP_SAMPLER = sampler_state
{
    Texture = <OPACITYMAP_TEXTURE>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

shared texture MISCMAP2_TEXTURE : GAINMAP;
sampler MISCMAP2_SAMPLER = sampler_state
{
    Texture = <MISCMAP2_TEXTURE>;
    AddressU = CLAMP;
    AddressV = MIRROR;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

texture HEIGHTMAP_TEXTURE : HeightMapTexture;
sampler2D HEIGHTMAP_SAMPLER = sampler_state
{
    Texture   = <HEIGHTMAP_TEXTURE>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = <BaseMinTextureFilter>;
    MAGFILTER = <BaseMagTextureFilter>;
};

sampler2D filter_sampler0 = sampler_state {
    Texture = <FilterTexture0>;
    AddressU = 3;
    AddressV = 3;
    MipFilter = 0;
    MinFilter = 2;
    MagFilter = 2;
};

sampler filter_sampler1 = sampler_state {
    Texture = <FilterTexture1>;
    AddressU = 3;
    AddressV = 3;
    MipFilter = 0;
    MinFilter = 2;
    MagFilter = 2;
};

sampler filter_sampler2 = sampler_state {
    Texture = <FilterTexture2>;
    AddressU = 3;
    AddressV = 3;
    MipFilter = 0;
    MinFilter = 2;
    MagFilter = 2;
};

sampler filter_sampler3 = sampler_state {
    Texture = <FilterTexture3>;
    AddressU = 3;
    AddressV = 3;
    MipFilter = 0;
    MinFilter = 2;
    MagFilter = 2;
};

// Vertex shader - pass through texture coordinates
struct filter_VertexShader_Input {
    float4 position : SV_Position;
    float4 texcoord : TEXCOORD;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
    float4 texcoord3 : TEXCOORD3;
};

struct filter_VertexShader_Output {
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
    float2 texcoord1 : TEXCOORD1;
    float2 texcoord2 : TEXCOORD2;
    float2 texcoord3 : TEXCOORD3;
};

filter_VertexShader_Output filter_VertexShader(filter_VertexShader_Input i) {
    filter_VertexShader_Output o;
    o.position = i.position;
    o.texcoord = i.texcoord.xy;
    o.texcoord1 = i.texcoord1.xy;
    o.texcoord2 = i.texcoord2.xy;
    o.texcoord3 = i.texcoord3.xy;
    return o;
}

struct VS_INPUT
{
    float4 position : POSITION;
    float4 color : COLOR;
    float4 tex : TEXCOORD;
    float4 tex1 : TEXCOORD1;
    float3 normal : NORMAL;
};

struct VtoP
{
    float4 position : POSITION;
    float4 tex : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 color : COLOR0;
};

struct VtoP_Depth
{
    float4 position : POSITION;
    float dist : COLOR0;
};

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------
// fwidth is available in ps_3_0; fallback shown below if you prefer
float fwidth1(float x) { return abs(ddx(x)) + abs(ddy(x)); }

float2 GetInvSceneSize_B(float2 uv)
{
    return float2(fwidth1(uv.x), fwidth1(uv.y));
}
//-----------------------------------------------------------------------------
// Shaders
//
VtoP vertex_shader_passthru(const VS_INPUT IN)
{
    VtoP OUT;
    OUT.position = IN.position;
    OUT.tex = IN.tex;

    return OUT;
}

float4 PS_Overbright(const VtoP IN) : COLOR0
{
    float3 bloomAccum = 0.0;
    float totalWeight = 0.0;

    float2 dx = ddx(IN.tex.xy);
    float2 dy = ddy(IN.tex.xy);
    float2 texelSize = GetInvSceneSize_B(IN.tex.xy);

    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            float2 offsetUV =
                (g_avSampleOffsetsBloomPass1[j].xy +
                 g_avSampleOffsetsBloomPass2[i].xy)
                * texelSize * 1.4; // PC radius

            float3 s = tex2Dgrad(
                DIFFUSEMAP_SAMPLER,
                IN.tex.xy + offsetUV,
                dx, dy
            ).rgb;

            float lum = dot(s, LUMINANCE_VECTOR);

            // PC MW threshold (kills sky)
            float t = saturate((lum - 0.88) * 6.0);
            float b = pow(t, 1.0 / 2.2);

            float3 c =
                (s * (1.0 + b)) * 2.0 -
                (max(0.0, OverBrightOffset) + 1.0);

            c = max(c, 0.0.xxx); // EARLY CLAMP (PC)

            float w =
                g_avSampleWeightsBloom[i].x *
                g_avSampleWeightsBloom[j].x;

            bloomAccum += c * w;
            totalWeight += w;
        }
    }

    bloomAccum /= max(totalWeight, 1e-4);

    // PC clamp + shoulder
    bloomAccum = min(bloomAccum, 1.1.xxx);
    bloomAccum = bloomAccum / (1.0 + bloomAccum);

    // PC sticky pixels (mild)
    bloomAccum = floor(bloomAccum * 160.0) / 160.0;

    return float4(bloomAccum, 1.0);
}

technique overbright <int shader = 1;>
{
    pass p0
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

        VertexShader = compile vs_3_0 vertex_shader_passthru();
        PixelShader = compile ps_3_0 PS_Overbright();
    }
}