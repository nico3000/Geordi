#include "StdAfx.h"
#include "RootNode.h"


RootNode::RootNode(void)
{
    m_pDSVTex = 0;
    m_pDSV = 0;
    for(int i=0; i < DS_NUM_TARGETS; ++i)
    {
        m_ppSRV[i] = 0;
        m_ppRTV[i] = 0;
        m_ppTex[i] = 0;
    }
}


RootNode::~RootNode(void)
{
    SAFE_RELEASE(m_pDSV);
    SAFE_RELEASE(m_pDSVTex);
    for(int i=0; i < DS_NUM_TARGETS; ++i)
    {
        SAFE_RELEASE(m_ppSRV[i]);
        SAFE_RELEASE(m_ppRTV[i]);
        SAFE_RELEASE(m_ppTex[i]);
    }
}


HRESULT RootNode::VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis)
{
    for(auto iter=m_staticNodes.begin(); iter != m_staticNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VOnUpdate(p_pScene, p_deltaMillis));
    }

    for(auto iter=m_dynamicNodes.begin(); iter != m_dynamicNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VOnUpdate(p_pScene, p_deltaMillis));
    }
    return S_OK;
}


HRESULT RootNode::VOnRestore(void)
{
    unsigned int sampleCount = LostIsland::g_pApp->GetConfig()->GetIntAttribute("graphics", "display", "msaa");
    unsigned int qualityLevel;
    LostIsland::g_pGraphics->GetDevice()->CheckMultisampleQualityLevels(DXGI_FORMAT_R32G32B32A32_FLOAT, sampleCount, &qualityLevel);
    if(qualityLevel == 0)
    {
        LI_WARNING("msaa level not supported, falling back to 1");
        sampleCount = 1;
    }
    else
    {
        qualityLevel -= 1;
        //numQualityLevels = 0;
    }

    D3D11_TEXTURE2D_DESC texDesc[DS_NUM_TARGETS];
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc[DS_NUM_TARGETS];
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc[DS_NUM_TARGETS];
    texDesc[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    for(int i=0; i < DS_NUM_TARGETS; ++i)
    {
        texDesc[i].ArraySize = 1;
        texDesc[i].BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc[i].CPUAccessFlags = 0;
        texDesc[i].Width = LostIsland::g_pGraphics->GetWidth();
        texDesc[i].Height = LostIsland::g_pGraphics->GetHeight();
        texDesc[i].MipLevels = 1;
        texDesc[i].MiscFlags = 0;
        texDesc[i].SampleDesc.Count = sampleCount;
        texDesc[i].SampleDesc.Quality = qualityLevel;
        texDesc[i].Usage = D3D11_USAGE_DEFAULT;

        rtvDesc[i].ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        rtvDesc[i].Format = texDesc[i].Format;

        srvDesc[i].ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        srvDesc[i].Format = texDesc[i].Format;

        SAFE_RELEASE(m_ppSRV[i]);
        SAFE_RELEASE(m_ppRTV[i]);
        SAFE_RELEASE(m_ppTex[i]);
        RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateTexture2D(&texDesc[i], 0, &m_ppTex[i]));
        RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateRenderTargetView(m_ppTex[i], &rtvDesc[i], &m_ppRTV[i]));
        RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateShaderResourceView(m_ppTex[i], &srvDesc[i], &m_ppSRV[i]));
    }

    D3D11_TEXTURE2D_DESC dsvTexDesc;
    dsvTexDesc.ArraySize = 1;
    dsvTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dsvTexDesc.CPUAccessFlags = 0;
    dsvTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvTexDesc.Width = LostIsland::g_pGraphics->GetWidth();
    dsvTexDesc.Height = LostIsland::g_pGraphics->GetHeight();
    dsvTexDesc.MipLevels = 1;
    dsvTexDesc.MiscFlags = 0;
    dsvTexDesc.SampleDesc.Count = sampleCount;
    dsvTexDesc.SampleDesc.Quality = qualityLevel;
    dsvTexDesc.Usage = D3D11_USAGE_DEFAULT;
    SAFE_RELEASE(m_pDSVTex);
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateTexture2D(&dsvTexDesc, 0, &m_pDSVTex));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0;
    dsvDesc.Format = dsvTexDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    SAFE_RELEASE(m_pDSV);
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateDepthStencilView(m_pDSVTex, &dsvDesc, &m_pDSV));

    VertexBuffer::ScreenQuadVertex pVertices[] = { { XMFLOAT2(-1.0f, -1.0f) },
                                                   { XMFLOAT2(+1.0f, -1.0f) },
                                                   { XMFLOAT2(-1.0f, +1.0f) },
                                                   { XMFLOAT2(+1.0f, +1.0f) } };
    Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
    if(!pVertexBuffer->Build(pVertices, ARRAYSIZE(pVertices), sizeof(VertexBuffer::ScreenQuadVertex)))
    {
        return S_FALSE;
    }
    m_screenQuad.SetVertices(pVertexBuffer);

    unsigned int pIndices[] = { 0, 1, 2, 3 };
    Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
    if(!pIndexBuffer->Build(pIndices, ARRAYSIZE(pIndices), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP))
    {
        return S_FALSE;
    }
    m_screenQuad.SetIndices(pIndexBuffer);
    

    std::ostringstream str;
    str << sampleCount;
    std::string val(str.str());
    D3D10_SHADER_MACRO pDefines[2] = 
    {
        { "SAMPLE_COUNT", val.c_str() },
        { 0, 0 },
    };
    if(!m_dsTest.Load("./Shader/DeferredShading.fx", "ScreenQuadVS", 0, "TexOutPS", pDefines))
    {
        return S_FALSE;
    }
    if(!m_dsTest.CreateInputLayout(VertexBuffer::sm_pScreenQuadVertexElementDesc, VertexBuffer::sm_screenQuadVertexNumElements))
    {
        return S_FALSE;
    }

    for(auto iter=m_staticNodes.begin(); iter != m_staticNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VOnRestore());
    }

    for(auto iter=m_dynamicNodes.begin(); iter != m_dynamicNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VOnRestore());
    }
    return S_OK;
}


HRESULT RootNode::VOnLostDevice(void)
{
    for(auto iter=m_staticNodes.begin(); iter != m_staticNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VOnLostDevice());
    }

    for(auto iter=m_dynamicNodes.begin(); iter != m_dynamicNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VOnLostDevice());
    }
    return S_OK;
}


HRESULT RootNode::VPreRender(Scene* p_pScene)
{
    LostIsland::g_pGraphics->GetContext()->OMSetRenderTargets(DS_NUM_TARGETS, m_ppRTV, m_pDSV);
    LostIsland::g_pGraphics->GetContext()->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0xff);
    static float pColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    LostIsland::g_pGraphics->GetContext()->ClearRenderTargetView(m_ppRTV[0], pColor);
    LostIsland::g_pGraphics->GetContext()->ClearRenderTargetView(m_ppRTV[1], pColor);
    LostIsland::g_pGraphics->GetContext()->ClearRenderTargetView(m_ppRTV[2], pColor);
    // TODO activate deferred shading
    return S_OK;
}


HRESULT RootNode::VRender(Scene* p_pScene)
{
    for(auto iter=m_staticNodes.begin(); iter != m_staticNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VPreRender(p_pScene));
        RETURN_IF_FAILED((*iter)->VRender(p_pScene));
        RETURN_IF_FAILED((*iter)->VRenderChildren(p_pScene));
        RETURN_IF_FAILED((*iter)->VPostRender(p_pScene));
    }

    for(auto iter=m_dynamicNodes.begin(); iter != m_dynamicNodes.end(); ++iter)
    {
        RETURN_IF_FAILED((*iter)->VPreRender(p_pScene));
        RETURN_IF_FAILED((*iter)->VRender(p_pScene));
        RETURN_IF_FAILED((*iter)->VRenderChildren(p_pScene));
        RETURN_IF_FAILED((*iter)->VPostRender(p_pScene));
    }
    return S_OK;
}


HRESULT RootNode::VRenderChildren(Scene* p_pScene)
{
    return S_OK;
}


HRESULT RootNode::VPostRender(Scene* p_pScene)
{
    // todo postfx!
    LostIsland::g_pGraphics->ActivateBackbuffer();
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(1, DS_NUM_TARGETS, m_ppSRV);
    m_dsTest.Bind();
    m_screenQuad.Draw();
    static ID3D11ShaderResourceView* ppNull[DS_NUM_TARGETS] = { 0, 0, 0 };
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(1, DS_NUM_TARGETS, ppNull);
    return S_OK;
}


bool RootNode::VAddChild(std::shared_ptr<ISceneNode> p_pChild)
{
    switch(p_pChild->VGetNodeType())
    {
    case UNKNOWN: return false;
    case DYNAMIC: m_dynamicNodes.push_back(p_pChild); break;
    case STATIC: m_staticNodes.push_back(p_pChild); break;
    }
    return true;
}


bool RootNode::VRemoveChild(ActorID p_actorID)
{
    // todo!
    return false;    
}