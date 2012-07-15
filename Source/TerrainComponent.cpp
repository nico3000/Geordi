#include "StdAfx.h"
#include "TerrainComponent.h"
#include "ActorEvents.h"


const ComponentID TerrainComponent::sm_componentID = 0x9b217029;


TerrainComponent::TerrainComponent(void)
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