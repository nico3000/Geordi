#pragma once
#include "GraphicsLayer.h"

class ParticleCloud
{
    friend class ParticleSystem;
    friend class ParticleNode;

public:
    struct ParticleDesc
    {
        XMFLOAT2 minMaxMassVariation;
        XMFLOAT2 minMaxSpeedVariation;
        XMFLOAT2 minMaxLifetimeVariation;
        XMFLOAT4 color;
    };

    struct EmitterDesc
    {
        XMFLOAT3 center;
        float mass;
        XMFLOAT3 velocity;
        float lifetime;
    };

private:
    struct Particle
    {
        XMFLOAT3 pos;
        XMFLOAT3 velocity;
        float currentLifetime;
        float mass;
        XMFLOAT4 color;
        XMFLOAT3 deltaVelocity;
        float deltaLifetime;
    };

    struct Emitter
    {
        XMFLOAT3 center;
        XMFLOAT3 velocity;
        float lifetime;
        float unused;
    };

    EmitterDesc m_emitterDesc;
    ParticleDesc m_particleDesc;
    Emitter m_emitter;
    ID3D11Buffer* m_pParticlesData;
    ID3D11Buffer* m_pEmitterData;
    ID3D11ShaderResourceView* m_pParticlesSRV;
    ID3D11UnorderedAccessView* m_pParticlesUAV;
    ID3D11UnorderedAccessView* m_pEmitterUAV;
    unsigned int m_count;

    bool Init(void);

protected:

public:
    ParticleCloud(unsigned int p_count, const EmitterDesc& p_emitterDesc, const ParticleDesc& p_particleDesc);
    ~ParticleCloud(void);

};

