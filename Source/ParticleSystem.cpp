#include "StdAfx.h"
#include "ParticleSystem.h"
#include "ActorEvents.h"
#include "ParticleComponent.h"

#define LI_LOGGER_TAG "ParticleSystem"

ParticleSystem::ParticleSystem(void) :
m_pComputeShader(0), m_initialized(false)
{
}


ParticleSystem::~ParticleSystem(void)
{
    SAFE_RELEASE(m_pComputeShader);
}


bool ParticleSystem::AddParticleCloud(std::weak_ptr<ParticleCloud> p_pParticleCloud)
{
    std::shared_ptr<ParticleCloud> pCloud = p_pParticleCloud.lock();
    if(pCloud && pCloud->Init())
    {
        m_clouds.push_back(p_pParticleCloud);
        return true;
    }
    else
    {
        LI_WARNING("ParticleCloud rejected");
        return false;
    }
}


bool ParticleSystem::Init(void)
{
    ID3D10Blob* pShaderBlob = 0;    
    ID3D10Blob* pErrorBlob = 0;
    HRESULT hr = D3DX11CompileFromFileA("./Shader/ParticleSystemCS.hlsl", 0, 0, "SimulateCS", "cs_5_0", 0, 0, 0, &pShaderBlob, &pErrorBlob, 0);
    if(FAILED(hr))
    {
        if(pErrorBlob)
        {
            std::string str((char*)pErrorBlob->GetBufferPointer(), (char*)pErrorBlob->GetBufferPointer() + pErrorBlob->GetBufferSize());
            LI_ERROR(str);
            SAFE_RELEASE(pErrorBlob);
        }
        else
        {
            RETURN_IF_FAILED(hr);
        }
        return false;
    }
    else
    {
        LI_LOG_WITH_TAG("Compute shader was successfully compiled to run on hardware.");
    }
    hr = LostIsland::g_pGraphics->GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), 0, &m_pComputeShader);
    SAFE_RELEASE(pErrorBlob);
    SAFE_RELEASE(pShaderBlob);
    RETURN_IF_FAILED(hr);

    if(!m_timeBuffer.BuildFromSharedData(&m_time, sizeof(XMFLOAT4)))
    {
        return false;
    }

    EventListenerDelegate actorCreated = fastdelegate::MakeDelegate(this, &ParticleSystem::ActorCreatedDelegate);
    EventManager::Get()->VAddListener(actorCreated, ActorCreatedEvent::sm_eventType);

    EventListenerDelegate actorDestroyed = fastdelegate::MakeDelegate(this, &ParticleSystem::ActorDestroyedDelegate);
    EventManager::Get()->VAddListener(actorDestroyed, ActorDestroyedEvent::sm_eventType);

    return m_initialized = true;
}


#define GROUP_SIZE 256 // WARNING: also in shader ParticleSystemCS.hlsl!!!
void ParticleSystem::Simulate(unsigned long p_deltaMillis)
{
    if(!m_initialized)
    {
        LI_ERROR("ParticleSystem not initialized!");
    }
    m_time.y = 1e-3f * (float)p_deltaMillis;
    m_time.x += m_time.y;
    m_timeBuffer.Update();
    m_timeBuffer.Bind(2, TARGET_CS);

    LostIsland::g_pGraphics->GetContext()->CSSetShader(m_pComputeShader, 0, 0);
    ID3D11UnorderedAccessView* ppUAVs[3] = { 0, 0, 0 };
    unsigned int pCount[3] = { 0, 0, 0 };

    auto iter = m_clouds.begin();
    while(iter != m_clouds.end())
    {
        std::shared_ptr<ParticleCloud> pCloud = (*iter).lock();
        if(pCloud)
        {
            ppUAVs[0] = pCloud->m_pParticlesUAV;
            ppUAVs[1] = pCloud->m_pEmitterUAV;
            ppUAVs[2] = 0;
            unsigned int groupCount = pCloud->m_count / GROUP_SIZE;
            LostIsland::g_pGraphics->GetContext()->CSSetUnorderedAccessViews(0, 3, ppUAVs, pCount);
            LostIsland::g_pGraphics->GetContext()->Dispatch(groupCount, 1, 1);
            ++iter;
        }
        else
        {
            iter = m_clouds.erase(iter);
            LI_LOG_WITH_TAG("ParticleCloud removed");
        }
    }
    ppUAVs[0] = 0;
    ppUAVs[1] = 0;
    ppUAVs[2] = 0;
    LostIsland::g_pGraphics->GetContext()->CSSetUnorderedAccessViews(0, 3, ppUAVs, pCount);
    
}


void ParticleSystem::ActorCreatedDelegate(IEventDataPtr p_pEventData)
{
    StrongActorPtr pActor = LostIsland::g_pApp->GetGameLogic()->VGetActor(std::static_pointer_cast<ActorCreatedEvent>(p_pEventData)->GetActorID()).lock();
    if(pActor)
    {
        std::shared_ptr<ParticleComponent> pComp = pActor->GetComponent<ParticleComponent>(ParticleComponent::sm_componentID).lock();
        if(pComp)
        {
            if(this->AddParticleCloud(pComp->GetParticleCloud()))
            {
                std::ostringstream str;
                str << "ParticleCloud with " << pComp->GetParticleCount() << " particles added";
                LI_LOG_WITH_TAG(str.str());
            }
        }
    }
}


void ParticleSystem::ActorDestroyedDelegate(IEventDataPtr p_pEventData)
{

}