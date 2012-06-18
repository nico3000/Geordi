cbuffer Camera : register(c0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 projectionView;
	float3 camPosition;
	float viewDistance;
}

cbuffer ModelProperties : register(c1)
{
    float4x4 model;
    float4x4 modelIT;
};

struct SimpleVertex
{
	float3 positionMC : POSITION0;
	float4 vertexColor : COLOR0;
};

struct SimpleVertexProjected
{
	float4 projected : SV_POSITION;
	float4 vertexColor : COLOR0;
};

SimpleVertexProjected SimpleVS(SimpleVertex input)
{
	SimpleVertexProjected output = (SimpleVertexProjected)0;
	output.projected = mul(projection, mul(view, mul(model, float4(input.positionMC, 1.0))));
	output.vertexColor = input.vertexColor;
	return output;
}

float4 SimplePS(SimpleVertexProjected input) : SV_Target0
{
	return input.vertexColor;
}