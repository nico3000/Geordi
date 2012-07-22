#pragma once
#include "actor.h"

class ISceneNode;
class TerrainData;

class TerrainComponent :
    public ActorComponent
{
private:
    std::shared_ptr<ISceneNode> m_pSceneNode;
    std::shared_ptr<TerrainData> m_pTerrain;

public:
    const static ComponentID sm_componentID;

    TerrainComponent(void);
    ~TerrainComponent(void);

    bool VInit(tinyxml2::XMLElement* p_pData);
    void VPostInit(void);
    
    std::shared_ptr<ISceneNode> GetSceneNode(void) const { return m_pSceneNode; }
    std::shared_ptr<TerrainData> GetTerrain(void) const { return m_pTerrain; }

    void VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis) {}
    ComponentID VGetComponentID(void) const { return sm_componentID; }
};

