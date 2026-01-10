int Blend_State[5] : BLENDSTATE;
int Cull_Mode : CULL_MODE;
int BaseMinTextureFilter : BASEMINTEXTUREFILTER;
int BaseMagTextureFilter : BASEMAGTEXTUREFILTER;
int BaseMipTextureFilter : BASEMIPTEXTUREFILTER;
int BaseAddressU : BASEADDRESSU;
int BaseAddressV : BASEADDRESSV;
int ColorWriteMode : COLORWRITEMODE;
column_major float4x4 WorldViewProj : WORLDVIEWPROJECTION : register(vs_3_0, c0) : register(vs_1_1, c0);
float4 ScreenOffset : SCREENOFFSET;
column_major float4x4 WorldView : WORLDVIEW : register(vs_3_0, c8);
column_major float4x4 LocalDirectionMatrix : LOCALDIRECTIONMATRIX;
column_major float4x4 LocalColourMatrix : LOCALCOLOURMATRIX : register(vs_3_0, c11);
float4 LocalEyePos : LOCALEYEPOS;
float MetallicScale : METALLICSCALE : register(ps_3_0, c3);
float SpecularHotSpot : SPECULARHOTSPOT : register(vs_3_0, c18) : register(ps_3_0, c4);
float4 DiffuseMin : DIFFUSEMIN;
float4 DiffuseRange : DIFFUSERANGE;
float4 SpecularMin : SPECULARMIN;
float4 SpecularRange : SPECULARRANGE;
float4 EnvmapMin : ENVMAPMIN : register(vs_3_0, c23) : register(ps_3_0, c5);
float4 EnvmapRange : ENVMAPANGE : register(vs_3_0, c24) : register(ps_3_0, c6);
float SpecularPower : SPECULARPOWER : register(ps_3_0, c7);
float EnvmapPower : ENVMAPPOWER;
float4 ShadowColour : CARSHADOWCOLOUR : register(ps_3_0, c8);
int g_bDoCarShadowMap : SHADOWMAP_CAR_SHADOW_ENABLED : register(ps_3_0, c9);
float FocalRange : FOCALRANGE;
float RVMSkyBrightness : RVM_SKY_BRIGHTNESS;
float RVMWorldBrightness : RVM_WORLD_BRIGHTNESS;
float Desaturation : DESATURATION;
float4 Coeffs0 : CURVE_COEFFS_0;
float4 Coeffs1 : CURVE_COEFFS_1;
float4 Coeffs2 : CURVE_COEFFS_2;
float4 Coeffs3 : CURVE_COEFFS_3;
float CombinedBrightness : COMBINED_BRIGHTNESS;
bool IS_NORMAL_MAPPED = { 1 };
bool HAS_METALIC_FLAKE;
float3 LUMINANCE_VECTOR = { 0.2125, 0.7154, 0.0721 };
texture DIFFUSEMAP_TEXTURE : DiffuseMap; // 1
sampler2D DIFFUSEMAP_SAMPLER : register(ps_3_0, s0) =
sampler_state
{
    Texture = <DIFFUSEMAP_TEXTURE>; // 2
    AddressU = 0;
    AddressV = 0;
    MipFilter = 0;
    MinFilter = 0;
    MagFilter = 0;
};
texture MISCMAP1_TEXTURE : MISCMAP1; // 3
sampler MISCMAP1_SAMPLER =
sampler_state
{
    Texture = <MISCMAP1_TEXTURE>; // 4
    AddressU = 0;
    AddressV = 0;
    MipFilter = 0;
    MinFilter = 0;
    MagFilter = 0;
};
texture NORMALMAP_TEXTURE : NormalMapTexture; // 5
sampler2D NORMALMAP_SAMPLER : register(ps_3_0, s1) =
sampler_state
{
    Texture = <NORMALMAP_TEXTURE>; // 6
    AddressU = 0;
    AddressV = 0;
    MipFilter = 0;
    MinFilter = 0;
    MagFilter = 0;
};
texture VOLUMEMAP_TEXTURE : VolumeMapTexture; // 7
sampler3D VOLUMEMAP_SAMPLER : register(ps_3_0, s2) =
sampler_state
{
    Texture = <VOLUMEMAP_TEXTURE>; // 8
    AddressU = 1;
    AddressV = 1;
    AddressW = 1;
    MipFilter = 1;
    MinFilter = 1;
    MagFilter = 1;
};
texture ENVIROMAP_TEXTURE : EnvMapTexture; // 9
samplerCUBE ENVIROMAP_SAMPLER =
sampler_state
{
    Texture = <ENVIROMAP_TEXTURE>; // 10
    AddressU = 2;
    AddressV = 2;
    MipFilter = 0;
    MinFilter = 0;
    MagFilter = 0;
};
float SHADOW_EPSILON = { 5e-05 };
texture ShadowMap : SHADOWMAP; // 11
sampler2D SHADOWMAP_SAMPLER : register(ps_3_0, s4) =
sampler_state
{
    Texture = <ShadowMap>; // 12
    BorderColor = 0xFFFFFFFF;
    AddressU = 4;
    AddressV = 4;
    MipFilter = 2;
    MinFilter = 1;
    MagFilter = 1;
};
texture ShadowColorMap : SHADOWCOLORMAP; // 13
sampler SHADOWCOLORMAP_SAMPLER =
sampler_state
{
    Texture = <ShadowColorMap>; // 14
    BorderColor = 0xFFFFFFFF;
    AddressU = 4;
    AddressV = 4;
    MipFilter = 2;
    MinFilter = 1;
    MagFilter = 1;
};
column_major float4x4 matShadowMapWVP : SHADOWTRANSFORM : register(vs_3_0, c4);
float g_fShadowMapAlphaMin : SHADOWMAP_ALPHAMIN;
row_major float4x4 g_matWorld : WORLD;
float g_fInvShadowStrength : INVSHADOWSTRENGTH;
float g_fShadowMapBias : SHADOWMAP_BIAS : register(ps_3_0, c10);
float g_fShadowMapScaleX : SHADOWMAP_SCALE_X : register(ps_3_0, c11);
float g_fShadowMapScaleY : SHADOWMAP_SCALE_Y : register(ps_3_0, c12);
int g_iShadowMapEnabled : SHADOWMAP_ENABLED : register(ps_3_0, c13);
int g_bShadowMapAlphaEnabled : SHADOWMAP_ALPHA_ENABLED;
int g_iShadowMapPCFLevel : SHADOWMAP_PCF_LEVEL;
float4 AmbientColour : AMBIENTCOLOUR;
float g_fLodDistance : SHADOWMAP_LOD_DISTANCE;
float g_fDiffuseMapWidth : DIFFUSEMAP_WIDTH;
float g_fDiffuseMapHeight : DIFFUSEMAP_HEIGHT;
float kShadowMapFallOff = { 0.85 };
// RenderWhiteAlphaTex_PixelShader1 Pixel_3_0 Has PRES False
float4 RenderWhiteAlphaTex_PixelShader1(float2 texcoord : TEXCOORD) : COLOR
{
    float4 out_color;
    float4 temp0;
    // def c1, -1, -0, 1, 0
    // dcl_texcoord v0.xy
    // dcl_2d s0
    // texld r0, v0, s0
    temp0 = tex2D(DIFFUSEMAP_SAMPLER, texcoord.xy);
    // add r0.x, -r0.w, c0.x
    temp0.x = -temp0.w + g_fShadowMapAlphaMin.x;
    // cmp r0, r0.x, c1.x, c1.y
    temp0 = (temp0.x >= 0) ? float4(-1, -1, -1, -1) : float4(-0, -0, -0, -0);
    // texkill r0
    clip(temp0);
    // mov oC0, c1.z
    out_color = float4(1, 1, 1, 1);
    // 

    return out_color;
}

// RenderWhiteAlphaTex_VertexShader2 Vertex_3_0 Has PRES False
struct RenderWhiteAlphaTex_VertexShader2_Input
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
};

struct RenderWhiteAlphaTex_VertexShader2_Output
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

RenderWhiteAlphaTex_VertexShader2_Output RenderWhiteAlphaTex_VertexShader2(RenderWhiteAlphaTex_VertexShader2_Input i)
{
    RenderWhiteAlphaTex_VertexShader2_Output o;
    // def c4, 1, 0, 0, 0
    // dcl_position v0
    // dcl_texcoord v1
    // dcl_position o0
    // dcl_color o1
    // dcl_texcoord o2.xy
    // dp4 o0.x, v0, c0
    o.position.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 o0.y, v0, c1
    o.position.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 o0.z, v0, c2
    o.position.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 o0.w, v0, c3
    o.position.w = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mov o1, c4.x
    o.color = float4(1, 1, 1, 1);
    // mov o2.xy, v1
    o.texcoord = i.texcoord;
    // 

    return o;
}

// RenderWhiteAlphaTex_Expression3 Expression_2_0 Has PRES False
float RenderWhiteAlphaTex_Expression3()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = ColorWriteMode.x;
    return expr0;
}

// RenderTexelDensity_PixelShader4 Pixel_3_0 Has PRES False
float4 RenderTexelDensity_PixelShader4(float2 texcoord : TEXCOORD) : COLOR
{
    float4 out_color;
    float3 temp0;
    // def c2, 0.1, 0.5, 2, 1
    // dcl_texcoord v0.xy
    // mul r0.x, c1.x, v0.y
    temp0.x = g_fDiffuseMapHeight.x * texcoord.y;
    // mul r0.x, r0.x, c2.x
    temp0.x = temp0.x * float1(0.1);
    // frc r0.y, r0.x
    temp0.y = frac(temp0.x);
    // add r0.x, -r0.y, r0.x
    temp0.x = -temp0.y + temp0.x;
    // mul r0.y, r0.x, c2.y
    temp0.y = temp0.x * float1(0.5);
    // frc r0.y, r0_abs.y
    temp0.y = frac(abs(temp0).y);
    // cmp r0.x, r0.x, r0.y, -r0.y
    temp0.x = (temp0.x >= 0) ? temp0.y : -temp0.y;
    // add r0.x, r0.x, r0.x
    temp0.x = temp0.x + temp0.x;
    // mul r0.y, c0.x, v0.x
    temp0.y = g_fDiffuseMapWidth.x * texcoord.x;
    // mul r0.y, r0.y, c2.x
    temp0.y = temp0.y * float1(0.1);
    // frc r0.z, r0.y
    temp0.z = frac(temp0.y);
    // add r0.y, -r0.z, r0.y
    temp0.y = -temp0.z + temp0.y;
    // mul r0.z, r0.y, c2.y
    temp0.z = temp0.y * float1(0.5);
    // frc r0.z, r0_abs.z
    temp0.z = frac(abs(temp0).z);
    // cmp r0.y, r0.y, r0.z, -r0.z
    temp0.y = (temp0.y >= 0) ? temp0.z : -temp0.z;
    // mad r0.x, r0.y, c2.z, r0.x
    temp0.x = temp0.y * float1(2) + temp0.x;
    // mul r0.y, r0.x, c2.y
    temp0.y = temp0.x * float1(0.5);
    // frc r0.y, r0_abs.y
    temp0.y = frac(abs(temp0).y);
    // cmp r0.x, r0.x, r0.y, -r0.y
    temp0.x = (temp0.x >= 0) ? temp0.y : -temp0.y;
    // add oC0.xyz, r0.x, r0.x
    out_color.xyz = temp0.x + temp0.x;
    // mov oC0.w, c2.w
    out_color.w = float1(1);
    // 

    return out_color;
}

// RenderTexelDensity_VertexShader5 Vertex_3_0 Has PRES False
struct RenderTexelDensity_VertexShader5_Input
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
};

struct RenderTexelDensity_VertexShader5_Output
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

RenderTexelDensity_VertexShader5_Output RenderTexelDensity_VertexShader5(RenderTexelDensity_VertexShader5_Input i)
{
    RenderTexelDensity_VertexShader5_Output o;
    // def c4, 1, 0, 0, 0
    // dcl_position v0
    // dcl_texcoord v1
    // dcl_position o0
    // dcl_color o1
    // dcl_texcoord o2.xy
    // dp4 o0.x, v0, c0
    o.position.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 o0.y, v0, c1
    o.position.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 o0.z, v0, c2
    o.position.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 o0.w, v0, c3
    o.position.w = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mov o1, c4.x
    o.color = float4(1, 1, 1, 1);
    // mov o2.xy, v1
    o.texcoord = i.texcoord;
    // 

    return o;
}

// RenderTexelDensity_Expression6 Expression_2_0 Has PRES False
float RenderTexelDensity_Expression6()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = ColorWriteMode.x;
    return expr0;
}

// RenderWhite_PixelShader7 Pixel_3_0 Has PRES False
float4 RenderWhite_PixelShader7() : COLOR
{
    float4 out_color;
    // def c0, 1, 0, 0, 0
    // mov oC0, c0.x
    out_color = float4(1, 1, 1, 1);
    // 

    return out_color;
}

// RenderWhite_VertexShader8 Vertex_3_0 Has PRES False
struct RenderWhite_VertexShader8_Output
{
    float4 position : POSITION;
    float4 color : COLOR;
};

RenderWhite_VertexShader8_Output RenderWhite_VertexShader8(float4 position : POSITION)
{
    RenderWhite_VertexShader8_Output o;
    // def c4, 1, 0, 0, 0
    // dcl_position v0
    // dcl_position o0
    // dcl_color o1
    // dp4 o0.x, v0, c0
    o.position.x = dot(position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 o0.y, v0, c1
    o.position.y = dot(position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 o0.z, v0, c2
    o.position.z = dot(position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 o0.w, v0, c3
    o.position.w = dot(position, (WorldViewProj._m03_m13_m23_m33));
    // mov o1, c4.x
    o.color = float4(1, 1, 1, 1);
    // 

    return o;
}

// RenderWhite_Expression9 Expression_2_0 Has PRES False
float RenderWhite_Expression9()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = ColorWriteMode.x;
    return expr0;
}

// RenderLightAlphaTex_PixelShader10 Pixel_3_0 Has PRES False
float4 RenderLightAlphaTex_PixelShader10(float2 texcoord : TEXCOORD) : COLOR
{
    float4 out_color;
    float4 temp0;
    // def c2, -1, -0, 1, 0
    // dcl_texcoord v0.xy
    // dcl_2d s0
    // if_ne c1.x, -c1.x
    if (g_bShadowMapAlphaEnabled.x != -g_bShadowMapAlphaEnabled.x) {
        // texld r0, v0, s0
        temp0 = tex2D(DIFFUSEMAP_SAMPLER, texcoord.xy);
        // add r0.x, -r0.w, c0.x
        temp0.x = -temp0.w + g_fShadowMapAlphaMin.x;
        // cmp r0, r0.x, c2.x, c2.y
        temp0 = (temp0.x >= 0) ? float4(-1, -1, -1, -1) : float4(-0, -0, -0, -0);
        // texkill r0
        clip(temp0);
        // endif
    }
    // mov oC0, c2.z
    out_color = float4(1, 1, 1, 1);
    // 

    return out_color;
}

// RenderLightAlphaTex_VertexShader11 Vertex_3_0 Has PRES False
struct RenderLightAlphaTex_VertexShader11_Input
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
};

struct RenderLightAlphaTex_VertexShader11_Output
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
    float2 texcoord1 : TEXCOORD1;
};

RenderLightAlphaTex_VertexShader11_Output RenderLightAlphaTex_VertexShader11(RenderLightAlphaTex_VertexShader11_Input i)
{
    RenderLightAlphaTex_VertexShader11_Output o;
    float4 temp0;
    // def c4, 1, 0, 0, 0
    // dcl_position v0
    // dcl_texcoord v1
    // dcl_position o0
    // dcl_texcoord o1
    // dcl_texcoord1 o2.xy
    // dp4 o0.x, v0, c0
    o.position.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 o0.y, v0, c1
    o.position.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 r0.z, v0, c2
    temp0.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 r0.w, v0, c3
    temp0.w = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mov o0.zw, r0
    o.position.zw = temp0.zw;
    // mov o2.xy, r0.zwzw
    o.texcoord1 = temp0.zwzw;
    // mad o1, v1.xyxx, c4.xxyy, c4.yyxx
    o.texcoord = i.texcoord.xyxx * float4(1, 1, 0, 0) + float4(0, 0, 1, 1);
    // 

    return o;
}

// RenderLightAlphaTex_Expression12 Expression_2_0 Has PRES False
float RenderLightAlphaTex_Expression12()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = ColorWriteMode.x;
    return expr0;
}

// RenderLight_VertexShader13 Vertex_1_1 Has PRES False
struct RenderLight_VertexShader13_Input
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
};

struct RenderLight_VertexShader13_Output
{
    float4 position : POSITION;
    float2 texcoord1 : TEXCOORD1;
    float4 texcoord : TEXCOORD;
};

RenderLight_VertexShader13_Output RenderLight_VertexShader13(RenderLight_VertexShader13_Input i)
{
    RenderLight_VertexShader13_Output o;
    float4 temp0;
    // def c4, 1, 0, 0, 0
    // dcl_position v0
    // dcl_texcoord v1
    // dp4 oPos.x, v0, c0
    o.position.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 oPos.y, v0, c1
    o.position.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 r0.z, v0, c2
    temp0.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 r0.w, v0, c3
    temp0.w = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mov oPos.zw, r0
    o.position.zw = temp0.zw;
    // mov oT1.xy, r0.zwzw
    o.texcoord1 = temp0.zwzw;
    // mad oT0, v1.xyxx, c4.xxyy, c4.yyxx
    o.texcoord = i.texcoord.xyxx * float4(1, 1, 0, 0) + float4(0, 0, 1, 1);
    // 

    return o;
}

// RenderVertexColour_PixelShader14 Pixel_2_0 Has PRES False
float4 RenderVertexColour_PixelShader14(float4 color : COLOR) : COLOR
{
    float4 out_color;
    // dcl v0
    // mov oC0, v0
    out_color = color;
    // 

    return out_color;
}

// RenderVertexColour_VertexShader15 Vertex_1_1 Has PRES False
struct RenderVertexColour_VertexShader15_Input
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct RenderVertexColour_VertexShader15_Output
{
    float4 position : POSITION;
    float4 color : COLOR;
};

RenderVertexColour_VertexShader15_Output RenderVertexColour_VertexShader15(RenderVertexColour_VertexShader15_Input i)
{
    RenderVertexColour_VertexShader15_Output o;
    // dcl_position v0
    // dcl_color v1
    // dp4 oPos.x, v0, c0
    o.position.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 oPos.y, v0, c1
    o.position.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 oPos.z, v0, c2
    o.position.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 oPos.w, v0, c3
    o.position.w = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mov oD0, v1
    o.color = i.color;
    // 

    return o;
}

// ZPrePass_VertexShader16 Vertex_1_1 Has PRES False
float4 ZPrePass_VertexShader16(float4 position : POSITION) : POSITION
{
    float4 out_position;
    // dcl_position v0
    // dp4 oPos.x, v0, c0
    out_position.x = dot(position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 oPos.y, v0, c1
    out_position.y = dot(position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 oPos.z, v0, c2
    out_position.z = dot(position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 oPos.w, v0, c3
    out_position.w = dot(position, (WorldViewProj._m03_m13_m23_m33));
    // 

    return out_position;
}

// ZPrePass_Expression17 Expression_2_0 Has PRES False
float ZPrePass_Expression17()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = Cull_Mode.x;
    return expr0;
}

// lowlod_PixelShader18 Pixel_3_0 Has PRES False
struct lowlod_PixelShader18_Input
{
    float4 color : COLOR;
    float4 color1 : COLOR1;
    float3 texcoord : TEXCOORD;
    float2 texcoord1 : TEXCOORD1;
};

float4 lowlod_PixelShader18(lowlod_PixelShader18_Input i) : COLOR
{
    float4 out_color;
    float4 temp0, temp1;
    float3 temp2;
    // dcl_color_pp v0
    // dcl_color1_pp v1
    // dcl_texcoord_pp v2.xyz
    // dcl_texcoord1_pp v3.xy
    // dcl_2d s0
    // dcl_cube s1
    // texld_pp r0, v2, s1
    temp0 = /* not implemented _pp modifier */ texCUBE(ENVIROMAP_SAMPLER, i.texcoord.xyz);
    // mul_pp r0.xyz, r0, v1.w
    temp0.xyz = /* not implemented _pp modifier */ temp0.xyz * i.color1.www;
    // texld_pp r1, v3, s0
    temp1 = /* not implemented _pp modifier */ tex2D(DIFFUSEMAP_SAMPLER, i.texcoord1.xy);
    // mov_pp r2.xyz, v0
    temp2.xyz = /* not implemented _pp modifier */ i.color.xyz;
    // mad_pp r1.xyz, r2, r1, v1
    temp1.xyz = /* not implemented _pp modifier */ temp2.xyz * temp1.xyz + i.color1.xyz;
    // mul_pp oC0.w, r1.w, v0.w
    out_color.w = /* not implemented _pp modifier */ temp1.w * i.color.w;
    // mad_pp oC0.xyz, v0.w, r0, r1
    out_color.xyz = /* not implemented _pp modifier */ i.color.www * temp0.xyz + temp1.xyz;
    // 

    return out_color;
}

// lowlod_VertexShader19 Vertex_3_0 Has PRES False
struct lowlod_VertexShader19_Input
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 color : COLOR;
    float4 texcoord : TEXCOORD;
};

struct lowlod_VertexShader19_Output
{
    float4 position : POSITION;
    float4 color : COLOR;
    float4 color1 : COLOR1;
    float4 texcoord : TEXCOORD;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
    float4 texcoord3 : TEXCOORD3;
    float4 texcoord4 : TEXCOORD4;
    float4 texcoord5 : TEXCOORD5;
    float4 texcoord6 : TEXCOORD6;
};

lowlod_VertexShader19_Output lowlod_VertexShader19(lowlod_VertexShader19_Input i)
{
    lowlod_VertexShader19_Output o;
    float4 temp0, temp1, temp2, temp3;
    float3 temp4;
    // def c27, 1, 2, -1, 0.01
    // def c28, 0.3, 100, 0.7, 0
    // dcl_position v0
    // dcl_normal v1
    // dcl_color v2
    // dcl_texcoord v3
    // dcl_position o0
    // dcl_color o1
    // dcl_color1 o2
    // dcl_texcoord o3
    // dcl_texcoord1 o4
    // dcl_texcoord2 o5
    // dcl_texcoord3 o6
    // dcl_texcoord4 o7
    // dcl_texcoord5 o8
    // dcl_texcoord6 o9
    // dp4 o0.z, v0, c2
    o.position.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 r0.x, v0, c0
    temp0.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 r0.y, v0, c1
    temp0.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 r0.z, v0, c3
    temp0.z = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mad o0.xy, c16, r0.z, r0
    o.position.xy = ScreenOffset.xy * temp0.zz + temp0.xy;
    // mov o0.w, r0.z
    o.position.w = temp0.z;
    // dp4 o5.z, v0, c6
    o.texcoord2.z = dot(i.position, (matShadowMapWVP._m02_m12_m22_m32));
    // dp4 r0.x, v0, c4
    temp0.x = dot(i.position, (matShadowMapWVP._m00_m10_m20_m30));
    // dp4 r0.y, v0, c7
    temp0.y = dot(i.position, (matShadowMapWVP._m03_m13_m23_m33));
    // rcp r0.z, r0.y
    temp0.z = 1.0f / temp0.y;
    // mul r1.x, r0.z, r0.x
    temp1.x = temp0.z * temp0.x;
    // dp4 r0.x, v0, c5
    temp0.x = dot(i.position, (matShadowMapWVP._m01_m11_m21_m31));
    // mad r1.z, r0.x, -r0.z, c27.x
    temp1.z = temp0.x * -temp0.z + float1(1);
    // mad r0.xz, r1, c27.y, c27.z
    temp0.xz = temp1.xz * float2(2, 2) + float2(-1, -1);
    // mul o5.xy, r0.y, r0.xzzw
    o.texcoord2.xy = temp0.yy * temp0.xz;
    // mov o5.w, r0.y
    o.texcoord2.w = temp0.y;
    // add r0.x, v2.x, v2.x
    temp0.x = i.color.x + i.color.x;
    // add r1, c17, -v0
    temp1 = LocalEyePos + -i.position;
    // dp4 r0.y, r1, r1
    temp0.y = dot(temp1, temp1);
    // rsq r0.y, r0.y
    temp0.y = 1 / sqrt(temp0.y);
    // mul r0.yzw, r0.y, r1.xxyz
    temp0.yzw = temp0.yyy * temp1.xyz;
    // dp3 r1.x, r0.yzww, v1
    temp1.x = dot(temp0.yzw, i.normal.xyz);
    // max r1.x, r1.x, c27.w
    temp1.x = max(temp1.x, float1(0.01));
    // log r1.y, r1.x
    temp1.y = log2(temp1.x);
    // mul r1.z, r1.y, c26.x
    temp1.z = temp1.y * EnvmapPower.x;
    // mul r1.y, r1.y, c25.x
    temp1.y = temp1.y * SpecularPower.x;
    // exp r1.y, r1.y
    temp1.y = exp2(temp1.y);
    // mov r2.xyz, c22
    temp2.xyz = SpecularRange.xyz;
    // mad r2.xyz, r1.y, r2, c21
    temp2.xyz = temp1.yyy * temp2.xyz + SpecularMin.xyz;
    // exp r1.y, r1.z
    temp1.y = exp2(temp1.z);
    // mov r3.x, c24.x
    temp3.x = EnvmapRange.x;
    // mad r1.y, r1.y, r3.x, c23.x
    temp1.y = temp1.y * temp3.x + EnvmapMin.x;
    // mul o2.w, r0.x, r1.y
    o.color1.w = temp0.x * temp1.y;
    // add r0.x, r1.x, r1.x
    temp0.x = temp1.x + temp1.x;
    // mov r3, c20
    temp3 = DiffuseRange;
    // mad r1, r1.x, r3, c19
    temp1 = temp1.x * temp3 + DiffuseMin;
    // mad r3.xyz, r0.x, v1, -r0.yzww
    temp3.xyz = temp0.xxx * i.normal.xyz + -temp0.yzw;
    // dp3 o3.x, r3, c8
    o.texcoord.x = dot(temp3.xyz, (WorldView._m00_m10_m20_m30).xyz);
    // dp3 o3.y, r3, c9
    o.texcoord.y = dot(temp3.xyz, (WorldView._m01_m11_m21_m31).xyz);
    // dp3 o3.z, r3, c10
    o.texcoord.z = dot(temp3.xyz, (WorldView._m02_m12_m22_m32).xyz);
    // dp3 r0.x, v1, c14
    temp0.x = dot(i.normal.xyz, (LocalDirectionMatrix._m00_m10_m20_m30).xyz);
    // add_sat r2.w, r0.x, c28.x
    temp2.w = saturate(temp0.x + float1(0.3));
    // mul r3.xyz, r0.x, v1
    temp3.xyz = temp0.xxx * i.normal.xyz;
    // mov r4.y, c27.y
    temp4.y = float1(2);
    // mad r3.xyz, r3, r4.y, -c14
    temp3.xyz = temp3.xyz * temp4.yyy + -(LocalDirectionMatrix._m00_m10_m20_m30).xyz;
    // dp3_sat r0.x, r3, r0.yzww
    temp0.x = saturate(dot(temp3.xyz, temp0.yzw));
    // mov o6.xyz, r0.yzww
    o.texcoord3.xyz = temp0.yzw;
    // log r0.x, r0.x
    temp0.x = log2(temp0.x);
    // mul r0.x, r0.x, c25.x
    temp0.x = temp0.x * SpecularPower.x;
    // dp3 r0.y, v1, c15
    temp0.y = dot(i.normal.xyz, (LocalDirectionMatrix._m01_m11_m21_m31).xyz);
    // add_sat r0.y, r0.y, c28.x
    temp0.y = saturate(temp0.y + float1(0.3));
    // mul r3.x, r0.y, c11.y
    temp3.x = temp0.y * (LocalColourMatrix._m00_m10_m20_m30).y;
    // mul r3.y, r0.y, c12.y
    temp3.y = temp0.y * (LocalColourMatrix._m01_m11_m21_m31).y;
    // mul r3.z, r0.y, c13.y
    temp3.z = temp0.y * (LocalColourMatrix._m02_m12_m22_m32).y;
    // mov r4.x, c11.x
    temp4.x = (LocalColourMatrix._m00_m10_m20_m30).x;
    // mov r4.y, c12.x
    temp4.y = (LocalColourMatrix._m01_m11_m21_m31).x;
    // mov r4.z, c13.x
    temp4.z = (LocalColourMatrix._m02_m12_m22_m32).x;
    // mad r0.yzw, r2.w, r4.xxyz, r3.xxyz
    temp0.yzw = temp2.www * temp4.xyz + temp3.xyz;
    // mul r0.yzw, r0, v2.x
    temp0.yzw = temp0.yzw * i.color.xxx;
    // mul o1.xyz, r0.yzww, r1
    o.color.xyz = temp0.yzw * temp1.xyz;
    // mul r0.y, r0.x, c28.y
    temp0.y = temp0.x * float1(100);
    // exp r0.x, r0.x
    temp0.x = exp2(temp0.x);
    // exp r0.y, r0.y
    temp0.y = exp2(temp0.y);
    // mul r0.y, r0.y, c18.x
    temp0.y = temp0.y * SpecularHotSpot.x;
    // mad r0.x, r0.x, c28.z, r0.y
    temp0.x = temp0.x * float1(0.7) + temp0.y;
    // mul r0.xyz, r4, r0.x
    temp0.xyz = temp4.xyz * temp0.xxx;
    // mul r0.xyz, r0, v2.x
    temp0.xyz = temp0.xyz * i.color.xxx;
    // mul r0.xyz, r1.w, r0
    temp0.xyz = temp1.www * temp0.xyz;
    // mov o1.w, r1.w
    o.color.w = temp1.w;
    // mul o2.xyz, r0, r2
    o.color1.xyz = temp0.xyz * temp2.xyz;
    // mov o3.w, c27.x
    o.texcoord.w = float1(1);
    // mov o4, v3
    o.texcoord1 = i.texcoord;
    // mov o6.w, c27.x
    o.texcoord3.w = float1(1);
    // mov o7.xyz, v1
    o.texcoord4.xyz = i.normal.xyz;
    // mov o7.w, v0.x
    o.texcoord4.w = i.position.x;
    // mov o8.xyz, c14
    o.texcoord5.xyz = (LocalDirectionMatrix._m00_m10_m20_m30).xyz;
    // mov o8.w, v0.y
    o.texcoord5.w = i.position.y;
    // mov o9.xyz, c15
    o.texcoord6.xyz = (LocalDirectionMatrix._m01_m11_m21_m31).xyz;
    // mov o9.w, v0.z
    o.texcoord6.w = i.position.z;
    // 

    return o;
}

// lowlod_Expression20 Expression_2_0 Has PRES False
float lowlod_Expression20()
{
    float1 expr0;
    // mov c0.x, c4.x
    expr0.x = Blend_State[4].x;
    return expr0;
}

// lowlod_Expression21 Expression_2_0 Has PRES False
float lowlod_Expression21()
{
    float1 expr0;
    // mov c0.x, c3.x
    expr0.x = Blend_State[3].x;
    return expr0;
}

// lowlod_Expression22 Expression_2_0 Has PRES False
float lowlod_Expression22()
{
    float1 expr0;
    // mov c0.x, c2.x
    expr0.x = Blend_State[2].x;
    return expr0;
}

// lowlod_Expression23 Expression_2_0 Has PRES False
float lowlod_Expression23()
{
    float1 expr0;
    // mov c0.x, c1.x
    expr0.x = Blend_State[1].x;
    return expr0;
}

// lowlod_Expression24 Expression_2_0 Has PRES False
float lowlod_Expression24()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = Blend_State[0].x;
    return expr0;
}

// carnormalmap_PixelShader25 Pixel_3_0 Has PRES False
struct carnormalmap_PixelShader25_Input
{
    float4 color : COLOR;
    float4 color1 : COLOR1;
    float4 texcoord : TEXCOORD;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
    float4 texcoord3 : TEXCOORD3;
    float4 texcoord4 : TEXCOORD4;
    float4 texcoord5 : TEXCOORD5;
    float4 texcoord6 : TEXCOORD6;
};

float4 carnormalmap_PixelShader25(carnormalmap_PixelShader25_Input i) : COLOR
{
    float4 out_color;
    float4 temp0, temp1, temp2, temp3, temp4, temp5, temp6;
    float3 temp7;
    // def c14, 0.001, 20, -3, -0.5
    // def c15, 1, -0.5, 0, 0.5
    // def c16, 0.11111111, 0.01, 0.02, 0.3
    // def c17, 0.08, 2, 100, 0.7
    // dcl_color_pp v0
    // dcl_color1_pp v1
    // dcl_texcoord_pp v2.w
    // dcl_texcoord1_pp v3
    // dcl_texcoord2 v4
    // dcl_texcoord3_pp v5
    // dcl_texcoord4 v6
    // dcl_texcoord5 v7
    // dcl_texcoord6 v8
    // dcl_2d s0
    // dcl_2d s1
    // dcl_volume s2
    // dcl_cube s3
    // dcl_2d s4
    // mov r0.zw, v4
    temp0.zw = i.texcoord2.zw;
    // mul r1.xy, -c14.w, v4
    temp1.xy = float2(0.5, 0.5) * i.texcoord2.xy;
    // rcp r1.z, v4.w
    temp1.z = 1.0f / i.texcoord2.w;
    // mad r2.xy, r1, r1.z, -c14.w
    temp2.xy = temp1.xy * temp1.zz + float2(0.5, 0.5);
    // add r2.z, -r2.y, c15.x
    temp2.z = -temp2.y + float1(1);
    // mul r1.xy, r2.xzzw, v4.w
    temp1.xy = temp2.xz * i.texcoord2.ww;
    // mul r1.xy, r1.z, r1
    temp1.xy = temp1.zz * temp1.xy;
    // mov r2.x, c11.x
    temp2.x = g_fShadowMapScaleX.x;
    // mov r2.y, c12.x
    temp2.y = g_fShadowMapScaleY.x;
    // mad r3, r2.xyxy, c15.yyyz, r1.xyxy
    temp3 = temp2.xyxy * float4(-0.5, -0.5, -0.5, 0) + temp1.xyxy;
    // mul r0.xy, r3, v4.w
    temp0.xy = temp3.xy * i.texcoord2.ww;
    // mul r3.xy, r3.zwzw, v4.w
    temp3.xy = temp3.zw * i.texcoord2.ww;
    // texld r0, r0, s4
    temp0 = tex2D(SHADOWMAP_SAMPLER, temp0.xy);
    // mov r3.zw, v4
    temp3.zw = i.texcoord2.zw;
    // texld r3, r3, s4
    temp3 = tex2D(SHADOWMAP_SAMPLER, temp3.xy);
    // mov r0.y, r3.x
    temp0.y = temp3.x;
    // mov r3.zw, v4
    temp3.zw = i.texcoord2.zw;
    // mad r4, r2.xyxy, c15.ywzy, r1.xyxy
    temp4 = temp2.xyxy * float4(-0.5, 0.5, 0, -0.5) + temp1.xyxy;
    // mul r3.xy, r4, v4.w
    temp3.xy = temp4.xy * i.texcoord2.ww;
    // mul r4.xy, r4.zwzw, v4.w
    temp4.xy = temp4.zw * i.texcoord2.ww;
    // texld r3, r3, s4
    temp3 = tex2D(SHADOWMAP_SAMPLER, temp3.xy);
    // mov r0.z, r3.x
    temp0.z = temp3.x;
    // mov r4.zw, v4
    temp4.zw = i.texcoord2.zw;
    // texld r3, r4, s4
    temp3 = tex2D(SHADOWMAP_SAMPLER, temp4.xy);
    // mov r0.w, r3.x
    temp0.w = temp3.x;
    // mad r1.w, v4.z, r1.z, c10.x
    temp1.w = i.texcoord2.z * temp1.z + g_fShadowMapBias.x;
    // mad r1.z, v4.y, r1.z, c14.w
    temp1.z = i.texcoord2.y * temp1.z + float1(-0.5);
    // add_sat r1.z, r1.z, r1.z
    temp1.z = saturate(temp1.z + temp1.z);
    // add r0, r0, -r1.w
    temp0 = temp0 + -temp1.w;
    // cmp r0, r0, c15.x, c15.z
    temp0 = (temp0 >= 0) ? float4(1, 1, 1, 1) : float4(0, 0, 0, 0);
    // dp4 r0.x, r0, r0
    temp0.x = dot(temp0, temp0);
    // mul r3.xy, r1, v4.w
    temp3.xy = temp1.xy * i.texcoord2.ww;
    // mov r3.zw, v4
    temp3.zw = i.texcoord2.zw;
    // texld r3, r3, s4
    temp3 = tex2D(SHADOWMAP_SAMPLER, temp3.xy);
    // mad r4, r2.xyxy, c15.zwwy, r1.xyxy
    temp4 = temp2.xyxy * float4(0, 0.5, 0.5, -0.5) + temp1.xyxy;
    // mad r2, r2.xyxy, c15.wzww, r1.xyxy
    temp2 = temp2.xyxy * float4(0.5, 0, 0.5, 0.5) + temp1.xyxy;
    // mul r5.xy, r4, v4.w
    temp5.xy = temp4.xy * i.texcoord2.ww;
    // mul r4.xy, r4.zwzw, v4.w
    temp4.xy = temp4.zw * i.texcoord2.ww;
    // mov r5.zw, v4
    temp5.zw = i.texcoord2.zw;
    // texld r5, r5, s4
    temp5 = tex2D(SHADOWMAP_SAMPLER, temp5.xy);
    // mov r3.y, r5.x
    temp3.y = temp5.x;
    // mov r4.zw, v4
    temp4.zw = i.texcoord2.zw;
    // texld r4, r4, s4
    temp4 = tex2D(SHADOWMAP_SAMPLER, temp4.xy);
    // mov r3.z, r4.x
    temp3.z = temp4.x;
    // mul r4.xy, r2, v4.w
    temp4.xy = temp2.xy * i.texcoord2.ww;
    // mul r2.xy, r2.zwzw, v4.w
    temp2.xy = temp2.zw * i.texcoord2.ww;
    // mov r4.zw, v4
    temp4.zw = i.texcoord2.zw;
    // texld r4, r4, s4
    temp4 = tex2D(SHADOWMAP_SAMPLER, temp4.xy);
    // mov r3.w, r4.x
    temp3.w = temp4.x;
    // add r3, -r1.w, r3
    temp3 = -temp1.w + temp3;
    // cmp r3, r3, c15.x, c15.z
    temp3 = (temp3 >= 0) ? float4(1, 1, 1, 1) : float4(0, 0, 0, 0);
    // dp4 r0.y, r3, r3
    temp0.y = dot(temp3, temp3);
    // add r0.x, r0.y, r0.x
    temp0.x = temp0.y + temp0.x;
    // mov r2.zw, v4
    temp2.zw = i.texcoord2.zw;
    // texld r2, r2, s4
    temp2 = tex2D(SHADOWMAP_SAMPLER, temp2.xy);
    // add r0.y, -r1.w, r2.x
    temp0.y = -temp1.w + temp2.x;
    // cmp r0.y, r0.y, c15.x, c15.z
    temp0.y = (temp0.y >= 0) ? float1(1) : float1(0);
    // add r0.x, r0.y, r0.x
    temp0.x = temp0.y + temp0.x;
    // mad_sat r0.x, r0.x, c16.x, r1.z
    temp0.x = saturate(temp0.x * float1(0.11111111) + temp1.z);
    // abs r0.y, c13.x
    temp0.y = abs(g_iShadowMapEnabled.x);
    // cmp r0.x, -r0.y, c15.x, r0.x
    temp0.x = (-temp0.y >= 0) ? float1(1) : temp0.x;
    // abs r0.y, c9.x
    temp0.y = abs(g_bDoCarShadowMap.x);
    // cmp r0.x, -r0.y, c15.x, r0.x
    temp0.x = (-temp0.y >= 0) ? float1(1) : temp0.x;
    // mov r1.x, c15.x
    temp1.x = float1(1);
    // lrp r2.xyz, r0.x, r1.x, c8
    temp2.xyz = lerp(ShadowColour.xyz, temp1.xxx, temp0.xxx);
    // dsy r1.y, v6.w
    temp1.y = ddy(i.texcoord4.w);
    // dsy r1.z, v7.w
    temp1.z = ddy(i.texcoord5.w);
    // dsy r1.x, v8.w
    temp1.x = ddy(i.texcoord6.w);
    // mul r0.yzw, r1.xxyz, v6.xyzx
    temp0.yzw = temp1.xyz * i.texcoord4.yzx;
    // mad r0.yzw, r1.xzxy, v6.xzxy, -r0
    temp0.yzw = temp1.zxy * i.texcoord4.zxy + -temp0.yzw;
    // dsx_pp r1.z, v6.w
    temp1.z = /* not implemented _pp modifier */ ddx(i.texcoord4.w);
    // dsx_pp r1.x, v7.w
    temp1.x = /* not implemented _pp modifier */ ddx(i.texcoord5.w);
    // dsx_pp r1.y, v8.w
    temp1.y = /* not implemented _pp modifier */ ddx(i.texcoord6.w);
    // mul r3.xyz, r1, v6.zxyw
    temp3.xyz = temp1.xyz * i.texcoord4.zxy;
    // mad r1.xyz, v6.yzxw, r1.yzxw, -r3
    temp1.xyz = i.texcoord4.yzx * temp1.yzx + -temp3.xyz;
    // dsy_pp r3.xy, v3
    temp3.xy = /* not implemented _pp modifier */ ddy(i.texcoord1.xy);
    // max r4.xy, r3_abs, c14.x
    temp4.xy = max(abs(temp3).xy, float2(0.001, 0.001));
    // mul r3.xyz, r1, r4.y
    temp3.xyz = temp1.xyz * temp4.yyy;
    // mul r1.xyz, r1, r4.x
    temp1.xyz = temp1.xyz * temp4.xxx;
    // dsx_pp r4.xy, v3
    temp4.xy = /* not implemented _pp modifier */ ddx(i.texcoord1.xy);
    // max r5.xy, r4_abs, c14.x
    temp5.xy = max(abs(temp4).xy, float2(0.001, 0.001));
    // mad r3.xyz, r0.yzww, r5.y, r3
    temp3.xyz = temp0.yzw * temp5.yyy + temp3.xyz;
    // mad r0.yzw, r0, r5.x, r1.xxyz
    temp0.yzw = temp0.yzw * temp5.xxx + temp1.xyz;
    // dp3 r1.x, r3, r3
    temp1.x = dot(temp3.xyz, temp3.xyz);
    // dp3 r1.y, r0.yzww, r0.yzww
    temp1.y = dot(temp0.yzw, temp0.yzw);
    // max r2.w, r1.y, r1.x
    temp2.w = max(temp1.y, temp1.x);
    // rsq r1.x, r2.w
    temp1.x = 1 / sqrt(temp2.w);
    // mul r0.yzw, r0, r1.x
    temp0.yzw = temp0.yzw * temp1.xxx;
    // nrm r1.xyz, r0.yzww
    temp1.xyz = normalize(temp0.yzww.xyz).xyz;
    // dp3_pp r3.x, r1, v5
    temp3.x = /* not implemented _pp modifier */ dot(temp1.xyz, i.texcoord3.xyz);
    // mul r0.yzw, r1.xyzx, v6.xzxy
    temp0.yzw = temp1.yzx * i.texcoord4.zxy;
    // mad_pp r0.yzw, v6.xyzx, r1.xzxy, -r0
    temp0.yzw = /* not implemented _pp modifier */ i.texcoord4.yzx * temp1.zxy + -temp0.yzw;
    // dp3_pp r3.y, r0.yzww, v5
    temp3.y = /* not implemented _pp modifier */ dot(temp0.yzw, i.texcoord3.xyz);
    // mov r4.xyz, v6
    temp4.xyz = i.texcoord4.xyz;
    // dp3_pp r3.z, r4, v5
    temp3.z = /* not implemented _pp modifier */ dot(temp4.xyz, i.texcoord3.xyz);
    // nrm_pp r5.xyz, r3
    temp5.xyz = /* not implemented _pp modifier */ normalize(temp3.xyz).xyz;
    // dp3_pp r3.x, r1, v7
    temp3.x = /* not implemented _pp modifier */ dot(temp1.xyz, i.texcoord5.xyz);
    // dp3_pp r1.x, r1, v8
    temp1.x = /* not implemented _pp modifier */ dot(temp1.xyz, i.texcoord6.xyz);
    // dp3_pp r3.y, r0.yzww, v7
    temp3.y = /* not implemented _pp modifier */ dot(temp0.yzw, i.texcoord5.xyz);
    // dp3_pp r1.y, r0.yzww, v8
    temp1.y = /* not implemented _pp modifier */ dot(temp0.yzw, i.texcoord6.xyz);
    // mov r6.x, v6.w
    temp6.x = i.texcoord4.w;
    // mov r6.y, v7.w
    temp6.y = i.texcoord5.w;
    // mov r6.z, v8.w
    temp6.z = i.texcoord6.w;
    // mul r6.xyz, r6, c14.y
    temp6.xyz = temp6.xyz * float3(20, 20, 20);
    // mov r6.w, c14.z
    temp6.w = float1(-3);
    // texld_pp r6, r6, s2
    temp6 = /* not implemented _pp modifier */ tex3D(VOLUMEMAP_SAMPLER, temp6.xyz);
    // add_pp r0.yzw, r6.xxyz, c14.w
    temp0.yzw = /* not implemented _pp modifier */ temp6.xyz + float3(-0.5, -0.5, -0.5);
    // mul_pp r0.yzw, r0, c3.x
    temp0.yzw = /* not implemented _pp modifier */ temp0.yzw * MetallicScale.xxx;
    // mul r0.yzw, r0, v5.w
    temp0.yzw = temp0.yzw * i.texcoord3.www;
    // texld_pp r6, v3, s1
    temp6 = /* not implemented _pp modifier */ tex2D(NORMALMAP_SAMPLER, i.texcoord1.xy);
    // add_pp r6, r6, c14.w
    temp6 = /* not implemented _pp modifier */ temp6 + float4(-0.5, -0.5, -0.5, -0.5);
    // add_pp r6, r6, r6
    temp6 = /* not implemented _pp modifier */ temp6 + temp6;
    // dp4_pp r1.w, r6, r6
    temp1.w = /* not implemented _pp modifier */ dot(temp6, temp6);
    // rsq_pp r1.w, r1.w
    temp1.w = /* not implemented _pp modifier */ 1 / sqrt(temp1.w);
    // mul_pp r6.xyz, r1.w, r6
    temp6.xyz = /* not implemented _pp modifier */ temp1.www * temp6.xyz;
    // mad r0.yzw, r0, c17.x, r6.xxyz
    temp0.yzw = temp0.yzw * float3(0.08, 0.08, 0.08) + temp6.xyz;
    // nrm_pp r7.xyz, r0.yzww
    temp7.xyz = /* not implemented _pp modifier */ normalize(temp0.yzww.xyz).xyz;
    // dp3_pp r3.z, r4, v7
    temp3.z = /* not implemented _pp modifier */ dot(temp4.xyz, i.texcoord5.xyz);
    // dp3_pp r0.y, r7, r3
    temp0.y = /* not implemented _pp modifier */ dot(temp7.xyz, temp3.xyz);
    // mul_pp r0.yzw, r7.xxyz, r0.y
    temp0.yzw = /* not implemented _pp modifier */ temp7.xyz * temp0.yyy;
    // mad_pp r0.yzw, r0, c17.y, -r3.xxyz
    temp0.yzw = /* not implemented _pp modifier */ temp0.yzw * float3(2, 2, 2) + -temp3.xyz;
    // dp3_pp r1.w, r6, r3
    temp1.w = /* not implemented _pp modifier */ dot(temp6.xyz, temp3.xyz);
    // add_sat_pp r1.w, r1.w, c16.w
    ;// error
    // dp3_sat_pp r0.y, r0.yzww, r5
    ;// error
    // log r0.y, r0.y
    temp0.y = log2(temp0.y);
    // mul r0.y, r0.y, c7.x
    temp0.y = temp0.y * SpecularPower.x;
    // mul r0.z, r0.y, c17.z
    temp0.z = temp0.y * float1(100);
    // exp r0.y, r0.y
    temp0.y = exp2(temp0.y);
    // exp r0.z, r0.z
    temp0.z = exp2(temp0.z);
    // mul r0.z, r0.x, r0.z
    temp0.z = temp0.x * temp0.z;
    // mul r0.z, r0.z, c4.x
    temp0.z = temp0.z * SpecularHotSpot.x;
    // mad_pp r0.y, r0.y, c17.w, r0.z
    temp0.y = /* not implemented _pp modifier */ temp0.y * float1(0.7) + temp0.z;
    // mov r3.x, c0.x
    temp3.x = (LocalColourMatrix._m00_m10_m20_m30).x;
    // mov r3.y, c1.x
    temp3.y = (LocalColourMatrix._m01_m11_m21_m31).x;
    // mov r3.z, c2.x
    temp3.z = (LocalColourMatrix._m02_m12_m22_m32).x;
    // mul_pp r0.yzw, r0.y, r3.xxyz
    temp0.yzw = /* not implemented _pp modifier */ temp0.yyy * temp3.xyz;
    // mul_pp r0.yzw, r0, v0.w
    temp0.yzw = /* not implemented _pp modifier */ temp0.yzw * i.color.www;
    // mul_pp r0.yzw, r0, v1.xxyz
    temp0.yzw = /* not implemented _pp modifier */ temp0.yzw * i.color1.xyz;
    // mul_pp r0.xyz, r0.x, r0.yzww
    temp0.xyz = /* not implemented _pp modifier */ temp0.xxx * temp0.yzw;
    // mul_pp r0.xyz, r0, v2.w
    temp0.xyz = /* not implemented _pp modifier */ temp0.xyz * i.texcoord.www;
    // dp3_pp r1.z, r4, v8
    temp1.z = /* not implemented _pp modifier */ dot(temp4.xyz, i.texcoord6.xyz);
    // dp3_pp r0.w, r6, r1
    temp0.w = /* not implemented _pp modifier */ dot(temp6.xyz, temp1.xyz);
    // add_sat_pp r0.w, r0.w, c16.w
    ;// error
    // mul r1.x, r0.w, c0.y
    temp1.x = temp0.w * (LocalColourMatrix._m00_m10_m20_m30).y;
    // mul r1.y, r0.w, c1.y
    temp1.y = temp0.w * (LocalColourMatrix._m01_m11_m21_m31).y;
    // mul r1.z, r0.w, c2.y
    temp1.z = temp0.w * (LocalColourMatrix._m02_m12_m22_m32).y;
    // mad_pp r1.xyz, r1.w, r3, r1
    temp1.xyz = /* not implemented _pp modifier */ temp1.www * temp3.xyz + temp1.xyz;
    // mul_pp r1.xyz, r1, v0
    temp1.xyz = /* not implemented _pp modifier */ temp1.xyz * i.color.xyz;
    // texld_pp r3, v3, s0
    temp3 = /* not implemented _pp modifier */ tex2D(DIFFUSEMAP_SAMPLER, i.texcoord1.xy);
    // mad_pp r0.xyz, r1, r3, r0
    temp0.xyz = /* not implemented _pp modifier */ temp1.xyz * temp3.xyz + temp0.xyz;
    // mul_pp oC0.w, r3.w, v0.w
    out_color.w = /* not implemented _pp modifier */ temp3.w * i.color.w;
    // dp3_pp r0.w, r5, r6
    temp0.w = /* not implemented _pp modifier */ dot(temp5.xyz, temp6.xyz);
    // add r1.x, -r0.w, c16.y
    temp1.x = -temp0.w + float1(0.01);
    // add r0.w, r0.w, r0.w
    temp0.w = temp0.w + temp0.w;
    // cmp r0.w, r1.x, c16.z, r0.w
    temp0.w = (temp1.x >= 0) ? float1(0.02) : temp0.w;
    // mad_pp r1.xyz, r0.w, r6, -r5
    temp1.xyz = /* not implemented _pp modifier */ temp0.www * temp6.xyz + -temp5.xyz;
    // texld_pp r1, r1, s3
    temp1 = /* not implemented _pp modifier */ texCUBE(ENVIROMAP_SAMPLER, temp1.xyz);
    // add_pp r1.xyz, r1, r1
    temp1.xyz = /* not implemented _pp modifier */ temp1.xyz + temp1.xyz;
    // mov r3.xyz, c6
    temp3.xyz = EnvmapRange.xyz;
    // mad r3.xyz, v1.w, r3, c5
    temp3.xyz = i.color1.www * temp3.xyz + EnvmapMin.xyz;
    // mul_pp r1.xyz, r1, r3
    temp1.xyz = /* not implemented _pp modifier */ temp1.xyz * temp3.xyz;
    // mul_pp r1.xyz, r1, v0.w
    temp1.xyz = /* not implemented _pp modifier */ temp1.xyz * i.color.www;
    // mul_pp r1.xyz, r1, v2.w
    temp1.xyz = /* not implemented _pp modifier */ temp1.xyz * i.texcoord.www;
    // mad_pp oC0.xyz, r0, r2, r1
    out_color.xyz = /* not implemented _pp modifier */ temp0.xyz * temp2.xyz + temp1.xyz;
    // 

    return out_color;
}

// carnormalmap_VertexShader26 Vertex_3_0 Has PRES False
struct carnormalmap_VertexShader26_Input
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 color : COLOR;
    float4 texcoord : TEXCOORD;
};

struct carnormalmap_VertexShader26_Output
{
    float4 position : POSITION;
    float4 color : COLOR;
    float4 color1 : COLOR1;
    float4 texcoord : TEXCOORD;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
    float4 texcoord3 : TEXCOORD3;
    float4 texcoord4 : TEXCOORD4;
    float4 texcoord5 : TEXCOORD5;
    float4 texcoord6 : TEXCOORD6;
};

carnormalmap_VertexShader26_Output carnormalmap_VertexShader26(carnormalmap_VertexShader26_Input i)
{
    carnormalmap_VertexShader26_Output o;
    float4 temp0, temp1;
    // def c18, -0.25, 1.2, 0, 0
    // def c19, 0.01, 0, 1, -2
    // dcl_position v0
    // dcl_normal v1
    // dcl_color v2
    // dcl_texcoord v3
    // dcl_position o0
    // dcl_color o1
    // dcl_color1 o2
    // dcl_texcoord o3
    // dcl_texcoord1 o4
    // dcl_texcoord2 o5
    // dcl_texcoord3 o6
    // dcl_texcoord4 o7
    // dcl_texcoord5 o8
    // dcl_texcoord6 o9
    // dp4 o0.z, v0, c2
    o.position.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 r0.x, v0, c0
    temp0.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 r0.y, v0, c1
    temp0.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 r0.z, v0, c3
    temp0.z = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mad o0.xy, c10, r0.z, r0
    o.position.xy = ScreenOffset.xy * temp0.zz + temp0.xy;
    // mov o0.w, r0.z
    o.position.w = temp0.z;
    // mov r0.xy, c19
    temp0.xy = float2(0.01, 0);
    // mad r0, c8.xyzx, r0.xxxy, v0
    temp0 = (LocalDirectionMatrix._m00_m10_m20_m30).xyzx * temp0.xxxy + i.position;
    // dp4 o5.z, r0, c6
    o.texcoord2.z = dot(temp0, (matShadowMapWVP._m02_m12_m22_m32));
    // dp4 r1.x, r0, c4
    temp1.x = dot(temp0, (matShadowMapWVP._m00_m10_m20_m30));
    // dp4 r1.y, r0, c7
    temp1.y = dot(temp0, (matShadowMapWVP._m03_m13_m23_m33));
    // dp4 r0.x, r0, c5
    temp0.x = dot(temp0, (matShadowMapWVP._m01_m11_m21_m31));
    // rcp r0.y, r1.y
    temp0.y = 1.0f / temp1.y;
    // mul r1.x, r0.y, r1.x
    temp1.x = temp0.y * temp1.x;
    // mad r1.z, r0.x, -r0.y, c19.z
    temp1.z = temp0.x * -temp0.y + float1(1);
    // mad r0.xy, r1.xzzw, -c19.w, -c19.z
    temp0.xy = temp1.xz * float2(2, 2) + float2(-1, -1);
    // mul o5.xy, r1.y, r0
    o.texcoord2.xy = temp1.yy * temp0.xy;
    // mov o5.w, r1.y
    o.texcoord2.w = temp1.y;
    // add r0.xyz, c11, -v0
    temp0.xyz = LocalEyePos.xyz + -i.position.xyz;
    // dp3 r0.w, r0, r0
    temp0.w = dot(temp0.xyz, temp0.xyz);
    // rsq r0.w, r0.w
    temp0.w = 1 / sqrt(temp0.w);
    // rcp r1.x, r0.w
    temp1.x = 1.0f / temp0.w;
    // mul r1.yzw, r0.w, r0.xxyz
    temp1.yzw = temp0.www * temp0.xyz;
    // mov o6.xyz, r0
    o.texcoord3.xyz = temp0.xyz;
    // dp3 r0.x, r1.yzww, v1
    temp0.x = dot(temp1.yzw, i.normal.xyz);
    // max r0.x, r0.x, c19.x
    temp0.x = max(temp0.x, float1(0.01));
    // mad_sat o6.w, r1.x, c18.x, c18.y
    o.texcoord3.w = saturate(temp1.x * float1(-0.25) + float1(1.2));
    // log r0.y, r0.x
    temp0.y = log2(temp0.x);
    // mov r1, c13
    temp1 = DiffuseRange;
    // mad o1, r0.x, r1, c12
    o.color = temp0.x * temp1 + DiffuseMin;
    // mul r0.x, r0.y, c17.x
    temp0.x = temp0.y * EnvmapPower.x;
    // mul r0.y, r0.y, c16.x
    temp0.y = temp0.y * SpecularPower.x;
    // exp r0.y, r0.y
    temp0.y = exp2(temp0.y);
    // mov r1.xyz, c15
    temp1.xyz = SpecularRange.xyz;
    // mad o2.xyz, r0.y, r1, c14
    o.color1.xyz = temp0.yyy * temp1.xyz + SpecularMin.xyz;
    // exp o2.w, r0.x
    o.color1.w = exp2(temp0.x);
    // mul o3, c19.yyyz, v2.x
    o.texcoord = float4(0, 0, 0, 1) * i.color.x;
    // mov o4, v3
    o.texcoord1 = i.texcoord;
    // mov o7.xyz, v1
    o.texcoord4.xyz = i.normal.xyz;
    // mov o7.w, v0.x
    o.texcoord4.w = i.position.x;
    // mov o8.xyz, c8
    o.texcoord5.xyz = (LocalDirectionMatrix._m00_m10_m20_m30).xyz;
    // mov o8.w, v0.y
    o.texcoord5.w = i.position.y;
    // mov o9.xyz, c9
    o.texcoord6.xyz = (LocalDirectionMatrix._m01_m11_m21_m31).xyz;
    // mov o9.w, v0.z
    o.texcoord6.w = i.position.z;
    // 

    return o;
}

// carnormalmap_Expression27 Expression_2_0 Has PRES False
float carnormalmap_Expression27()
{
    float1 expr0;
    // mov c0.x, c4.x
    expr0.x = Blend_State[4].x;
    return expr0;
}

// carnormalmap_Expression28 Expression_2_0 Has PRES False
float carnormalmap_Expression28()
{
    float1 expr0;
    // mov c0.x, c3.x
    expr0.x = Blend_State[3].x;
    return expr0;
}

// carnormalmap_Expression29 Expression_2_0 Has PRES False
float carnormalmap_Expression29()
{
    float1 expr0;
    // mov c0.x, c2.x
    expr0.x = Blend_State[2].x;
    return expr0;
}

// carnormalmap_Expression30 Expression_2_0 Has PRES False
float carnormalmap_Expression30()
{
    float1 expr0;
    // mov c0.x, c1.x
    expr0.x = Blend_State[1].x;
    return expr0;
}

// carnormalmap_Expression31 Expression_2_0 Has PRES False
float carnormalmap_Expression31()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = Blend_State[0].x;
    return expr0;
}

// Expression32 Expression_2_0 Has PRES False
float Expression32()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMagTextureFilter.x;
    return expr0;
}

// Expression33 Expression_2_0 Has PRES False
float Expression33()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMinTextureFilter.x;
    return expr0;
}

// Expression34 Expression_2_0 Has PRES False
float Expression34()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMipTextureFilter.x;
    return expr0;
}

// Expression35 Expression_2_0 Has PRES False
float Expression35()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMagTextureFilter.x;
    return expr0;
}

// Expression36 Expression_2_0 Has PRES False
float Expression36()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMinTextureFilter.x;
    return expr0;
}

// Expression37 Expression_2_0 Has PRES False
float Expression37()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMipTextureFilter.x;
    return expr0;
}

// Expression38 Expression_2_0 Has PRES False
float Expression38()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseAddressV.x;
    return expr0;
}

// Expression39 Expression_2_0 Has PRES False
float Expression39()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseAddressU.x;
    return expr0;
}

// Expression40 Expression_2_0 Has PRES False
float Expression40()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMagTextureFilter.x;
    return expr0;
}

// Expression41 Expression_2_0 Has PRES False
float Expression41()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMinTextureFilter.x;
    return expr0;
}

// Expression42 Expression_2_0 Has PRES False
float Expression42()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMipTextureFilter.x;
    return expr0;
}

// Expression43 Expression_2_0 Has PRES False
float Expression43()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseAddressV.x;
    return expr0;
}

// Expression44 Expression_2_0 Has PRES False
float Expression44()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseAddressU.x;
    return expr0;
}

// Expression45 Expression_2_0 Has PRES False
float Expression45()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMagTextureFilter.x;
    return expr0;
}

// Expression46 Expression_2_0 Has PRES False
float Expression46()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMinTextureFilter.x;
    return expr0;
}

// Expression47 Expression_2_0 Has PRES False
float Expression47()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMipTextureFilter.x;
    return expr0;
}

// Expression48 Expression_2_0 Has PRES False
float Expression48()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseAddressV.x;
    return expr0;
}

// Expression49 Expression_2_0 Has PRES False
float Expression49()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseAddressU.x;
    return expr0;
}

technique carnormalmap <int shader = 1;>
{
    pass p0
    {
        AlphaTestEnable = carnormalmap_Expression31(); // 0
        AlphaRef = carnormalmap_Expression30(); // 0
        AlphaBlendEnable = carnormalmap_Expression29(); // 0
        SrcBlend = carnormalmap_Expression28(); // 0
        DestBlend = carnormalmap_Expression27(); // 0
        CullMode = 1;
        VertexShader = compile vs_3_0 carnormalmap_VertexShader26(); // 15
        PixelShader = compile ps_3_0 carnormalmap_PixelShader25(); // 16
    }
}

technique lowlod <int shader = 1;>
{
    pass p0
    {
        AlphaTestEnable = lowlod_Expression24(); // 0
        AlphaRef = lowlod_Expression23(); // 0
        AlphaBlendEnable = lowlod_Expression22(); // 0
        SrcBlend = lowlod_Expression21(); // 0
        DestBlend = lowlod_Expression20(); // 0
        CullMode = 1;
        VertexShader = compile vs_3_0 lowlod_VertexShader19(); // 17
        PixelShader = compile ps_3_0 lowlod_PixelShader18(); // 18
    }
}

technique ZPrePass <int shader = 1;>
{
    pass p0
    {
        AlphaTestEnable = 0;
        CullMode = ZPrePass_Expression17(); // 0
        VertexShader = compile vs_1_1 ZPrePass_VertexShader16(); // 19
        PixelShader = NULL /* Blob is NULL!!! */; // 0
    }
}

technique RenderVertexColour <int shader = 1;>
{
    pass p0
    {
        VertexShader = compile vs_1_1 RenderVertexColour_VertexShader15(); // 20
        PixelShader = compile ps_2_0 RenderVertexColour_PixelShader14(); // 21
    }
}

technique RenderLight <int shader = 1;>
{
    pass p0
    {
        CullMode = 1;
        Clipping = 0;
        VertexShader = compile vs_1_1 RenderLight_VertexShader13(); // 22
        PixelShader = NULL /* Blob is NULL!!! */; // 0
    }
}

technique RenderLightAlphaTex <int shader = 1;>
{
    pass p0
    {
        CullMode = 1;
        Clipping = 0;
        ColorWriteEnable = RenderLightAlphaTex_Expression12(); // 0
        VertexShader = compile vs_3_0 RenderLightAlphaTex_VertexShader11(); // 23
        PixelShader = compile ps_3_0 RenderLightAlphaTex_PixelShader10(); // 24
    }
}

technique RenderWhite <int shader = 1;>
{
    pass p0
    {
        CullMode = 1;
        ColorWriteEnable = RenderWhite_Expression9(); // 0
        VertexShader = compile vs_3_0 RenderWhite_VertexShader8(); // 25
        PixelShader = compile ps_3_0 RenderWhite_PixelShader7(); // 26
    }
}

technique RenderTexelDensity <int shader = 1;>
{
    pass p0
    {
        CullMode = 1;
        ColorWriteEnable = RenderTexelDensity_Expression6(); // 0
        VertexShader = compile vs_3_0 RenderTexelDensity_VertexShader5(); // 27
        PixelShader = compile ps_3_0 RenderTexelDensity_PixelShader4(); // 28
    }
}

technique RenderWhiteAlphaTex <int shader = 1;>
{
    pass p0
    {
        CullMode = 1;
        ColorWriteEnable = RenderWhiteAlphaTex_Expression3(); // 0
        VertexShader = compile vs_3_0 RenderWhiteAlphaTex_VertexShader2(); // 29
        PixelShader = compile ps_3_0 RenderWhiteAlphaTex_PixelShader1(); // 30
    }
}

