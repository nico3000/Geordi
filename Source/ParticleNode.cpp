#include "StdAfx.h"
#include "ParticleNode.h"
#include "ParticleComponent.h"
#include "Scene.h"

ParticleNode::ParticleNode(ActorID p_actorID, std::weak_ptr<ParticleCloud> p_pCloud) :
BaseSceneNode(p_actorID), m_pCloud(p_pCloud)
{
    StrongActorPtr pActor = m_pActor.lock();
    if(pActor)
    {
        m_program.Load("./Shader/NicotopiaTest.fx", "ParticleVS", 0, "SimplePS");
        //m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements);
    }
}


ParticleNode::~ParticleNode(void)
{
}


HRESULT ParticleNode::VPreRender(Scene* p_pScene)
{
    HRESULT hr = BaseSceneNode::VPreRender(p_pScene);
    if(SUCCEEDED(hr))
    {
        return p_pScene->UpdateModelMatrixBuffer() ? S_OK : S_FALSE;
    }
    return hr;
}


HRESULT ParticleNode::VRender(Scene* p_pScene)
{
    std::shared_ptr<ParticleCloud> pCloud = m_pCloud.lock();
    if(pCloud)
    {
        m_program.Bind();

        ID3D11ShaderResourceView* ppSRVs[1] = { pCloud->m_pParticlesSRV };
        LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(0, 1, ppSRVs);

        static ID3D11Buffer* ppNull[1] = { 0 };
        static unsigned int null = 0;
        LostIsland::g_pGraphics->GetContext()->IASetVertexBuffers(0, 1, ppNull, &null, &null);
        LostIsland::g_pGraphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        LostIsland::g_pGraphics->GetContext()->Draw(pCloud->m_count, 0);

        ppSRVs[0] = 0;
        LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(0, 1, ppSRVs);

        return S_OK;
    }
    else
    {
        LI_WARNING("ParticleNode without ParticleCloud");
        return S_FALSE;
    }
}