#include "CommonDefinitions.hlsl"

struct TerrainVertex
{
    float3 positionWC : POSITION0;
    float3 normalWC : NORMAL0;
    float4x4 material : MATERIAL;
};

struct TerrainVertexProjected
{
    float4 projected : SV_POSITION;
    float3 positionWC : POSITION1;
    float3 normalWC : NORMAL0;
    float4x4 material : MATERIAL;
};


Texture2D g_diffuseTex : register(t0);
Texture2D g_bumpTex : register(t1);
Texture2D g_normalTex : register(t2);


// Vertex Shaders
TerrainVertexProjected TerrainVS(TerrainVertex input)
{
    TerrainVertexProjected output = (TerrainVertexProjected)0;
    output.positionWC = input.positionWC;
    output.projected = mul(g_projectionView, float4(input.positionWC, 1.0));
    output.normalWC = input.normalWC;
    output.material = input.material;
    return output;
}


// Pixel Shaders
DeferredShadingOutput TerrainNormalMappingPS(TerrainVertexProjected input)
{
    DeferredShadingOutput output = (DeferredShadingOutput)0;
    output.world = float4(input.positionWC, 1.0);

    float3 normal = normalize(input.normalWC);

    // Determine the blend weights for the 3 planar projections.  
    // N_orig is the vertex-interpolated normal vector.  
    float3 blend_weights = abs(normal);   // Tighten up the blending zone:  
    blend_weights = (blend_weights - 0.2) * 7;  
    blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)  
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z).xxx;   
    // Now determine a color value and bump vector for each of the 3  
    // projections, blend them, and store blended results in these two  
    // vectors:  

    float2 tex_scale = float2(1, 1);
    float3 blended_bump_vec;  
    {  
        // Compute the UV coords for each of the 3 planar projections.  
        // tex_scale (default ~ 1.0) determines how big the textures appear.  
        float2 coord1 = input.positionWC.yz * tex_scale;  
        float2 coord2 = input.positionWC.zx * tex_scale;  
        float2 coord3 = input.positionWC.xy * tex_scale;  
        // This is where you would apply conditional displacement mapping.  
        //if (blend_weights.x > 0) coord1 = . . .  
        //if (blend_weights.y > 0) coord2 = . . .  
        //if (blend_weights.z > 0) coord3 = . . .  
        // Sample color maps for each projection, at those UV coords.  
        float4 col1 = g_diffuseTex.Sample(AnisotropicSampler, coord1);  
        float4 col2 = g_diffuseTex.Sample(AnisotropicSampler, coord2);  
        float4 col3 = g_diffuseTex.Sample(AnisotropicSampler, coord3);  
        // Sample bump maps too, and generate bump vectors.  
        // (Note: this uses an oversimplified tangent basis.)  
        float2 bumpFetch1 = g_normalTex.Sample(AnisotropicSampler, coord1).xy - 0.5;  
        float2 bumpFetch2 = g_normalTex.Sample(AnisotropicSampler, coord2).xy - 0.5;  
        float2 bumpFetch3 = g_normalTex.Sample(AnisotropicSampler, coord3).xy - 0.5;  
        float3 bump1 = float3(0, bumpFetch1.x, bumpFetch1.y);  
        float3 bump2 = float3(bumpFetch2.y, 0, bumpFetch2.x);  
        float3 bump3 = float3(bumpFetch3.x, bumpFetch3.y, 0);  
        // Finally, blend the results of the 3 planar projections.  
        output.diffuse = col1.xyzw * blend_weights.xxxx +  
                         col2.xyzw * blend_weights.yyyy +  
                         col3.xyzw * blend_weights.zzzz; 
        output.diffuse *= input.material._m00 * float4(1,0,0,1) + input.material._m10 * float4(0,1,0,1);
        blended_bump_vec = bump1.xyz * blend_weights.xxx +  
                           bump2.xyz * blend_weights.yyy +  
                           bump3.xyz * blend_weights.zzz;  
    }  
    // Apply bump vector to vertex-interpolated normal vector.  
    output.normal.xyz = normalize(normal + blended_bump_vec);
    return output;
}