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


//RWTexture2D<float4> g_backbuffer : register(u0);
//Texture2D<float4> g_finalImage : register(t0);
//
//#define KERNEL_SIZE 7
//#define TILE_SIZE 24
//#define NUM_THREADS TILE_SIZE + KERNEL_SIZE - 1
//groupshared float4 g_tile[NUM_THREADS][NUM_THREADS];
//
//[numthreads(NUM_THREADS, NUM_THREADS, 1)] 
//void PostFXCS2(uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID)
//{
	//uint2 groupTexelOffset = gid.xy * float2(TILE_SIZE, TILE_SIZE);
	//uint2 texelID = groupTexelOffset + gtid.xy - float2(KERNEL_SIZE / 2, KERNEL_SIZE / 2);
	//g_tile[gtid.x][gtid.y] = g_finalImage[texelID];
	//GroupMemoryBarrierWithGroupSync();
//
	//if(gtid.x < TILE_SIZE && gtid.y < TILE_SIZE)
	//{
		//const uint weights[KERNEL_SIZE] = { 1, 6, 15, 20, 15, 6, 1};
		//const float sum = 4096.0;
//
		//float4 color = float4(0.0, 0.0, 0.0, 0.0);
		//for(int y=0; y < KERNEL_SIZE; ++y)
		//{
			//for(int x=0; x < KERNEL_SIZE; ++x)
			//{
				//float weight = weights[x] * weights[y];
				//color += weight * g_tile[gtid.x + x][gtid.y + y];
			//}
		//}
		//color /= sum;
		////float brightness = dot(float3(0.2126, 0.7152, 0.0722), color.rgb);
		//g_backbuffer[groupTexelOffset + gtid.xy] = g_tile[gtid.x + KERNEL_SIZE / 2][gtid.y + KERNEL_SIZE / 2] + color;
	//}
//}
//
//
//[numthreads(TILE_SIZE, TILE_SIZE, 1)] 
//void PostFXCS(uint3 dtid : SV_DispatchThreadID)
//{
	//const uint weights[KERNEL_SIZE] = { 1, 6, 15, 20, 15, 6, 1};
	//const float sum = 4096.0;
//
	//float4 color = float4(0.0, 0.0, 0.0, 0.0);
	//for(int y=0; y < KERNEL_SIZE; ++y)
	//{
		//for(int x=0; x < KERNEL_SIZE; ++x)
		//{
			//float weight = weights[x] * weights[y];
			//uint2 texel = dtid.xy - uint2(x - KERNEL_SIZE / 2, y - KERNEL_SIZE / 2);
			//color += weight * g_finalImage[texel];
		//}
	//}
	//color /= sum;
	////float brightness = dot(float3(0.2126, 0.7152, 0.0722), color.rgb);
	//g_backbuffer[dtid.xy] = g_finalImage[dtid.xy] + color;
//}
//
//
//
////#define TILE_SIZE 16
//[numthreads(TILE_SIZE, TILE_SIZE, 1)] 
//void PostFXCS3(uint3 dtid : SV_DispatchThreadID)
//{
    //const int kernelSize = 3;
    //const uint weights[3][3] = { { 1, 0, -1 },
							     //{ 2, 0, -2 },
							     //{ 1, 0, -1 }, };
    //float sum = 1.0;
//
    //float4 color = float4(0,0,0,0);
    //for(int x=-kernelSize / 2; x <= kernelSize / 2; ++x)
    //{
        //for(int y=-kernelSize / 2; y <= kernelSize / 2; ++y)
        //{
            //color += weights[y + kernelSize / 2][x + kernelSize / 2] * g_finalImage[dtid.xy + int2(x, y)];
        //}
    //}
	//color /= sum;
    //g_backbuffer[dtid.xy] = color;
//}