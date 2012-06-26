#pragma once
#include "BaseSceneNode.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "ConstantBuffer.h"

class MeshNode :
    public BaseSceneNode
{
private:
    const std::shared_ptr<Geometry> m_pGeometry;
    LocalPose::ModelMatrixData m_modelData;
    WeakActorPtr m_pActor;

    // TODO: TEMP!!!!!
    ShaderProgram m_program;
    ConstantBuffer m_modelBuffer;

public:
    MeshNode(ActorID p_actorID, std::shared_ptr<Geometry> p_pGeometry);
    ~MeshNode(void) { }

    HRESULT VOnRestore(void) { return S_OK; }
    HRESULT VOnLostDevice(void) { return S_OK; }

    HRESULT VPreRender(Scene* p_pScene);
    HRESULT VRender(Scene* p_pScene);
    HRESULT VPostRender(Scene* p_pScene);

};

