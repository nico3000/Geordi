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


#define GROUP_SIZE 16
bool ParticleSystem::Init(void)
{
    ID3D10Blob* pShaderBlob = 0;    
    ID3D10Blob* pErrorBlob = 0;
    std::ostringstream groupSize;
    groupSize << GROUP_SIZE;
    std::string groupSizeStr = groupSize.str();
    D3D10_SHADER_MACRO pDefines[2] = {
        { "GROUP_SIZE", groupSizeStr.c_str() },
        { 0, 0 },
    };
//     m_pComputeShader = LostIsland::g_pGraphics->CompileComputeShader("./Shader/ParticleSystemCS.hlsl", "SimulateCS", pDefines);
//     if(!m_pComputeShader)
//     {
//         return false;
//     }

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


void ParticleSystem::Simulate(unsigned long p_deltaMillis)
{
    static unsigned long dTime = 0;
    dTime += p_deltaMillis;
    if(dTime < 20 || p_deltaMillis == 0)
    {
        return;
    }
    if(!m_initialized)
    {
        LI_ERROR("ParticleSystem not initialized!");
    }
    m_time.y = 1e-3f * (float)dTime;
    m_time.x += m_time.y;
    m_timeBuffer.Update();
    m_timeBuffer.Bind(2, TARGET_CS);
    dTime = 0;

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