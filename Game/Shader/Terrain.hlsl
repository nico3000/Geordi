#include "CommonDefinitions.hlsl"

struct TerrainVertex
{
    float3 positionWC : POSITION0;
    float3 normalWC : NORMAL0;
    float matmix : MATMIX;
    int material0 : MATERIAL0;
    int material1 : MATERIAL1;
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


// Util functions
struct ColorAndNormal
{
	float4 color;
	float3 normal;
};


ColorAndNormal GetFromMaterial(int material, float3 positionWC, float3 normal)
{
	ColorAndNormal output = (ColorAndNormal)0;
	const float2 texScale = float2(1, 1);

    // Determine the blend weights for the 3 planar projections.  
    // N_orig is the vertex-interpolated normal vector.  
    float3 blend_weights = abs(normal);   // Tighten up the blending zone:  
    blend_weights = (blend_weights - 0.2) * 7;  
    blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)  
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z).xxx;   
    // Now determine a color value and bump vector for each of the 3  
    // projections, blend them, and store blended results

    // Compute the UV coords for each of the 3 planar projections.  
    // tex_scale (default ~ 1.0) determines how big the textures appear.  
    float2 coord1 = positionWC.yz * texScale;  
    float2 coord2 = positionWC.zx * texScale;  
    float2 coord3 = positionWC.xy * texScale;  
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
    output.color = col1.xyzw * blend_weights.xxxx +  
                   col2.xyzw * blend_weights.yyyy +  
                   col3.xyzw * blend_weights.zzzz; 
    output.normal = bump1.xyz * blend_weights.xxx +  
                    bump2.xyz * blend_weights.yyy +  
                    bump3.xyz * blend_weights.zzz; 
	return output; 
}


// Vertex Shaders
TerrainVertexProjected TerrainVS(TerrainVertex input)
{
    TerrainVertexProjected output = (TerrainVertexProjected)0;
    output.positionWC = input.positionWC;
    output.projected = mul(g_projectionView, float4(input.positionWC, 1.0));
    output.normalWC = input.normalWC;
    output.material._m00 += (input.material0 == 0 ? input.matmix : 0.0) + (input.material1 == 0 ? 1.0 - input.matmix : 0.0);
    output.material._m01 += (input.material0 == 1 ? input.matmix : 0.0) + (input.material1 == 1 ? 1.0 - input.matmix : 0.0);
    output.material._m02 += (input.material0 == 2 ? input.matmix : 0.0) + (input.material1 == 2 ? 1.0 - input.matmix : 0.0);
    output.material._m03 += (input.material0 == 3 ? input.matmix : 0.0) + (input.material1 == 3 ? 1.0 - input.matmix : 0.0);
    output.material._m10 += (input.material0 == 4 ? input.matmix : 0.0) + (input.material1 == 4 ? 1.0 - input.matmix : 0.0);
    output.material._m11 += (input.material0 == 5 ? input.matmix : 0.0) + (input.material1 == 5 ? 1.0 - input.matmix : 0.0);
    output.material._m12 += (input.material0 == 6 ? input.matmix : 0.0) + (input.material1 == 6 ? 1.0 - input.matmix : 0.0);
    output.material._m13 += (input.material0 == 7 ? input.matmix : 0.0) + (input.material1 == 7 ? 1.0 - input.matmix : 0.0);
    output.material._m20 += (input.material0 == 8 ? input.matmix : 0.0) + (input.material1 == 8 ? 1.0 - input.matmix : 0.0);
    output.material._m21 += (input.material0 == 9 ? input.matmix : 0.0) + (input.material1 == 9 ? 1.0 - input.matmix : 0.0);
    output.material._m22 += (input.material0 == 10 ? input.matmix : 0.0) + (input.material1 == 10 ? 1.0 - input.matmix : 0.0);
    output.material._m23 += (input.material0 == 11 ? input.matmix : 0.0) + (input.material1 == 11 ? 1.0 - input.matmix : 0.0);
    output.material._m30 += (input.material0 == 12 ? input.matmix : 0.0) + (input.material1 == 12 ? 1.0 - input.matmix : 0.0);
    output.material._m31 += (input.material0 == 13 ? input.matmix : 0.0) + (input.material1 == 13 ? 1.0 - input.matmix : 0.0);
    output.material._m32 += (input.material0 == 14 ? input.matmix : 0.0) + (input.material1 == 14 ? 1.0 - input.matmix : 0.0);
    output.material._m33 += (input.material0 == 15 ? input.matmix : 0.0) + (input.material1 == 15 ? 1.0 - input.matmix : 0.0);
    return output;
}


// Pixel Shaders
DeferredShadingOutput TerrainNormalMappingPS(TerrainVertexProjected input)
{
    DeferredShadingOutput output = (DeferredShadingOutput)0;
    
    float3 normal = normalize(input.normalWC);
    ColorAndNormal con = GetFromMaterial(0, input.positionWC, normal);

    // Apply bump vector to vertex-interpolated normal vector.  
	output.diffuse = input.material._m00 * float4(0.0, 1.0, 0.0, 1.0) + 
                     input.material._m01 * float4(1.0, 1.0, 0.0, 1.0) + 
                     input.material._m02 * float4(1.0, 0.0, 0.0, 1.0) + 
                     input.material._m03 * float4(0.0, 0.0, 1.0, 1.0);//*/con.color;
    output.normal = float4(normalize(normal + con.normal), 0.0);
	output.world = float4(input.positionWC, 1.0);
    return output;
}