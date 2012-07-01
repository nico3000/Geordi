#pragma once
#include "ParticleCloud.h"
#include "ConstantBuffer.h"
class ParticleSystem
{
private:
    typedef std::list<std::weak_ptr<ParticleCloud>> ParticleClouds;

    XMFLOAT4 m_time;            // m_time.x: absolute time in seconds, m_time.y: last timestep in seconds
    ConstantBuffer m_timeBuffer;
    ParticleClouds m_clouds;
    ID3D11ComputeShader* m_pComputeShader;
    bool m_initialized;

public:
    ParticleSystem(void);
    ~ParticleSystem(void);

    bool Init(void);
    bool AddParticleCloud(std::weak_ptr<ParticleCloud> p_pParticleCloud);
    void Simulate(unsigned long p_deltaMillis);

    void ActorCreatedDelegate(IEventDataPtr p_pEventData);
    void ActorDestroyedDelegate(IEventDataPtr p_pEventData);

};

