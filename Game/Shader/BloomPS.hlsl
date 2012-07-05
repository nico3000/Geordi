#include "CommonDefinitions.hlsl"
#include "DeferredShadingDefinitions.hlsl"


#define KERNEL_SIZE 11
#define TILE_SIZE 128

static const float g_Kernel[KERNEL_SIZE] = { 1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1 };
static const float g_Sum = 1024.0;
static const float3 g_BrightnessFactors = float3(0.2126, 0.7152, 0.0722);


Texture2D<float4> g_SourceTex : register(t0);
Texture2D<float4> g_BlurredTex : register(t1);


float4 BlurHorPS(ScreenQuadFragment input) : SV_Target0
{
	return g_SourceTex.Sample(PointSampler, input.tex);

	uint width, height;
	g_SourceTex.GetDimensions(width, height);
	float2 offset = float2(0.0, 0.0);
	offset.x = 1.0 / (float)width;
	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	for(int i=0; i < KERNEL_SIZE; ++i)
	{
		float4 texel = g_SourceTex.Sample(PointSampler, input.tex + ((float)i - floor(KERNEL_SIZE / 2)) * offset);
		float brightness = dot(texel.rgb, g_BrightnessFactors);
		color += ceil(clamp(brightness, 0.0, 2.0) - 1.0) * texel * g_Kernel[i];
	}
    return color / g_Sum;
}


float4 BlurVerPS(ScreenQuadFragment input) : SV_Target0
{
	return g_SourceTex.Sample(PointSampler, input.tex);

	uint width, height;
	g_BlurredTex.GetDimensions(width, height);
	float2 offset = float2(0.0, 0.0);
	offset.y = 1.0 / (float)height;
	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	for(int i=0; i < KERNEL_SIZE; ++i)
	{
		float4 texel = g_BlurredTex.Sample(PointSampler, input.tex + ((float)i - floor(KERNEL_SIZE / 2)) * offset);

		color += texel * g_Kernel[i];
	}
    return g_SourceTex.Sample(PointSampler, input.tex) + color / g_Sum;
}