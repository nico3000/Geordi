#include "StdAfx.h"
#include "ParticleCloud.h"


ParticleCloud::ParticleCloud(unsigned int p_count, const EmitterDesc& p_emitterDesc, const ParticleDesc& p_particleDesc) :
m_count(p_count), m_emitterDesc(p_emitterDesc), m_particleDesc(p_particleDesc), m_pParticlesData(0), m_pParticlesSRV(0), m_pParticlesUAV(0)
{
}


ParticleCloud::~ParticleCloud(void)
{
    SAFE_RELEASE(m_pParticlesUAV);
    SAFE_RELEASE(m_pParticlesSRV);
    SAFE_RELEASE(m_pParticlesData);
 
    SAFE_RELEASE(m_pEmitterUAV);
    SAFE_RELEASE(m_pEmitterData);
}


bool ParticleCloud::Init(void)
{
    HRESULT hr = S_OK;

    // begin emitter data
    m_emitter.center = m_emitterDesc.center;
    m_emitter.lifetime = m_emitterDesc.lifetime;
    m_emitter.velocity = m_emitterDesc.velocity;

    D3D11_BUFFER_DESC emitterBufferDesc;
    emitterBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    emitterBufferDesc.ByteWidth = sizeof(Emitter);
    emitterBufferDesc.CPUAccessFlags = 0;
    emitterBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    emitterBufferDesc.StructureByteStride = sizeof(Emitter);
    emitterBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA emitterData;
    emitterData.pSysMem = &m_emitter;
    emitterData.SysMemPitch = 0;
    emitterData.SysMemSlicePitch = 0;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateBuffer(&emitterBufferDesc, &emitterData, &m_pEmitterData);
    RETURN_IF_FAILED(hr);

    D3D11_UNORDERED_ACCESS_VIEW_DESC emitterUAViewDesc;
    emitterUAViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    emitterUAViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    emitterUAViewDesc.Buffer.FirstElement = 0;
    emitterUAViewDesc.Buffer.Flags = 0;
    emitterUAViewDesc.Buffer.NumElements = 1;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateUnorderedAccessView(m_pEmitterData, &emitterUAViewDesc, &m_pEmitterUAV);
    RETURN_IF_FAILED(hr);
    // end emitter data

    // begin particle data
    Particle *pParticles = new Particle[m_count];
    ZeroMemory(pParticles, m_count * sizeof(Particle));
    for(unsigned int i=0; i < m_count; ++i)
    {
        float phi = XM_2PI * (float)rand() / (float)RAND_MAX;
        float theta = XM_PI * (float)rand() / (float)RAND_MAX;
        float radius = m_particleDesc.minMaxSpeedVariation.x + (m_particleDesc.minMaxSpeedVariation.y - m_particleDesc.minMaxSpeedVariation.x) * ((float)rand() / (float)RAND_MAX);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        pParticles[i].deltaVelocity.x = radius * sinTheta * sinPhi;
        pParticles[i].deltaVelocity.y = radius * sinTheta * cosPhi;
        pParticles[i].deltaVelocity.z = radius * cosTheta;
        pParticles[i].deltaLifetime = m_particleDesc.minMaxLifetimeVariation.x + (m_particleDesc.minMaxLifetimeVariation.y - m_particleDesc.minMaxLifetimeVariation.x) * (float)rand() / (float)RAND_MAX;
        pParticles[i].currentLifetime = pParticles[i].deltaLifetime;
        pParticles[i].color = m_particleDesc.color;
        pParticles[i].mass = m_emitterDesc.mass + m_particleDesc.minMaxMassVariation.x + (m_particleDesc.minMaxMassVariation.y - m_particleDesc.minMaxMassVariation.x) * (float)rand() / (float)RAND_MAX;
    }

    D3D11_BUFFER_DESC particleBufferDesc;
    particleBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    particleBufferDesc.ByteWidth = m_count * sizeof(Particle);
    particleBufferDesc.CPUAccessFlags = 0;
    particleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    particleBufferDesc.StructureByteStride = sizeof(Particle);
    particleBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA particleData;
    particleData.pSysMem = pParticles;
    particleData.SysMemPitch = 0;
    particleData.SysMemSlicePitch = 0;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateBuffer(&particleBufferDesc, &particleData, &m_pParticlesData);
    SAFE_DELETE(pParticles);
    RETURN_IF_FAILED(hr);

    D3D11_UNORDERED_ACCESS_VIEW_DESC particlesUAViewDesc;
    particlesUAViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    particlesUAViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    particlesUAViewDesc.Buffer.FirstElement = 0;
    particlesUAViewDesc.Buffer.Flags = 0;
    particlesUAViewDesc.Buffer.NumElements = m_count;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateUnorderedAccessView(m_pParticlesData, &particlesUAViewDesc, &m_pParticlesUAV);
    RETURN_IF_FAILED(hr);

    D3D11_SHADER_RESOURCE_VIEW_DESC particlesSRViewDesc;
    particlesSRViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    particlesSRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    particlesSRViewDesc.Buffer.ElementOffset = 0;
    particlesSRViewDesc.Buffer.ElementWidth = sizeof(Particle);
    particlesSRViewDesc.Buffer.FirstElement = 0;
    particlesSRViewDesc.Buffer.NumElements = m_count;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateShaderResourceView(m_pParticlesData, &particlesSRViewDesc, &m_pParticlesSRV);
    RETURN_IF_FAILED(hr);
    // end particle data

    return true;
}
