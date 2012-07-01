#pragma once
#include "BaseSceneNode.h"
#include "ShaderProgram.h"

class ParticleNode :
    public BaseSceneNode
{
private:
    std::weak_ptr<ParticleCloud> m_pCloud;

    // TODO!!!
    ShaderProgram m_program;

public:
    ParticleNode(ActorID p_actorID, std::weak_ptr<ParticleCloud> p_pCloud);
    ~ParticleNode(void);

    HRESULT VPreRender(Scene* p_pScene);
    HRESULT VRender(Scene* p_pScene);

};

