#include "StdAfx.h"
#include "TerrainComponent.h"
#include "ActorEvents.h"
#include "TerrainNode.h"


const ComponentID TerrainComponent::sm_componentID = 0x9b217029;


TerrainComponent::TerrainComponent(void) : m_pSceneNode(0)
{
}


TerrainComponent::~TerrainComponent(void)
{
}


bool TerrainComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    return true;
}


void TerrainComponent::VPostInit(void)
{
    IEventDataPtr pEvent(new TerrainComponentCreatedEvent(m_pOwner->GetID()));
    EventManager::Get()->VQueueEvent(pEvent);
}


std::shared_ptr<ISceneNode> TerrainComponent::GetSceneNode(void)
{
    if(!m_pSceneNode)
    {
        m_pSceneNode.reset(new TerrainNode("octree_test"));
    }
    return m_pSceneNode;
}