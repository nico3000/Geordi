#pragma once
#include "actor.h"
#include "ISceneNode.h"
class ParticleComponent :
    public ActorComponent
{
private:
    std::shared_ptr<ISceneNode> m_pSceneNode;
    std::shared_ptr<ParticleCloud> m_pCloud;
    unsigned int m_particleCount;

public:
    const static ComponentID sm_componentID;

    ParticleComponent(void);
    ~ParticleComponent(void);

    bool VInit(tinyxml2::XMLElement* p_pData);
    std::shared_ptr<ISceneNode> GetSceneNode(void);

    std::weak_ptr<ParticleCloud> GetParticleCloud(void) const { return m_pCloud; }
    unsigned int GetParticleCount(void) const { return m_particleCount; }
    ComponentID VGetComponentID(void) const { return sm_componentID; }
};

