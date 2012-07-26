#pragma once
#include "actor.h"

class ISceneNode;

class ClipmapTerrainComponent :
    public ActorComponent
{
private:
    std::shared_ptr<ISceneNode> m_pSceneNode;

public:
    const static ComponentID sm_componentID;

    ClipmapTerrainComponent(void);
    ~ClipmapTerrainComponent(void);

    bool VInit(tinyxml2::XMLElement* p_pData);
    void VPostInit(void);
    void VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis) {}

    std::shared_ptr<ISceneNode> GetSceneNode(void);
    ComponentID VGetComponentID(void) const { return sm_componentID; }

};

