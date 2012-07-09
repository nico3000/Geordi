cbuffer Camera : register(b0)
{
	float4x4 g_view;
	float4x4 g_projection;
	float4x4 g_projectionView;
	float3 g_camPosition;
	float g_viewDistance;
    float g_aspect;
    float3 Camera_unused;
};


cbuffer ModelProperties : register(b1)
{
    float4x4 g_model;
    float4x4 g_modelIT;
};


cbuffer Timing : register(b2)
{
    float g_time;
    float g_dTime;
    float2 Timing_unused;
};

SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);
SamplerState AnisotropicSampler : register(s2);