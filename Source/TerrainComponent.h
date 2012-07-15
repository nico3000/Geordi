#pragma once
#include "actor.h"
class TerrainComponent :
    public ActorComponent
{
public:
    const static ComponentID sm_componentID;

    TerrainComponent(void);
    ~TerrainComponent(void);

    bool VInit(tinyxml2::XMLElement* p_pData);
    void VPostInit(void);

    void VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis) {}
    ComponentID VGetComponentID(void) const { return sm_componentID; }
};

