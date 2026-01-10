int Cull_Mode : CULL_MODE;
int Blend_State[5] : BLENDSTATE;
texture diffusemap : DiffuseMap; // 1
int BaseMinTextureFilter : BASEMINTEXTUREFILTER;
int BaseMagTextureFilter : BASEMAGTEXTUREFILTER;
int BaseMipTextureFilter : BASEMIPTEXTUREFILTER;
int BaseTextureFilterParam : BASETEXTUREFILTERPARAM;
sampler diffuse_sampler =
sampler_state
{
    Texture = <diffusemap>; // 2
    AddressU = 1;
    AddressV = 1;
    MipFilter = 0;
    MinFilter = 0;
    MagFilter = 0;
    MaxAnisotropy = 0;
};

column_major float4x4 WorldViewProj : WORLDVIEWPROJECTION : register(vs_1_1, c0);
int ColorWriteMode : COLORWRITEMODE;
// car_PixelShader1 Pixel_1_1 Has PRES False
float4 car_PixelShader1() : COLOR
{
    float4 temp0;
    // def c0, 1, 0, 0, 0
    // mov r0, c0
    temp0 = float4(1, 0, 0, 0);
    // 

    return temp0;
}

// car_VertexShader2 Vertex_1_1 Has PRES False
struct car_VertexShader2_Input
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
};

struct car_VertexShader2_Output
{
    float4 position : POSITION;
    float4 texcoord : TEXCOORD;
};

car_VertexShader2_Output car_VertexShader2(car_VertexShader2_Input i)
{
    car_VertexShader2_Output o;
    // dcl_position v0
    // dcl_texcoord v1
    // dp4 oPos.x, v0, c0
    o.position.x = dot(i.position, (WorldViewProj._m00_m10_m20_m30));
    // dp4 oPos.y, v0, c1
    o.position.y = dot(i.position, (WorldViewProj._m01_m11_m21_m31));
    // dp4 oPos.z, v0, c2
    o.position.z = dot(i.position, (WorldViewProj._m02_m12_m22_m32));
    // dp4 oPos.w, v0, c3
    o.position.w = dot(i.position, (WorldViewProj._m03_m13_m23_m33));
    // mov oT0, v1
    o.texcoord = i.texcoord;
    // 

    return o;
}

// car_Expression3 Expression_2_0 Has PRES False
float car_Expression3()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = Cull_Mode.x;
    return expr0;
}

// car_Expression4 Expression_2_0 Has PRES False
float car_Expression4()
{
    float1 expr0;
    // mov c0.x, c4.x
    expr0.x = Blend_State[4].x;
    return expr0;
}

// car_Expression5 Expression_2_0 Has PRES False
float car_Expression5()
{
    float1 expr0;
    // mov c0.x, c3.x
    expr0.x = Blend_State[3].x;
    return expr0;
}

// car_Expression6 Expression_2_0 Has PRES False
float car_Expression6()
{
    float1 expr0;
    // mov c0.x, c2.x
    expr0.x = Blend_State[2].x;
    return expr0;
}

// car_Expression7 Expression_2_0 Has PRES False
float car_Expression7()
{
    float1 expr0;
    // mov c0.x, c1.x
    expr0.x = Blend_State[1].x;
    return expr0;
}

// car_Expression8 Expression_2_0 Has PRES False
float car_Expression8()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = Blend_State[0].x;
    return expr0;
}

// car_Expression9 Expression_2_0 Has PRES False
float car_Expression9()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = ColorWriteMode.x;
    return expr0;
}

// Expression10 Expression_2_0 Has PRES False
float Expression10()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseTextureFilterParam.x;
    return expr0;
}

// Expression11 Expression_2_0 Has PRES False
float Expression11()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMagTextureFilter.x;
    return expr0;
}

// Expression12 Expression_2_0 Has PRES False
float Expression12()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMinTextureFilter.x;
    return expr0;
}

// Expression13 Expression_2_0 Has PRES False
float Expression13()
{
    float1 expr0;
    // mov c0.x, c0.x
    expr0.x = BaseMipTextureFilter.x;
    return expr0;
}

technique car <int shader = 1;>
{
    pass p0
    {
        FogEnable = 0;
        ColorWriteEnable = car_Expression9(); // 0
        AlphaTestEnable = car_Expression8(); // 0
        AlphaRef = car_Expression7(); // 0
        AlphaBlendEnable = car_Expression6(); // 0
        SrcBlend = car_Expression5(); // 0
        DestBlend = car_Expression4(); // 0
        ZFunc = 4;
        CullMode = car_Expression3(); // 0
        VertexShader = compile vs_1_1 car_VertexShader2(); // 3
        PixelShader = compile ps_2_0 car_PixelShader1(); // 4
    }
}

