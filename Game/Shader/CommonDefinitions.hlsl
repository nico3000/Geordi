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
    float4x4 g_modelInv;
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


struct DeferredShadingOutput
{
    float4 diffuse : SV_Target0;
    float4 world : SV_Target1;
    float4 normal : SV_Target2;
};


struct SimpleVertex
{
	float3 positionMC : POSITION0;
    float3 normalMC : NORMAL0;
	float4 vertexColor : COLOR0;
};

struct SimpleVertexProjected
{
    float4 world : POSITION1;
	float4 projected : SV_POSITION;
    float3 normalWC : NORMAL0;
	float4 vertexColor : COLOR0;
    float2 tex : TEXCOORD0;
};