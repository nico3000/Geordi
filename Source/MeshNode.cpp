#include "StdAfx.h"
#include "MeshNode.h"
#include "VertexBuffer.h"
#include "Scene.h"


MeshNode::MeshNode(ActorID p_actorID, std::shared_ptr<Geometry> p_pGeometry) : BaseSceneNode(p_actorID), m_pGeometry(p_pGeometry)
{
    m_pActor = LostIsland::g_pApp->GetGameLogic()->VGetActor(p_actorID);

    StrongActorPtr pActor = m_pActor.lock();
    if(pActor)
    {
        m_program.Load("./Shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS");
        m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements);

        m_modelBuffer.Build((void*)&m_modelData, sizeof(LocalPose::ModelMatrixData));
    }
}


HRESULT MeshNode::VPreRender(Scene* p_pScene)
{
    StrongActorPtr pActor = m_pActor.lock();
    if(pActor)
    {
        const LocalPose::ModelMatrixData& data = pActor->GetPose().GetModelMatrixBuffer(true);
        p_pScene->GetModelStack().PushMatrix(data.model);
        p_pScene->GetModelInvStack().PushMatrix(data.modelInv);
        m_modelData.model = p_pScene->GetModelStack().Top();
        m_modelData.modelInv = p_pScene->GetModelInvStack().Top();
    }
    return S_OK;
}


HRESULT MeshNode::VRender(Scene* p_pScene)
{
    m_modelBuffer.Update();
    m_modelBuffer.Bind(1, ConstantBuffer::TARGET_ALL);
    m_program.Bind();
    m_pGeometry->Draw();
    return S_OK;
}


HRESULT MeshNode::VPostRender(Scene* p_pScene)
{
    p_pScene->GetModelStack().PopMatrix();
    p_pScene->GetModelInvStack().PopMatrix();
    return S_OK;
}