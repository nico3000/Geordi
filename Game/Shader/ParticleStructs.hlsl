struct Particle
{
    float3 pos;
    float3 velocity;
    float currentLifetime;
    float mass;
    float4 color;
    float3 deltaVelocity;
    float deltaLifetime;
};


struct Emitter
{
    float3 center;
    float3 velocity;
    float lifetime;
    float unused;
};


struct Attractor
{
    float3 center;
    float strength;
};