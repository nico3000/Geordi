#include "StdAfx.h"
#include "ParticleComponent.h"
#include "ParticleNode.h"
#include "ActorEvents.h"


const ComponentID ParticleComponent::sm_componentID = 0x19b4f1d3;



ParticleComponent::ParticleComponent(void)
{
}


ParticleComponent::~ParticleComponent(void)
{
}


bool ParticleComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    ParticleCloud::ParticleDesc particleDesc;
    ParticleCloud::EmitterDesc emitterDesc;
    tinyxml2::XMLElement* pParticlesData = p_pData->FirstChildElement("Particles");
    if(pParticlesData)
    {
        tinyxml2::XMLElement* pCountData = pParticlesData->FirstChildElement("Count");
        if(!pCountData || pCountData->QueryUnsignedAttribute("value", &m_particleCount) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no Particles.count attribute");
            return false;
        }
        tinyxml2::XMLElement* pMassVariationData = pParticlesData->FirstChildElement("MassVariation");
        if(!pMassVariationData || pMassVariationData->QueryFloatAttribute("min", &particleDesc.minMaxMassVariation.x) != tinyxml2::XML_NO_ERROR
                               || pMassVariationData->QueryFloatAttribute("max", &particleDesc.minMaxMassVariation.y) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no MassVariation.min / MassVariation.max attribute");
            return false;
        }
        tinyxml2::XMLElement* pSpeedVariationData = pParticlesData->FirstChildElement("SpeedVariation");
        if(!pSpeedVariationData || pSpeedVariationData->QueryFloatAttribute("min", &particleDesc.minMaxSpeedVariation.x) != tinyxml2::XML_NO_ERROR
                                || pSpeedVariationData->QueryFloatAttribute("max", &particleDesc.minMaxSpeedVariation.y) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no SpeedVariation.min / SpeedVariation.max attribute");
            return false;
        }
        tinyxml2::XMLElement* pLifetimeVariationData = pParticlesData->FirstChildElement("LifetimeVariation");
        if(!pLifetimeVariationData || pLifetimeVariationData->QueryFloatAttribute("min", &particleDesc.minMaxLifetimeVariation.x) != tinyxml2::XML_NO_ERROR
                                   || pLifetimeVariationData->QueryFloatAttribute("max", &particleDesc.minMaxLifetimeVariation.y) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no LifetimeVariation.min / LifetimeVariation.max attribute");
            return false;
        }
        tinyxml2::XMLElement* pColorData = pParticlesData->FirstChildElement("Color");
        if(!pColorData || pColorData->QueryFloatAttribute("r", &particleDesc.color.x) != tinyxml2::XML_NO_ERROR
                       || pColorData->QueryFloatAttribute("g", &particleDesc.color.y) != tinyxml2::XML_NO_ERROR
                       || pColorData->QueryFloatAttribute("b", &particleDesc.color.z) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no Color.r / Color.g / Color.b attribute");
            return false;
        }
    }
    else
    {
        LI_ERROR("no Particles element");
        return false;
    }
    tinyxml2::XMLElement* pEmitterData = p_pData->FirstChildElement("Emitter");
    if(pEmitterData)
    {
        tinyxml2::XMLElement* pCenterData = pEmitterData->FirstChildElement("Center");
        if(!pCenterData || pCenterData->QueryFloatAttribute("x", &emitterDesc.center.x) != tinyxml2::XML_NO_ERROR
                        || pCenterData->QueryFloatAttribute("y", &emitterDesc.center.y) != tinyxml2::XML_NO_ERROR
                        || pCenterData->QueryFloatAttribute("z", &emitterDesc.center.z) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no Center.x / Center.y / Center.z attribute");
            return false;
        }
        tinyxml2::XMLElement* pVelocityData = pEmitterData->FirstChildElement("Velocity");
        if(!pVelocityData || pVelocityData->QueryFloatAttribute("x", &emitterDesc.velocity.x) != tinyxml2::XML_NO_ERROR
                          || pVelocityData->QueryFloatAttribute("y", &emitterDesc.velocity.y) != tinyxml2::XML_NO_ERROR
                          || pVelocityData->QueryFloatAttribute("z", &emitterDesc.velocity.z) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no Velocity.x / Velocity.y / Velocity.z attribute");
            return false;
        }
        tinyxml2::XMLElement* pMassData = pEmitterData->FirstChildElement("Mass");
        if(!pMassData || pMassData->QueryFloatAttribute("value", &emitterDesc.mass) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no Mass.value attribute");
            return false;
        }
        tinyxml2::XMLElement* pLifetimeData = pEmitterData->FirstChildElement("Lifetime");
        if(!pLifetimeData || pLifetimeData->QueryFloatAttribute("value", &emitterDesc.lifetime) != tinyxml2::XML_NO_ERROR)
        {
            LI_ERROR("no Lifetime.value attribute");
            return false;
        }
    }
    else
    {
        LI_ERROR("no Emitter element");
        return false;
    }
    m_pCloud.reset(new ParticleCloud(m_particleCount, emitterDesc, particleDesc));
    return true;
}


void ParticleComponent::VPostInit(void)
{
    IEventDataPtr pEvent(new ParticleComponentCreatedEvent(m_pOwner->GetID()));
    EventManager::Get()->VQueueEvent(pEvent);
}


std::shared_ptr<ISceneNode> ParticleComponent::GetSceneNode(void)
{
    if(!m_pSceneNode)
    {
        m_pSceneNode.reset(new ParticleNode(this->m_pOwner->GetID(), m_pCloud));
    }
    return m_pSceneNode;
}
