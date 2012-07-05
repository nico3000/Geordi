#include "CommonDefinitions.hlsl"
#include "DeferredShadingDefinitions.hlsl"

Texture2DMS<float4, SAMPLE_COUNT> g_DiffuseTexMS : register(t0);
Texture2DMS<float4, SAMPLE_COUNT> g_ViewTexMS : register(t1);
Texture2DMS<float4, SAMPLE_COUNT> g_NormalTexMS : register(t2);

float4 TexOutMSPS(ScreenQuadFragment input) : SV_Target0
{
    float2 tex;
    float samples;
    g_DiffuseTexMS.GetDimensions(tex.x, tex.y, samples);
	tex *= input.tex;
    float4 color = float4(0,0,0,0);
    for(int i=0; i < samples; ++i)
    {	
		float2 samplePosition = g_DiffuseTexMS.GetSamplePosition(i);
        color += g_DiffuseTexMS.Load(tex + float2(0.5, 0.5), i);
    }
    return color / SAMPLE_COUNT;
}


Texture2D<float4> g_DiffuseTex : register(t0);
Texture2D<float4> g_ViewTex : register(t1);
Texture2D<float4> g_NormalTex : register(t2);

float4 TexOutPS(ScreenQuadFragment input) : SV_Target0
{
    return g_DiffuseTex.Sample(PointSampler, input.tex);
}