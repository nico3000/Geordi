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
        m_program.Load("./Shader/NicotopiaTest.fx", "ParticleVS", "ParticleGS", "SimplePS");
        //m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements);
        m_pTexture = 0;
        m_pBlendState = 0;
        m_pDepthStencilState = 0;
    }
}


ParticleNode::~ParticleNode(void)
{
    SAFE_RELEASE(m_pTexture);
    SAFE_RELEASE(m_pBlendState);
    SAFE_RELEASE(m_pDepthStencilState);
}


HRESULT ParticleNode::VOnRestore(void)
{
    SAFE_RELEASE(m_pTexture);
    SAFE_RELEASE(m_pBlendState);
    SAFE_RELEASE(m_pDepthStencilState);

    ID3D11Resource* pResource;
    RETURN_IF_FAILED(D3DX11CreateTextureFromFileA(LostIsland::g_pGraphics->GetDevice(), "./Texture/sprite_alpha.png", 0, 0, &pResource, 0));
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateShaderResourceView(pResource, 0, &m_pTexture));
    SAFE_RELEASE(pResource);

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateBlendState(&blendDesc, &m_pBlendState));

    D3D11_DEPTH_STENCIL_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsvDesc.DepthEnable = true;
    dsvDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsvDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateDepthStencilState(&dsvDesc, &m_pDepthStencilState));
    
    return true;
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
        ID3D11DepthStencilState* pCurrentDepthStencilState;
        unsigned int currentStencilRef;
        LostIsland::g_pGraphics->GetContext()->OMGetDepthStencilState(&pCurrentDepthStencilState, &currentStencilRef);
        LostIsland::g_pGraphics->GetContext()->OMSetDepthStencilState(m_pDepthStencilState, 0);
        FLOAT pFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        LostIsland::g_pGraphics->GetContext()->OMSetBlendState(m_pBlendState, pFactor, 0xffffffff);

        m_program.Bind();

        ID3D11ShaderResourceView* ppSRVs[1] = { pCloud->m_pParticlesSRV };
        LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(0, 1, ppSRVs);

        static ID3D11Buffer* ppNull[1] = { 0 };
        static unsigned int null = 0;
        LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(1, 1, &m_pTexture);
        LostIsland::g_pGraphics->GetContext()->IASetVertexBuffers(0, 1, ppNull, &null, &null);
        LostIsland::g_pGraphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        LostIsland::g_pGraphics->GetContext()->Draw(pCloud->m_count, 0);

        ppSRVs[0] = 0;
        LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(0, 1, ppSRVs);

        LostIsland::g_pGraphics->GetContext()->OMSetDepthStencilState(pCurrentDepthStencilState, currentStencilRef);
        LostIsland::g_pGraphics->GetContext()->OMSetBlendState(0, pFactor, 0xffffffff);

        SAFE_RELEASE(pCurrentDepthStencilState);

        return S_OK;
    }
    else
    {
        LI_WARNING("ParticleNode without ParticleCloud");
        return S_FALSE;
    }
}