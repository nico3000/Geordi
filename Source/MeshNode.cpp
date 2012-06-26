#include "StdAfx.h"
#include "MeshNode.h"
#include "VertexBuffer.h"
#include "Scene.h"


MeshNode::MeshNode(ActorID p_actorID, const XMFLOAT4X4& p_model, std::shared_ptr<Geometry> p_pGeometry) : BaseSceneNode(p_actorID, p_model), m_pGeometry(p_pGeometry)
{
    m_program.Load("./Shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS");
    m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements);
    m_model.Build((void*)&m_properties.m_model, 2 * sizeof(XMMATRIX));
}


HRESULT MeshNode::VPreRender(Scene* p_pScene)
{
    p_pScene->GetMatrixStack().PushMatrix(m_properties.GetModel());
    return S_OK;
}


HRESULT MeshNode::VRender(Scene* p_pScene)
{
    m_model.
    m_program.Bind();
    m_pGeometry->Draw();
    return S_OK;
}


HRESULT MeshNode::VPostRender(Scene* p_pScene)
{
    p_pScene->GetMatrixStack().PopMatrix();
    return S_OK;
}