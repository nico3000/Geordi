Texture2D<float4> g_InputTex : register(t0);
RWTexture2D<float4> g_OutputTex : register(u0);

#define KERNEL_SIZE 11
#define TILE_SIZE 128

static const float g_Kernel[KERNEL_SIZE] = { 1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1 };
static const float g_Sum = 1024.0;

groupshared float4 localMem[TILE_SIZE + KERNEL_SIZE - 1];

[numthreads(TILE_SIZE + KERNEL_SIZE - 1, 1, 1)]
void BlurHorCS(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID, uint3 dtid : SV_DispatchThreadID)
{
	uint texCoord = gid.x * TILE_SIZE + gtid.x - KERNEL_SIZE / 2;
	localMem[gtid.x] = g_InputTex[uint2(texCoord, dtid.y)];
	GroupMemoryBarrierWithGroupSync();

	if(gtid.x < TILE_SIZE)
	{
		float4 color = (float4)0;
		for(int i=0; i < KERNEL_SIZE; ++i)
		{
			color += g_Kernel[i] * localMem[gtid.x + i];
		}
		g_OutputTex[uint2(gid.x * TILE_SIZE + gtid.x, dtid.y)] = color / g_Sum;
	}
}


[numthreads(1, TILE_SIZE + KERNEL_SIZE - 1, 1)]
void BlurVerCS(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID, uint3 dtid : SV_DispatchThreadID)
{
	uint texCoord = gid.y * TILE_SIZE + gtid.y - KERNEL_SIZE / 2;
	localMem[gtid.y] = g_InputTex[uint2(dtid.x, texCoord)];
	GroupMemoryBarrierWithGroupSync();

	if(gtid.y < TILE_SIZE)
	{
		float4 color = (float4)0;
		for(int i=0; i < KERNEL_SIZE; ++i)
		{
			color += g_Kernel[i] * localMem[gtid.y + i];
		}
		g_OutputTex[uint2(dtid.x, gid.y * TILE_SIZE + gtid.y)] = color / g_Sum;
	}
}