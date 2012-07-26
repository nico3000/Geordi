#include "CommonDefinitions.hlsl"

struct TileProperties
{
	float scale;
	float3 translation;
};


cbuffer TilePropertiesBuffer : register(b3)
{
	TileProperties g_properties;
};


SimpleVertexProjected ClipmapVS(SimpleVertex input)
{
	SimpleVertexProjected output = (SimpleVertexProjected)0;
    output.view = mul(g_view, mul(g_model, float4(input.positionMC, 1.0)));
	output.projected = mul(g_projection, output.view);
    output.normalWC = mul(transpose(g_modelInv), float4(input.normalMC, 0)).xyz;
	output.vertexColor = input.vertexColor;
	return output;
}


DeferredShadingOutput ClipmapPS(SimpleVertexProjected input)
{
    DeferredShadingOutput output = (DeferredShadingOutput)0;
    output.view = input.view;
    output.normal = float4(normalize(input.normalWC), 0.0);
    output.diffuse = input.vertexColor;
    return output;
}