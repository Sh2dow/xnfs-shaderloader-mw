/////////////////////////////////////////////////////////////////////////////////////////
shared int Blend_State[5] : BLENDSTATE;
shared int Cull_Mode : CULL_MODE;

shared int BaseMipTextureFilter : BASEMIPTEXTUREFILTER;
shared int BaseMinTextureFilter : BASEMINTEXTUREFILTER;
shared int BaseMagTextureFilter : BASEMAGTEXTUREFILTER;

#define MIPMAPBIAS (BaseMinTextureFilter > 2 ? -0.25f : -2.0f)
// float		MipMapBias			: MIPMAPBIAS;

// The per-color weighting to be used for luminance	calculations in	RGB	order.
// float3	LUMINANCE_VECTOR  =	float3(0.2125f,	0.7154f, 0.0721f);
float3	LUMINANCE_VECTOR  =	float3(0.6125, 0.5154, 0.0721);

float4x4 	WorldViewProj   : WORLDVIEWPROJECTION ;
float4x4	cmWorldViewProj			: WorldViewProj;
float4		cvScreenOffset			: cvScreenOffset;
float4		cvVertexPowerBrightness : cvVertexPowerBrightness;
shared float4 ScreenOffset		: SCREENOFFSET;

#define DECLARE_TEXTURE(textured)	                 shared texture textured : textured;
#define ASSIGN_TEXTURE(textured)	                 Texture = <textured>;
#define DECLARE_MINFILTER(default_filter)			 MINFILTER = default_filter;
#define DECLARE_MAGFILTER(default_filter)			 MAGFILTER = default_filter;
#define DECLARE_MIPFILTER(default_filter)			 MIPFILTER = default_filter;
#define COMPILETIME_BOOL bool
#define cmWorldViewProj WorldViewProj

// -------------------- Blend_State helper (replaces 10+ Expression shaders) --------------------
int GetBlendState(int idx)
{
    return Blend_State[idx];
}

// Convenience mapping (readability)
#define BS_AlphaTestEnable   GetBlendState(0)
#define BS_AlphaRef          GetBlendState(1)
#define BS_AlphaBlendEnable  GetBlendState(2)
#define BS_SrcBlend          GetBlendState(3)
#define BS_DestBlend         GetBlendState(4)
// ------------------------------------------------------------------
// Common pass state macro
// ------------------------------------------------------------------
// #define COMMON_FULLSCREEN_PASS_STATE \
//     AlphaTestEnable   = 0;          \
//     AlphaRef          = 0;          \
//     AlphaBlendEnable  = 0;          \
//     SrcBlend          = 2;          \
//     DestBlend         = 1;          \
//     CullMode          = 1;          \
//     ZFunc             = 8;
	
// Horizontal offsets (X axis only)
static const float BloomOffsetsX[16] = { 
	0.00000,
	0.00625, 0.0125, 0.01875, 
	0.025, 0.03125, 0.0375, 0.04375,
   -0.00625, -0.0125, -0.01875,
   -0.025, -0.03125, -0.0375, -0.04375,
	0.0046875
};

// Vertical offsets (Y axis only)
static const float BloomOffsetsY[16] =
{
     0.0000000,
     0.0111111,
     0.0222222,
     0.0333333,
     0.0444444,
     0.0555556,
     0.0666667,
     0.0777778,
    -0.0111111,
    -0.0222222,
    -0.0333333,
    -0.0444444,
    -0.0555556,
    -0.0666667,
    -0.0777778,
     0.00833333,   // last one kept correct (Y component)
};


// Weights for both X and Y
static const float BloomWeights[16] =
{
    0.07029905,
    0.08779289,
    0.07029905,
    0.036092736,
    0.08779289,
    0.109640054,
    0.08779289,
    0.045074373,
    0.07029905,
    0.08779289,
    0.07029905,
    0.036092736,
    0.036092736,
    0.045074373,
    0.036092736,
    0.018530628,
};



static const float4 g_avSampleOffsetsBloomPass1[16]
= {
	float4( 0, 0, 0, 0 ),
    float4( 0.00625, 0, 0, 0 ),
    float4( 0.0125, 0, 0, 0 ),
    float4( 0.01875, 0, 0, 0 ),
    float4( 0.025, 0, 0, 0 ),
    float4( 0.03125, 0, 0, 0 ),
    float4( 0.0375, 0, 0, 0 ),
    float4( 0.04375, 0, 0, 0 ),
    float4( -0.00625, -0, -0, -0 ),
    float4( -0.0125, -0, -0, -0 ),
    float4( -0.01875, -0, -0, -0 ),
    float4( -0.025, -0, -0, -0 ),
    float4( -0.03125, -0, -0, -0 ),
    float4( -0.0375, -0, -0, -0 ),
    float4( -0.04375, -0, -0, -0 ),
    float4( 0.0046875, 0.00833333, 0, 0 ),
};

static const float4 g_avSampleOffsetsBloomPass2[16]
= {
	float4( 0, 0, 0, 0 ),
	float4( 0, 0.0111111, 0, 0 ),
	float4( 0, 0.0222222, 0, 0 ),
	float4( 0, 0.0333333, 0, 0 ),
	float4( 0, 0.0444444, 0, 0 ),
	float4( 0, 0.0555556, 0, 0 ),
	float4( 0, 0.0666667, 0, 0 ),
	float4( 0, 0.0777778, 0, 0 ),
	float4( -0, -0.0111111, -0, -0 ),
	float4( -0, -0.0222222, -0, -0 ),
	float4( -0, -0.0333333, -0, -0 ),
	float4( -0, -0.0444444, -0, -0 ),
	float4( -0, -0.0555556, -0, -0 ),
	float4( -0, -0.0666667, -0, -0 ),
	float4( -0, -0.0777778, -0, -0 ),
	float4( 0.0046875, 0.00833333, 0, 0 ),
};

static const float4 g_avSampleWeightsBloom[16]
= {
	float4(0.07029905,0.07029905,0.07029905,1.0),
	float4(0.08779289,0.08779289,0.08779289,1.0),
	float4(0.07029905,0.07029905,0.07029905,1.0),
	float4(0.036092736,0.036092736,0.036092736,1.0),
	float4(0.08779289,0.08779289,0.08779289,1.0),
	float4(0.109640054,0.109640054,0.109640054,1.0),
	float4(0.08779289,0.08779289,0.08779289,1.0),
	float4(0.045074373,0.045074373,0.045074373,1.0),
	float4(0.07029905,0.07029905,0.07029905,1.0),
	float4(0.08779289,0.08779289,0.08779289,1.0),
	float4(0.07029905,0.07029905,0.07029905,1.0),
	float4(0.036092736,0.036092736,0.036092736,1.0),
	float4(0.036092736,0.036092736,0.036092736,1.0),
	float4(0.045074373,0.045074373,0.045074373,1.0),
	float4(0.036092736,0.036092736,0.036092736,1.0),
	float4(0.018530628,0.018530628,0.018530628,1.0),
};

/////////////////////////////////////////////////////////////////////////////////////////

float4 world_position( float4 screen_pos )
{
 	float4 p = mul(screen_pos, WorldViewProj);
	p.xy += ScreenOffset.xy * p.w;
    return p;
}

float4 screen_position( float4 screen_pos )
{
	screen_pos.xy += ScreenOffset.xy;
    return screen_pos;
}

float4 CalcVertexColour(float4 colour)
{
    float4 result = pow(colour, cvVertexPowerBrightness.x) * cvVertexPowerBrightness.y;
    result.w = colour.w;
    return result;
}

float3 ScaleHeadLightIntensity(float3 colour)
{
    float3 result = colour * cvVertexPowerBrightness.z;
    return result;
}

float rand(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719)){
	//make value smaller to avoid artefacts
	float3 smallValue = sin(value);
	//get scalar value from 3d vector
	float random = dot(smallValue, dotDir);
	//make value more random by making it bigger and then taking the factional part
	random = frac(sin(random) * 143758.5453);
	return random;
}

float4 noiseFunc(float4 texCoord) {
	float2 dx_vtc = ddx(texCoord * 32);
	float2 dy_vtc = ddy(texCoord * 32);
	float md = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));
	float mipLevel = max(0, 0.5 * min(6, log2(md) + texCoord.w));
	float resolution = 32.0f * floor(1+mipLevel);
	texCoord = (floor(texCoord * resolution) / resolution);
	float noiseR = saturate(rand3dTo1d(texCoord.xyz + 1346));
	float noiseG = saturate(rand3dTo1d(texCoord.xyz + 6347));
	float noiseB = saturate(rand3dTo1d(texCoord.xyz + 5980));
	return lerp(float4(noiseR, noiseG, (noiseB * 0.5) + 0.5, 1), float4(0.5,0.5,0.75,1), floor(pow(saturate(mipLevel / 6), 2) * 6) / 6);
}

float4 noiseFunc(float3 texCoord) {
	return noiseFunc(float4(texCoord,0));
}

float4 noiseFuncLinear(float4 texCoord) {
	float off = 1.0f/32.0f;
	float4 noise = noiseFunc(texCoord);
	float4 noise_y = noiseFunc(texCoord + float4(0,off,0,0));
	float4 noise_x = noiseFunc(texCoord + float4(off,0,0,0));
	float4 noise_xy = noiseFunc(texCoord + float4(off,off,0,0));
	
	float4 noise_z = noiseFunc(texCoord + float4(0,0,off,0));
	float4 noise_yz = noiseFunc(texCoord + float4(0,off,off,0));
	float4 noise_xz = noiseFunc(texCoord + float4(off,0,off,0));
	float4 noise_xyz = noiseFunc(texCoord + float4(off,1/32,off,0));
	
	float3 pixel = texCoord * 32 + 0.5;
	float3 lerpuv = frac(pixel);
	
	float4 lerp1_top = lerp(noise, noise_x, lerpuv.x);
	float4 lerp1_bottom = lerp(noise_y, noise_xy, lerpuv.x);
	
	float4 lerp1 = lerp(lerp1_top, lerp1_bottom, lerpuv.y);
	
	float4 lerp2_top = lerp(noise_z, noise_xz, lerpuv.x);
	float4 lerp2_bottom = lerp(noise_yz, noise_xyz, lerpuv.x);
	float4 lerp2 = lerp(lerp2_top, lerp2_bottom, lerpuv.y);
	
	return lerp(lerp1, lerp2, lerpuv.z);
}

float4 noiseFuncLinear(float3 texCoord) {
	return noiseFuncLinear(float4(texCoord,0));
}

/////////////////////////////////////////////////////////////////////////////////////////
// HDR Colour Space compression
/////////////////////////////////////////////////////////////////////////////////////////

float3 CompressColourSpace(float3 c)
{
    // filmic response, without implicit gamma
    return (1 - (pow(1 - (c * 0.5), 2)));
}

float3 DeCompressColourSpace(float3 c)
{
    // no curve removal at this time
    return (c * 1.25);
}