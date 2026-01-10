#define CAR_SELF_SHADOW 1
//
// Car Effects
//
#include "global.h"

float4x4 WorldView       : WORLDVIEW ;
float4x4 LocalDirectionMatrix	: LOCALDIRECTIONMATRIX;
float4x4 LocalColourMatrix		: LOCALCOLOURMATRIX;

float4 LocalEyePos : LOCALEYEPOS;
//float4 CarLocalEyePos;

float  MetallicScale	 : METALLICSCALE;
float  SpecularHotSpot	 : SPECULARHOTSPOT;

float4 DiffuseMin		 : DIFFUSEMIN;
float4 DiffuseRange		 : DIFFUSERANGE;
float4 SpecularMin		 : SPECULARMIN;
float4 SpecularRange     : SPECULARRANGE;
float4 _EnvmapMin		 : ENVMAPMIN;
float4 _EnvmapRange      : ENVMAPANGE;
#define EnvmapMin		 (_EnvmapMin/2)
#define EnvmapRange		 (_EnvmapRange/2)
float  SpecularPower	 : SPECULARPOWER;
float  EnvmapPower		 : ENVMAPPOWER;

float4 ShadowColour		 : CARSHADOWCOLOUR; 

float g_fInvShadowStrength : INVSHADOWSTRENGTH;

const int	g_bDoCarShadowMap = 1;//: SHADOWMAP_CAR_SHADOW_ENABLED;

float FocalRange		 : FOCALRANGE;

float RVMSkyBrightness	 : RVM_SKY_BRIGHTNESS;
float RVMWorldBrightness : RVM_WORLD_BRIGHTNESS;

float Desaturation				: DESATURATION;

float4	Coeffs0					: CURVE_COEFFS_0;
float4	Coeffs1					: CURVE_COEFFS_1;
float4	Coeffs2					: CURVE_COEFFS_2;
float4	Coeffs3					: CURVE_COEFFS_3;

float	CombinedBrightness		: COMBINED_BRIGHTNESS;

const bool IS_NORMAL_MAPPED = 1;
const bool HAS_METALIC_FLAKE;

shared texture DIFFUSEMAP_TEXTURE       : DiffuseMap;
sampler DIFFUSEMAP_SAMPLER = sampler_state
{
	Texture = <DIFFUSEMAP_TEXTURE>;
	AddressU = WRAP;
	AddressV = WRAP;
	MIPFILTER = LINEAR;
	MINFILTER = <BaseMinTextureFilter>;
	MAGFILTER = <BaseMagTextureFilter>;
};

shared texture NORMALMAP_TEXTURE       : NormalMapTexture;
sampler NORMALMAP_SAMPLER = sampler_state
{
	Texture = <NORMALMAP_TEXTURE>;
	AddressU = WRAP;
	AddressV = WRAP;
	MIPFILTER = LINEAR;
	MINFILTER = <BaseMinTextureFilter>;
	MAGFILTER = <BaseMagTextureFilter>;
};


shared texture VOLUMEMAP_TEXTURE       : VolumeMap;
sampler3D VOLUMEMAP_SAMPLER = sampler_state
{
	Texture = <VOLUMEMAP_TEXTURE>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
	MIPFILTER = POINT;
	MINFILTER = POINT;
	MAGFILTER = POINT;
};

shared texture ENVIROMAP_TEXTURE       : EnvMapTexture;
samplerCUBE ENVIROMAP_SAMPLER = sampler_state
{
	Texture = <ENVIROMAP_TEXTURE>;
	AddressU = MIRROR;
	AddressV = MIRROR;
	MIPFILTER = LINEAR;
	MINFILTER = <BaseMinTextureFilter>;
	MAGFILTER = <BaseMagTextureFilter>;
};

struct VS_INPUT
{
	float4 position : POSITION;
	float4 normal   : NORMAL;
	float4 color    : COLOR;
	float4 tex		: TEXCOORD;
	float4 tangent	: TANGENT;
};

struct PS_OUTPUT
{
	float4 color : COLOR0;
};

#include "shadowmap_fx_def.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// Car Shader Normalmap Shaders
//
//

struct VtoP
{
	float4 position       : POSITION;
	half4  diffuse		  : COLOR0;
	half4  specular		  : COLOR1;
	half4  t0             : TEXCOORD0;	// Environment Map
	half4  t1             : TEXCOORD1;	// Base Texture Map
	half4  t2             : TEXCOORD2;	// Shadow Map
	half4  view			  : TEXCOORD3;
	half4  normal		  : TEXCOORD4;
	half4  lights[2]	  : TEXCOORD5;
	float4  localPos		  : TEXCOORD7;
};

VtoP vertex_shader_normalmap(const VS_INPUT IN)
{
	VtoP OUT;
	OUT.position = world_position(IN.position);
	OUT.normal	= IN.normal;
	OUT.t1	= IN.tex;
	OUT.t2  = vertex_shadow_tex( IN.position );
	OUT.t2.z -= 0.0025;	// bias shadowmap
	
	// Calculate parameters use for the grazingto facing colour shift
	// based on the viewing angle
	float3 ViewDir = LocalEyePos - IN.position;

	// Metallic paint distance falloff coeff
	OUT.view.w = saturate(length(ViewDir)*-0.25 + 1.2);	// line equation
	
	half3 view_vector = normalize(ViewDir); 
	half vdotn		= dot( view_vector, IN.normal );
		 vdotn		= max( 0.01f, vdotn );
	half specvdotn	= pow( vdotn, SpecularPower );
	half envvdotn	= pow( vdotn, EnvmapPower );
	OUT.diffuse  = DiffuseMin  + (vdotn	 * DiffuseRange);
	OUT.specular = SpecularMin + (specvdotn * SpecularRange);
	OUT.specular.w	= envvdotn;

	// Transform view and lights from local to tangent space
	float4 tangent;
	float3 c1 = cross(IN.normal, float3(0.0, 0.0, 1.0));
	float3 c2 = cross(IN.normal, float3(0.0, 1.0, 0.0));
	
	if (length(c1)>length(c2))
	{
		tangent.xyz = c1;
	}
	else
	{
		tangent.xyz = c2;
	}
	
	float3 binormal = cross( tangent.xyz, -IN.normal.xyz );
	binormal = normalize(binormal);
	
	tangent.xyz = normalize(tangent.xyz);
	tangent.w = (dot(cross(IN.normal, tangent.xyz), binormal) < 0.0F) ? -1.0F : 1.0F;
	half3x3 mToTangent;
	mToTangent[0]		= tangent;
	mToTangent[2]		= IN.normal;
	mToTangent[1]		= cross( mToTangent[2], mToTangent[0] ) * tangent.w;
	OUT.lights[0].xyz	= mul( mToTangent, LocalDirectionMatrix._m00_m10_m20);
	OUT.lights[1].xyz	= mul( mToTangent, LocalDirectionMatrix._m01_m11_m21);
	OUT.view.xyz		= mul( mToTangent, ViewDir );
	// If normal mapped then the reflection to view calculations are done in pixel space
	OUT.t0				= 0;
	
	// We have run out of textcord slots so move the world position into the w components
	// of the normal and light vectors
	OUT.normal.w = IN.position.x;
	OUT.lights[0].w = IN.position.y;
	OUT.lights[1].w = IN.position.z;
	
	OUT.localPos = 1;

	//float4 shadowTex = vertex_shadow_tex( IN.position );
	//OUT.t1.z = shadowTex.x;
	//OUT.t1.w = shadowTex.y;
	
	OUT.t0.w = IN.color.x;

	return OUT; 
}

float4 pixel_shader_normalmap(const VtoP IN) : COLOR0
{
	//
	// Implement phong specular model with environmental mapping
	// 
	half4 diffuse_sample	= tex2D(DIFFUSEMAP_SAMPLER, IN.t1.xy);
    
    half3 viewDir			= normalize(IN.view.xyz);			// V

	float3 position			= float3(IN.normal.w, IN.lights[0].w, IN.lights[1].w);
	half4 noise_sample		= noiseFunc(float4(position*20, -3));//tex3Dbias(VOLUMEMAP_SAMPLER, float4(position*20, -3));
	half3 flakeNoise		= (noise_sample - 0.5) * 2;
	half	shadow = 1.0;
	if ( g_bDoCarShadowMap )
	{
		shadow = DoShadow( IN.t2, 1 );
	}
	//half shadow				= DoShadowMapAlpha( IN.t2 );
	
	// Extract the normal from the map
	half4 norm_sample	= tex2Dbias(NORMALMAP_SAMPLER, IN.t1); // normal map
	norm_sample = (norm_sample - 0.5)*2;		//convert between unsigned and signed normal map data
	half3 normal = normalize(norm_sample);			// N
	
	// Calculate the reflection componenet using the normal maps normal
	half vdotn     = dot( viewDir, normal );
	vdotn			= max( 0.01f, vdotn );
	half4 envmapTex = float4(2.0f*vdotn*normal - viewDir, 0.0f); // R = 2 * (N.V) * N - V
	
	half4 envmap_sample = texCUBE( ENVIROMAP_SAMPLER, envmapTex);
	half3 envmap_color		= envmap_sample * 2 * (EnvmapMin + IN.specular.w * EnvmapRange);
	
	// Accumulate the diffuse for both lights
	#ifndef E3_CARSHADER
		half  ndotL1 = dot(normal, IN.lights[0].xyz);				// N.L1
		half  ndotL2 = dot(normal, IN.lights[1].xyz);				// N.L2
		half3 diffuse = saturate(ndotL1+0.3) * LocalColourMatrix[0].xyz  * 1.0;
		diffuse		  += saturate(ndotL2+0.3) * LocalColourMatrix[1].xyz * 0.85;
	#else
		half  ndotL1 = dot(normal, IN.lights[0].xyz);				// N.L1
		half  ndotL2 = dot(normal, -IN.lights[0].xyz);				// N.L2
		half3 diffuse = saturate(ndotL1+0.3) * LocalColourMatrix[0].xyz  * 1.0;
		diffuse		  += saturate(ndotL2+0.3) * LocalColourMatrix[1].xyz * 1.75;
	#endif
	
	// Apply a metallic flake to the specular
	half3 normalFlake = normalize(normal + flakeNoise*0.04*0*0.35f*IN.view.w);
	half  ndotL1Flake = dot(normalFlake, IN.lights[0].xyz);				// N.L1
	// Calculate the specular for just the first light (i.e. the sun)
	half3 reflection  = 2*normalFlake*ndotL1Flake - IN.lights[0].xyz;		// R = 2 * (N.L1) * N - L1

	// Base specular falloff	
	half3 reflectDotView = saturate(dot(reflection, viewDir));
	half3 specular = pow(reflectDotView, SpecularPower) * 0.7; // S = (R.V) ^ n
	// Hot spot specular - helps define the sun centre
	float _SpecularHotSpot = 1.0f * round(saturate(SpecularMin * 2));
	specular += pow(reflectDotView, SpecularPower*100) * shadow.r * _SpecularHotSpot;			// S = (R.V) ^ n
	specular *= LocalColourMatrix[0].xyz;
	
	// Calculate the self shadow
	//float3 selfShadow = saturate(4 * ndotL1);	// self-shadowing term 
	
	half ambientOcclusion = IN.t0.w;
	
	half4 result;
	#ifndef E3_CARSHADER
		result.xyz = diffuse * IN.diffuse;// * ambientOcclusion;						// diffuse
	#else
		result.xyz = diffuse * IN.diffuse * ambientOcclusion;						// diffuse
	#endif
	result.xyz *= diffuse_sample;
	result.xyz += IN.diffuse.w * specular * IN.specular * shadow.r * ambientOcclusion;			// specular
	result.xyz *= lerp(ShadowColour.xyz, 1, saturate(shadow.r + g_fInvShadowStrength));											// shadow;
	result.xyz += IN.diffuse.w * envmap_color * ambientOcclusion;							// environ gloss mapping (2x)
	result.w   = IN.diffuse.w * diffuse_sample.w;												// alpha
	
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Car Shader
//
//

VtoP vertex_shader(const VS_INPUT IN)
{
	VtoP OUT;
	OUT.position = world_position(IN.position);
	OUT.normal	= IN.normal;
	OUT.t1	= IN.tex;
	OUT.t2  = vertex_shadow_tex( IN.position );
	#if defined(CAR_SELF_SHADOW)
	OUT.t2.z -= 0.0025;	// bias shadowmap
	#else
	OUT.t2.z -= 0.12;	// bias shadowmap
	#endif
		
	#ifdef E3_CARSHADER
		float radiosity = IN.color;		// vertex colours: 0.5 = 1.0 to allow for overbrightening
	#else
		float radiosity = saturate(IN.color.x*2);		// vertex colours: 0.5 = 1.0 to allow for overbrightening
	#endif
	
	// Calculate parameters use for the grazingto facing colour shift
	// based on the viewing angle
	float3 ViewDir = LocalEyePos - IN.position;

	// Metallic paint distance falloff coeff
	OUT.view.w = saturate(length(ViewDir)*-0.25 + 1.2);	// line equation
	OUT.view.w *= 0.04*MetallicScale * 0.35f;
	
	half3 view_vector = normalize(ViewDir); 
	half vdotn		= dot( view_vector, IN.normal );
		 vdotn		= max( 0.01f, vdotn );
	half specvdotn	= pow( vdotn, SpecularPower );
	half envvdotn	= pow( vdotn, EnvmapPower );
	// Grazing=Min, Facing = Min+Range
	//OUT.diffuse  = DiffuseMin*0.1  + (vdotn * DiffuseRange/0.1);
	OUT.diffuse  = DiffuseMin  + (vdotn * DiffuseRange);
	OUT.specular = SpecularMin + (specvdotn * SpecularRange);
	OUT.specular *= radiosity;
	OUT.specular.w	= (EnvmapMin.x + envvdotn * EnvmapRange.x);
	OUT.specular.w *= radiosity;

	// Leave view and lights in local space
	OUT.view.xyz		= ViewDir;
	OUT.lights[0].xyz	= LocalDirectionMatrix._m00_m10_m20;
	OUT.lights[1].xyz	= OUT.lights[0].xyz;
	OUT.lights[1].w		= OUT.lights[0].w = 0;
	// Environment mapping texture coords
	half4 reflection = half4(2.0f*vdotn*IN.normal - view_vector, 0.0f); // R = 2 * (N.V) * N - V
	OUT.t0 = half4(mul( reflection, WorldView ).xyz, 1.0f);

	// Accumulate the diffuse for both lights
	#ifndef E3_CARSHADER
		half  ndotL1 = dot(IN.normal, LocalDirectionMatrix._m00_m10_m20);				// N.L1
		half  ndotL2 = dot(IN.normal, LocalDirectionMatrix._m01_m11_m21);				// N.L2
		half  ndotL3 = dot(IN.normal, LocalDirectionMatrix._m02_m12_m22);				// N.L2
		half3 diffuse  = saturate(ndotL1+0.1)  * LocalColourMatrix[0].xyz * 1.0;
		diffuse		  += saturate(ndotL2+0.1) * LocalColourMatrix[1].xyz * 0.85;
		diffuse		  += saturate(ndotL3)     * LocalColourMatrix[2].xyz * 0.85;
	#else
		half  ndotL1 = dot(IN.normal, LocalDirectionMatrix._m00_m10_m20);				// N.L1
		half  ndotL2 = dot(IN.normal, -LocalDirectionMatrix._m00_m10_m20);				// N.L2
		half3 diffuse  = saturate(ndotL1+0.3)  * LocalColourMatrix[0].xyz * 1.0;
		diffuse		  += saturate(ndotL2+0.3) * LocalColourMatrix[1].xyz * 1.75;
	#endif
	OUT.diffuse.xyz *= diffuse * radiosity;
	
	// We have run out of textcord slots so move the world position into the w components
	// of the normal and light vectors
	OUT.localPos    = IN.position * 20;
	OUT.localPos.w = 0;
	
	return OUT; 
}

float4 pixel_shader(const VtoP IN) : COLOR0
{
	//
	// Implement phong specular model with environmental mapping
	// 
	half4	diffuse_sample	= tex2D(DIFFUSEMAP_SAMPLER, IN.t1.xy);
	float3	position		= IN.localPos;
	half4	noise_sample	= noiseFunc(float4(position, -3)); //tex3Dbias(VOLUMEMAP_SAMPLER, float4(position, -3));
	half3	flakeNoise		= (noise_sample - 0.5) * 2;
	half	shadow = 1.0;

	if ( g_bDoCarShadowMap )
	{
		shadow = DoShadow( IN.t2, 1 );
	}

	// Use the vertex normal
	half3 normal = normalize(IN.normal.xyz);				// N

	half4 envmap_sample = texCUBE( ENVIROMAP_SAMPLER, IN.t0);
	// The environment maps alpha channel stores a light bloom mask
	half3 envmap_bloom = 0;//envmap_sample.w*envmap_sample.xyz*2;
	half3 envmap_color = (envmap_sample + envmap_bloom) * IN.specular.w; 

	// Apply a metallic flake to the specular
    half3 viewDir	  = normalize(IN.view.xyz);			// V
	half3 normalFlake = normalize(normal + flakeNoise*IN.view.w);
	half  ndotL1Flake = dot(normalFlake, IN.lights[0].xyz);				// N.L1

	// Calculate the specular for just the first light (i.e. the sun)
	// Base specular falloff	
	half3 reflection  = 2*normalFlake*ndotL1Flake - IN.lights[0].xyz;		// R = 2 * (N.L1) * N - L1
	half3 rdotV = saturate(dot(reflection, viewDir));
	half3 specular = pow(rdotV, SpecularPower); // S = (R.V) ^ n
	// Hot spot specular - helps define the sun centre
	float _SpecularHotSpot = 1.0f * round(saturate(SpecularMin * 2));
	#ifndef E3_CARSHADER
		half3 hotSpot = pow(rdotV, SpecularPower*80) * shadow.r * _SpecularHotSpot * 5;
	#else
		half3 hotSpot = pow(rdotV, SpecularPower*100) * shadow.r * _SpecularHotSpot;			// S = (R.V) ^ n
	#endif
	specular += hotSpot;			// S = (R.V) ^ n
	specular *= LocalColourMatrix[0].xyz;
	
	#ifdef CAR_SELF_SHADOW
		float3 selfShadow = saturate(4 * dot(IN.normal, LocalDirectionMatrix._m00_m10_m20)); // early blackbox code
		shadow = saturate(shadow + (1-selfShadow));
	#endif
	
	half4 result;
	result.xyz = IN.diffuse * diffuse_sample;								// diffuse
	result.xyz += IN.diffuse.w * specular * IN.specular * shadow.r;			// specular
	result.xyz *= lerp(ShadowColour.xyz, 1, saturate(shadow.r + g_fInvShadowStrength));						// shadow;
	result.xyz += IN.diffuse.w * envmap_color;								// environ gloss mapping
	result.w   =  IN.diffuse.w * diffuse_sample.w;		// alpha
	
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Low Lod Shader
//
//
struct VtoP_lowlod
{
	float4 position       : POSITION;
	half4  diffuse		  : COLOR0;
	half4  specular		  : COLOR1;
	half4  t0             : TEXCOORD0;	// Environment Map
	half4  t1             : TEXCOORD1;	// Base Texture Map
	half4  t2             : TEXCOORD2;	// Shadow Map
	half4  view			  : TEXCOORD3;
	half4  normal		  : TEXCOORD4;
	half3  lights[1]	  : TEXCOORD5;
	half3  localPos		  : TEXCOORD6;
};

VtoP_lowlod vertex_shader_lowlod(const VS_INPUT IN)
{
	VtoP_lowlod OUT;
	OUT.position = world_position(IN.position);
	OUT.normal	= IN.normal;
	OUT.t1	= IN.tex;
	OUT.t2  = vertex_shadow_tex( IN.position );
	
	// Calculate parameters use for the grazingto facing colour shift
	// based on the viewing angle

	half3 view_vector = normalize(LocalEyePos - IN.position); 
	half vdotn		= dot( view_vector, IN.normal );
		 vdotn		= max( 0.01f, vdotn );
	half specvdotn	= pow( vdotn, SpecularPower );
	half envvdotn	= pow( vdotn, EnvmapPower );
	OUT.diffuse		= DiffuseMin  + (vdotn	 * DiffuseRange);
	OUT.specular = SpecularMin + (specvdotn * SpecularRange);
	OUT.specular.w	= 2 * IN.color.x * (EnvmapMin.x + envvdotn * EnvmapRange.x);

	// Leave view and lights in local space
	OUT.view.xyz		= view_vector;
	OUT.view.w			= 1;
	OUT.lights[0].xyz	= LocalDirectionMatrix._m00_m10_m20;
	// Environment mapping texture coords
	half4 reflection = half4(2.0f*vdotn*IN.normal - view_vector, 0.0f); // R = 2 * (N.V) * N - V
	OUT.t0 = half4(mul( reflection, WorldView ).xyz, 1.0f);
	
	// We have run out of textcord slots so move the world position into the w components
	// of the normal and light vectors
	OUT.localPos    = IN.position;

	// Accumulate the diffuse for both lights
	half  ndotL1	= dot(IN.normal, LocalDirectionMatrix._m00_m10_m20);				// N.L1
	half  ndotL2	= dot(IN.normal, LocalDirectionMatrix._m01_m11_m21);				// N.L2
	half3 diffuse	= saturate(ndotL1+0.3) * LocalColourMatrix[0].xyz;
	diffuse			+= saturate(ndotL2+0.3) * LocalColourMatrix[1].xyz * 0.8;
	OUT.diffuse.xyz	*= diffuse * IN.color.x;
	
	// Calculate the specular for just the first light (i.e. the sun)
	half3 reflectionSpec  = 2*IN.normal*ndotL1 - OUT.lights[0].xyz;		// R = 2 * (N.L1) * N - L1
	half3 reflectDotView = saturate(dot(reflectionSpec, view_vector));
	half3 specular = pow(reflectDotView, SpecularPower) * 0.7; // S = (R.V) ^ n
	specular += pow(reflectDotView, SpecularPower*100) * SpecularHotSpot;			// S = (R.V) ^ n
	// Hot spot specular - helps define the sun centre
	specular *= LocalColourMatrix[0].xyz;
	OUT.specular.xyz *= specular * IN.color.x * OUT.diffuse.w; 
	// OUT.specular.xyz = 0;
	
	return OUT; 
}

float4 pixel_shader_lowlod(const VtoP_lowlod IN) : COLOR0
{
	half4 diffuse_sample	= tex2D(DIFFUSEMAP_SAMPLER, IN.t1.xy);
	half4 envmap_sample		= texCUBE( ENVIROMAP_SAMPLER, IN.t0);
	half3 envmap_color		= envmap_sample * IN.specular.w;
	half  shadow = 1.0;
	
	if ( g_bDoCarShadowMap )
	{
		shadow = DoShadow( IN.t2, 1 );
	}

	half4 result;
	result.xyz = IN.diffuse;			// diffuse
	result.xyz *= diffuse_sample;
	result.xyz *= lerp(ShadowColour.xyz, 1, saturate(shadow.r + g_fInvShadowStrength));						// shadow;
	//result.xyz += IN.specular;;			// specular
	result.xyz += IN.diffuse.w * envmap_color;							// environ gloss mapping (2x)
	result.w   = IN.diffuse.w * diffuse_sample.w;												// alpha
	
	//result.xyz = float3(0, 1, 0);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Rear View Mirror Shader
//
//
struct VtoP_RVM
{
	float4 position       : POSITION;
	half4  t0             : TEXCOORD0;	// Opacity Texture Map
	half4  t1             : TEXCOORD1;	// Environment Map
};

VtoP_RVM vertex_shader_rvm(const VS_INPUT IN)
{
//	struct VS_INPUT
//	{
//		float4 position : POSITION;		// Position in screen space
//		float4 normal   : NORMAL;		// Texture coordinates for opacity texture
//		float4 color    : COLOR;
//		float4 tex		: TEXCOORD;		// Look up for environment map
//		float4 tangent	: TANGENT;
//	};

	VtoP_RVM OUT;
	OUT.position	= IN.position;
	OUT.t0			= IN.normal;
	OUT.t1			= IN.tex;

	return OUT; 
}

float4 pixel_shader_rvm(const VtoP_RVM IN) : COLOR0
{
	float4 result;

	float4 normal = normalize(IN.t0);

	half4 opacity = tex2D(DIFFUSEMAP_SAMPLER,IN.t1.xy);
	half4 envmap = texCUBE(ENVIROMAP_SAMPLER,normal);
	
	result.xyz = envmap.xyz * RVMWorldBrightness;
	
	// Get the luminance from the full screen image
	float luminance = dot( result.xyz, LUMINANCE_VECTOR );
	
	// compute the curves 
	float4 curve = Coeffs3*luminance + Coeffs2; 
	curve = curve*luminance + Coeffs1;                  
	curve = curve*luminance + Coeffs0;
	
	// Desaturate the original image by blending between the screen and the luminance
	float3 desatScreen = luminance.xxx + Desaturation * (result.xyz - luminance.xxx);

	// Black Bloom screen
	float3 bb_result = desatScreen * curve.x;

	// Add screen result to colour bloom
	result.xyz = bb_result + curve.yzw * result.xyz;

	// Brightness masking
	result.xyz *= CombinedBrightness;

	result.w = opacity.w;

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Normalmap switching logic
//
//
float4 cmax(float4 a, float4 b) {
	return float4(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z), max(a.w,b.w));
}

VtoP vertex_shader_compare(const VS_INPUT IN)
{
	float4 check = 0;
	// checks multiple places in texture to make damn sure that any difference is caught
	half checkPrecision = 32;
	for (int i=0; i<=checkPrecision; i++) {
		for (int j=0; j<=checkPrecision; j++) {
			float4 compuv = float4(j/checkPrecision, i/checkPrecision,0,0);
			float4 normsmp = tex2Dlod(NORMALMAP_SAMPLER, compuv);
			float4 diffsmp = tex2Dlod(DIFFUSEMAP_SAMPLER, compuv);
			check = cmax(check, normsmp - diffsmp);
		}
	}
	check = ceil(saturate(check));
	
	// ultra jank that works 99.9% of the time
	if ((max(max(check.x, check.y), max(check.z, check.w)) > 0))
		return vertex_shader_normalmap(IN);
	else
		return vertex_shader(IN);
}

float4 pixel_shader_compare(const VtoP IN) : COLOR0
{
	if (IN.localPos.w > 0)
		return pixel_shader_normalmap(IN);	// run carnormalmap if normalmapped
	else
		return pixel_shader(IN);			// run regular car shader if not
}

#ifndef NO_CARNORMALMAP
technique carnormalmap <int shader = 1;>
{
    pass p0
    {
		AlphaTestEnable = (Blend_State[0]);
		AlphaRef = (Blend_State[1]);
		AlphaBlendEnable = (Blend_State[2]);
		SrcBlend = (Blend_State[3]);
		DestBlend = (Blend_State[4]);
		CullMode = (Cull_Mode);
		
        VertexShader = compile vs_3_0 vertex_shader_compare();
        PixelShader  = compile ps_3_0 pixel_shader_compare();
    }
}
#endif

technique car <int shader = 1;> // fallback hopefully
{
    pass p0
    {
		AlphaTestEnable = (Blend_State[0]);
		AlphaRef = (Blend_State[1]);
		AlphaBlendEnable = (Blend_State[2]);
		SrcBlend = (Blend_State[3]);
		DestBlend = (Blend_State[4]);
		CullMode = (Cull_Mode);
		
        VertexShader = compile vs_1_1 vertex_shader();
        PixelShader  = compile ps_3_0 /*aa*/ pixel_shader();
    }
}

technique lowlod <int shader = 1;>
{
    pass p0
    {
		AlphaTestEnable = (Blend_State[0]);
		AlphaRef = (Blend_State[1]);
		AlphaBlendEnable = (Blend_State[2]);
		SrcBlend = (Blend_State[3]);
		DestBlend = (Blend_State[4]);
		CullMode = (Cull_Mode);
		
		VertexShader = compile vs_3_0 vertex_shader_lowlod();
		PixelShader  = compile ps_3_0 pixel_shader_lowlod();
		//VertexShader = compile vs_3_0 vertex_shader();
		//PixelShader  = compile ps_3_0 pixel_shader();
    }
}

technique rvm <int shader = 1;>
{
    pass p0
    {
		AlphaTestEnable = (Blend_State[0]);
		AlphaRef = (Blend_State[1]);
		AlphaBlendEnable = (Blend_State[2]);
		SrcBlend = (Blend_State[3]);
		DestBlend = (Blend_State[4]);
		CullMode = (Cull_Mode);
		
		VertexShader = compile vs_3_0 vertex_shader_rvm();
		PixelShader  = compile ps_3_0 pixel_shader_rvm();
    }
}

//#include "ZPrePass_fx.h"

#include "shadowmap_fx.h"

