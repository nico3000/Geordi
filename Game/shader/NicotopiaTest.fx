cbuffer Camera : register(c0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 projectionView;
	float3 eye;
	float viewDistance;
}

struct SimpleVertexIn
{
	float3 positionMC : POSITION0;
	float4 vertexColor : COLOR0;
};

struct SimpleVertexOut
{
	float4 projected : SV_POSITION;
	float4 vertexColor : COLOR0;
};

SimpleVertexOut SimpleVS(SimpleVertexIn input)
{
	SimpleVertexOut output = (SimpleVertexOut)0;
	output.projected = mul(projectionView, float4(input.positionMC, 1.0));
	output.vertexColor = input.vertexColor;
	return output;
}

float4 SimplePS(SimpleVertexOut input) : SV_Target0
{
	return input.vertexColor;
}