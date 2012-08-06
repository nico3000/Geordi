Texture2D<float4> g_Input : register(t0);
RWTexture2D<float4> g_PrimaryOutput : register(u0);

Texture2D<float4> g_Source : register(t1);

#define KERNEL_SIZE 11
#define TILE_SIZE 128

static const float g_Kernel[KERNEL_SIZE] = { 1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1 };
static const float g_Sum = 1024.0;
static const float3 g_BrightnessFactors = float3(0.2126, 0.7152, 0.0722);

groupshared float4 localMem[TILE_SIZE + KERNEL_SIZE - 1];

/**
 * Blurt g_Input horizontal nach g_PrimaryOutput
 * Kopiert g_Input nach g_SecondaryOutput
 */
[numthreads(TILE_SIZE + KERNEL_SIZE - 1, 1, 1)]
void BlurHorCS(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID, uint3 dtid : SV_DispatchThreadID)
{
    uint texCoord = gid.x * TILE_SIZE + gtid.x - KERNEL_SIZE / 2;
    float4 texel = g_Input[uint2(texCoord, dtid.y)];
    float brightness = dot(texel.rgb, g_BrightnessFactors);
    localMem[gtid.x] = ceil(clamp(brightness, 0.0, 2.0) - 1.0) * texel;

    GroupMemoryBarrierWithGroupSync();

    if(gtid.x < TILE_SIZE)
    {
        float4 color = (float4)0;
        for(int i=0; i < KERNEL_SIZE; ++i)
        {
            color += g_Kernel[i] * localMem[gtid.x + i];
        }
        g_PrimaryOutput[uint2(gid.x * TILE_SIZE + gtid.x, dtid.y)] = color / g_Sum;
    }
}


/**
 * Blurt g_Input vertikal nach g_PrimaryOutput und addiert g_Source drauf
 */
[numthreads(1, TILE_SIZE + KERNEL_SIZE - 1, 1)]
void BlurVerCS(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID, uint3 dtid : SV_DispatchThreadID)
{
    uint texCoord = gid.y * TILE_SIZE + gtid.y - KERNEL_SIZE / 2;
    localMem[gtid.y] = g_Input[uint2(dtid.x, texCoord)];
    GroupMemoryBarrierWithGroupSync();

    if(gtid.y < TILE_SIZE)
    {
        float4 color = (float4)0;
        for(int i=0; i < KERNEL_SIZE; ++i)
        {
            color += g_Kernel[i] * localMem[gtid.y + i];
        }
        g_PrimaryOutput[uint2(dtid.x, gid.y * TILE_SIZE + gtid.y)] = g_Source[uint2(dtid.x, gid.y * TILE_SIZE + gtid.y)] + color / g_Sum;
    }
}