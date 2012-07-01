#pragma once
#include "BaseSceneNode.h"
#include "Geometry.h"
#include "ShaderProgram.h"

class MeshNode :
    public BaseSceneNode
{
private:
    const std::shared_ptr<Geometry> m_pGeometry;

    // TODO: TEMP!!!!!
    ShaderProgram m_program;

public:
    MeshNode(ActorID p_actorID, std::shared_ptr<Geometry> p_pGeometry);
    ~MeshNode(void) { }

    HRESULT VPreRender(Scene* p_pScene);
    HRESULT VRender(Scene* p_pScene);

};

