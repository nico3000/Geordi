#include "DeferredShadingDefinitions.hlsl"

uint g_sampleCount : register(b3);


Texture2DMS<float4, SAMPLE_COUNT> g_DiffuseTex : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> g_ViewTex : register(t1);
Texture2DMS<float4, SAMPLE_COUNT> g_NormalTex : register(t2);


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