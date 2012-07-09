#include "ParticleStructs.hlsl"
#include "CommonDefinitions.hlsl"

RWStructuredBuffer<Particle> g_Particles : register(u0);
RWStructuredBuffer<Emitter> g_Emitter : register(u1);
StructuredBuffer<Attractor> g_Attractors : register(u2);

groupshared Emitter l_Emitter;
groupshared Particle l_Particles[GROUP_SIZE];


float3x3 Inverse(float3x3 src, float det)
{
    float a = src._11; float b = src._12; float c = src._13;
    float d = src._21; float e = src._22; float f = src._23;
    float g = src._31; float h = src._32; float i = src._33;
    float3x3 adj = { { e*i-f*h, c*h-b*i, b*f-c*e },
                     { f*g-d*i, a*i-c*g, c*d-a*f },
                     { d*h-c*g, b*g-a*h, a*e-b*d } };
    return 1.0 / det * adj;
}


float3 GetAttractorLineDV(float3 pos, float3 lineStart, float3 lineEnd)
{
    float3 posRel = pos - lineStart;
    float3 lineEndRel = lineEnd - lineStart;
    float lineLength = length(lineEndRel);
    float3 lineVec = lineEndRel / lineLength;
    float t = dot(posRel, lineVec);
    float3 vec = clamp(t, 0, lineLength) * lineVec - posRel;

    float r_squared = dot(vec, vec);
    if(r_squared == 0 || r_squared > 0.5) return float3(0, 0, 0);

    float strength = 1e-3 / (1.0 + r_squared);
    return strength * normalize(vec);
}


bool ReflectOnQuad(uint particleID, float3 quadCenter, float3 quadVecU, float3 quadVecV)
{
    Particle p = l_Particles[particleID];
    float3 pEnd = p.pos + g_dTime * p.velocity;
    float3x3 mat = { { quadVecU.x, quadVecV.x, p.pos.x - pEnd.x },
                     { quadVecU.y, quadVecV.y, p.pos.y - pEnd.y },
                     { quadVecU.z, quadVecV.z, p.pos.z - pEnd.z } };
    float det = determinant(mat);
    if(det != 0)
    {
        mat = Inverse(mat, det);
        float3 rst = mul(mat, p.pos - quadCenter);
        if(0 < rst.x && rst.x < 1 && 0 < rst.y && rst.y < 1 && 0 < rst.z && rst.z < 1)
        {
            p.velocity = 0.9 * reflect(p.velocity, normalize(cross(quadVecU, quadVecV)));
            l_Particles[particleID] = p;
            return true;
        }
    }
    return false;
}


bool ReflectOnCuboid(uint particleID, float3 center, float3 extents)
{
    if(ReflectOnQuad(particleID, center - 0.5 * extents, float3(extents.x, 0.0, 0.0), float3(0.0, extents.y, 0.0)))
    {
        return true;
    }
    if(ReflectOnQuad(particleID, center - 0.5 * extents, float3(extents.x, 0.0, 0.0), float3(0.0, 0.0, extents.z)))
    {
        return true;
    }
    if(ReflectOnQuad(particleID, center - 0.5 * extents, float3(0.0, extents.y, 0.0), float3(0.0, 0.0, extents.z)))
    {
        return true;
    }

    if(ReflectOnQuad(particleID, center + 0.5 * extents, -float3(extents.x, 0.0, 0.0), -float3(0.0, extents.y, 0.0)))
    {
        return true;
    }
    if(ReflectOnQuad(particleID, center + 0.5 * extents, -float3(extents.x, 0.0, 0.0), -float3(0.0, 0.0, extents.z)))
    {
        return true;
    }
    if(ReflectOnQuad(particleID, center + 0.5 * extents, -float3(0.0, extents.y, 0.0), -float3(0.0, 0.0, extents.z)))
    {
        return true;
    }
    return false;
}


[numthreads(GROUP_SIZE, 1, 1)]
void SimulateCS(uint3 dtid : SV_DispatchThreadID, uint gid : SV_GroupIndex)
{
    // emitter to groupshared memory
    if(gid == 0)
    {
        l_Emitter = g_Emitter[0];  
    }
    l_Particles[gid] = g_Particles[dtid.x];
    Particle me = g_Particles[dtid.x]; // particle to private and local memory
    

    GroupMemoryBarrierWithGroupSync();
           
    if(me.currentLifetime <= 0.0)
    {
        me.pos = l_Emitter.center;
        me.velocity = me.deltaVelocity + l_Emitter.velocity;
        me.currentLifetime = me.deltaLifetime + l_Emitter.lifetime;
    }

    if(dot(me.pos, me.pos) > 0)
    {
        float3 acc = 0.1 * 9.81 * normalize(-me.pos);
        float3 velo = acc * g_dTime;
        me.velocity += velo;
    }


    //me.velocity += GetAttractorLineDV(me.pos, float3(-3, -2, 0.0), float3(-3, +2, 0.0));
    //me.velocity += GetAttractorLineDV(me.pos, float3(-1, -2, 0.0), float3(-1, +2, 0.0));
    //me.velocity += GetAttractorLineDV(me.pos, float3(-1, -2, 0.0), float3(+1, -2, 0.0));
    //me.velocity += GetAttractorLineDV(me.pos, float3(-1, +2, 0.0), float3(+1, +2, 0.0));
    //me.velocity += GetAttractorLineDV(me.pos, float3(+2, -2, 0.0), float3(+2, +2, 0.0));
    //me.velocity += GetAttractorLineDV(me.pos, float3(+2, +0, 0.0), float3(+4, +0, 0.0));
    //me.velocity += GetAttractorLineDV(me.pos, float3(+4, -2, 0.0), float3(+4, +2, 0.0));
//
    //me.velocity += GetAttractorLineDV(me.pos, float3(0.0, -10.0, 0.0), float3(0.0, +10.0, 0.0));

    me.pos += g_dTime * me.velocity;

    if(ReflectOnCuboid(gid, float3(0,0,0), float3(1,1,1)))
    {
        me = l_Particles[gid];
        //me.color = float4(1, 0, 0, 1);
    }
    
    //if(length(me.pos) > 6) me.currentLifetime = 0;

    //me.velocity *= pow(2.718, -length(0.1 * g_dTime * me.velocity));

    me.currentLifetime = max(0.0, me.currentLifetime - g_dTime);    // update lifetime
    g_Particles[dtid.x] = me;                                      // particle back to global memory
}

