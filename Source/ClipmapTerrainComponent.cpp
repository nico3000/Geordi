#include "StdAfx.h"
#include "ClipmapTerrainComponent.h"
#include "ClipmapTerrainNode.h"
#include "ActorEvents.h"


const ComponentID ClipmapTerrainComponent::sm_componentID = 0xe9d68451;


ClipmapTerrainComponent::ClipmapTerrainComponent(void):
m_pSceneNode(0)
{
}


ClipmapTerrainComponent::~ClipmapTerrainComponent(void)
{
}


bool ClipmapTerrainComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    return true;
}


void ClipmapTerrainComponent::VPostInit(void)
{
    IEventDataPtr pEvent(new ClipmapTerrainComponentCreatedEvent(m_pOwner->GetID()));
    EventManager::Get()->VQueueEvent(pEvent);
}


std::shared_ptr<ISceneNode> ClipmapTerrainComponent::GetSceneNode(void)
{
    if(!m_pSceneNode)
    {
        m_pSceneNode.reset(new ClipmapTerrainNode(m_pOwner->GetID()));
    }
    return m_pSceneNode;
}