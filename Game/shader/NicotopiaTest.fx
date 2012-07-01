#include "CommonDefinitions.hlsl"
#include "ParticleStructs.hlsl"

Texture2D g_Tex : register(t1);

struct SimpleVertex
{
	float3 positionMC : POSITION0;
	float4 vertexColor : COLOR0;
};

struct SimpleVertexProjected
{
    float4 view : POSITION1;
	float4 projected : SV_POSITION;
	float4 vertexColor : COLOR0;
};


struct DeferredShadingOutput
{
    float4 diffuse : SV_Target0;
    float4 view : SV_Target1;
    float4 normal : SV_Target2;
};

// vertex shaders

SimpleVertexProjected SimpleVS(SimpleVertex input)
{
	SimpleVertexProjected output = (SimpleVertexProjected)0;
    output.view = mul(g_view, mul(g_model, float4(input.positionMC, 1.0)));
	output.projected = mul(g_projection, output.view);
    output.view.z /= g_viewDistance;
	output.vertexColor = input.vertexColor;
	return output;
}


StructuredBuffer<Particle> g_Particles : register(t0);

SimpleVertexProjected ParticleVS(uint vid : SV_VertexID)
{
	SimpleVertexProjected output = (SimpleVertexProjected)0;
    output.view = mul(g_view, mul(g_model, float4(g_Particles[vid].pos, 1.0)));
	output.projected = mul(g_projection, output.view);
    output.view.z /= g_viewDistance;
	//output.vertexColor = float4(0.5 + 0.5 * normalize(g_Particles[vid].velocity), 1.0);
    output.vertexColor = g_Particles[vid].color;
    //output.vertexColor *= 0.5;
	return output;
}

// pixel shaders

DeferredShadingOutput SimplePS(SimpleVertexProjected input)
{
    DeferredShadingOutput output = (DeferredShadingOutput)0;
	//return lerp(g_Tex.Sample(MeshTextureSampler, input.world.xy), input.vertexColor, 0.5);
    output.view = input.view;
    output.diffuse = float4(input.vertexColor.rgb, 0.5);
    return output;
}