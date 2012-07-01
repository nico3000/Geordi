#include "CommonDefinitions.hlsl"

struct ScreenQuadVertex
{
    float2 position : POSITION0;
};


struct ScreenQuadFragment
{
    float4 position : SV_Position;
    float2 tex : TEXCOORD0;
};


uint g_sampleCount : register(b3);

//#define SAMPLE_COUNT 1.0
Texture2DMS<float4, SAMPLE_COUNT> g_DiffuseTex : register(t1);
Texture2DMS<float4, SAMPLE_COUNT> g_ViewTex : register(t2);
Texture2DMS<float4, SAMPLE_COUNT> g_NormalTex : register(t3);


ScreenQuadFragment ScreenQuadVS(ScreenQuadVertex input)
{
    ScreenQuadFragment output = (ScreenQuadFragment)0;
    output.position = float4(input.position, 0.0, 1.0);
    output.tex = 0.5 + float2(+0.5, -0.5) * input.position;
    return output;
}


float4 TexOutPS(ScreenQuadFragment input) : SV_Target0
{
    uint width;
    uint height;
    uint samples;
    g_DiffuseTex.GetDimensions(width, height, samples);
    float4 color = float4(0,0,0,0);
    for(int i=0; i < SAMPLE_COUNT; ++i)
    {
        color += g_DiffuseTex.Load(input.tex * float2(width, height), i);
    }
    return color / SAMPLE_COUNT;
}