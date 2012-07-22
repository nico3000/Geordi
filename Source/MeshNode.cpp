#include "StdAfx.h"
#include "MeshNode.h"
#include "VertexBuffer.h"
#include "Scene.h"


MeshNode::MeshNode(ActorID p_actorID, std::shared_ptr<Geometry> p_pGeometry) : BaseSceneNode(p_actorID), m_pGeometry(p_pGeometry)
{
    StrongActorPtr pActor = m_pActor.lock();
    if(pActor)
    {
        m_program.Load(L"./Shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS");
        m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements);
    }
}


HRESULT MeshNode::VPreRender(Scene* p_pScene)
{
    HRESULT hr = BaseSceneNode::VPreRender(p_pScene);
    if(SUCCEEDED(hr))
    {
        return p_pScene->UpdateModelMatrixBuffer() ? S_OK : S_FALSE;
    }
    return hr;
}


HRESULT MeshNode::VRender(Scene* p_pScene)
{
    m_program.Bind();
    m_pGeometry->Draw();
    return S_OK;
}