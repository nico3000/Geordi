#include "CommonDefinitions.hlsl"

struct TileProperties
{
    float3 globalTranslation;
    float scale;
    float3 tileTranslation;
    float level;
    float gridSize;
    float3 unused;
};


cbuffer TilePropertiesBuffer : register(b3)
{
	TileProperties g_properties;
};

Texture2DArray g_clipmaps : register(t7);

SimpleVertexProjected ClipmapVS(SimpleVertex input)
{
    float extremeX = 0.5f * (float)(g_properties.gridSize + 1);
    float extremeZ = 0.5f * (float)(g_properties.gridSize - 1);

	SimpleVertexProjected output = (SimpleVertexProjected)0;
    float3 localPos = input.positionMC + g_properties.tileTranslation;
    float2 tex = (float2(extremeX, extremeZ) + localPos.xz) / (g_properties.gridSize + 1.0);
    float height = g_clipmaps.Gather(PointSampler, float3(tex, g_properties.level)).r;
    float3 globalPos = g_properties.scale * (g_properties.globalTranslation + localPos) + float3(0.0, height, 0.0);

    float height_x = g_clipmaps.Gather(PointSampler, float3(tex, g_properties.level), uint2(1,0)).r;
    float height_y = g_clipmaps.Gather(PointSampler, float3(tex, g_properties.level), uint2(0,1)).r;
    float3 tangent = float3(g_properties.scale, height_x - height, 0.0);
    float3 binormal = float3(0.0, height_y - height, g_properties.scale);

    output.tex = tex;

    output.view = mul(g_view, mul(g_model, float4(globalPos, 1.0)));
	output.projected = mul(g_projection, output.view);
    //output.normalWC = mul(transpose(g_modelInv), float4(input.normalMC, 0)).xyz;
    output.normalWC = normalize(mul(transpose(g_modelInv), float4(cross(binormal, tangent), 0)).xyz);
	output.vertexColor = input.vertexColor;
    if(length(globalPos) < 1e-3)
    {
        //output.vertexColor = float4(1,0,0,1);
    }
	return output;
}


DeferredShadingOutput ClipmapPS(SimpleVertexProjected input)
{
    DeferredShadingOutput output = (DeferredShadingOutput)0;
    output.view = input.view;
    output.normal = float4(normalize(input.normalWC), 0.0);
    output.diffuse = input.vertexColor;//g_clipmaps.Sample(PointSampler, float3(input.tex, g_properties.level));
    return output;
}