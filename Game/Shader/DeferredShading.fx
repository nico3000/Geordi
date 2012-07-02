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

Texture2DMS<float4, SAMPLE_COUNT> g_DiffuseTex : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> g_ViewTex : register(t1);
Texture2DMS<float4, SAMPLE_COUNT> g_NormalTex : register(t2);


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


RWTexture2D<float4> g_backbuffer : register(u0);
Texture2D<float4> g_finalImage : register(t0);

#define TILE_SIZE 16
[numthreads(TILE_SIZE, TILE_SIZE, 1)] 
void PostFXCS(uint3 dtid : SV_DispatchThreadID)
{
    const int kernelSize = 9;
    const uint weights[9] = { 1, 8, 28, 56, 70, 56, 28, 7, 1 };
    float sum = 4096.0;

    float4 color = float4(0,0,0,0);
    for(int x=-kernelSize / 2; x <= kernelSize / 2; ++x)
    {
        for(int y=-kernelSize / 2; y <= kernelSize / 2; ++y)
        {
            color += weights[y + kernelSize / 2] * weights[x + kernelSize / 2] * g_finalImage[dtid.xy + int2(x, y)];
        }
    }
    g_backbuffer[dtid.xy] = color / sum;
}