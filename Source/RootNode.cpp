#include "StdAfx.h"
#include "RootNode.h"


RootNode::RootNode(void)
{
    m_pPostFXCS = 0;
    m_pBlurHor = 0;
}


RootNode::~RootNode(void)
{
    SAFE_RELEASE(m_pPostFXCS);
    SAFE_RELEASE(m_pBlurHor);
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
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = LostIsland::g_pApp->GetConfig()->GetIntAttribute("graphics", "display", "msaa");
    LostIsland::g_pGraphics->GetDevice()->CheckMultisampleQualityLevels(DXGI_FORMAT_R32G32B32A32_FLOAT, sampleDesc.Count, &sampleDesc.Quality);
    if(sampleDesc.Quality == 0)
    {
        LI_WARNING("msaa level not supported, falling back to 1");
        sampleDesc.Quality = 0;
    }
    else
    {
        sampleDesc.Quality -= 1;
    }

    unsigned int width = LostIsland::g_pGraphics->GetWidth();
    unsigned int height = LostIsland::g_pGraphics->GetHeight();
    DXGI_FORMAT pBaseFormats[3] = {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
    };
    if(!m_base.Init2D(width, height, 3, RenderTarget::RTV_DSV_SRV, pBaseFormats, sampleDesc))
    {
        return S_FALSE;
    }

    DXGI_FORMAT enlightenedFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_SAMPLE_DESC enlightenedSampleDesc;
    enlightenedSampleDesc.Count = 1;
    enlightenedSampleDesc.Quality = 0;
    if(!m_enlightened.Init2D(width, height, 1, RenderTarget::RTV_SRV, &enlightenedFormat, enlightenedSampleDesc))
    {
        return S_FALSE;
    } 
    if(!m_temp.Init2D(width, height, 1, RenderTarget::RTV_SRV_UAV, &enlightenedFormat, sampleDesc))
    {
        return S_FALSE;
    }
    

    std::ostringstream str;
    str << sampleDesc.Count;
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

    ID3D10Blob* pShaderBlob = 0;
    ID3D10Blob* pErrorBlob = 0;
    HRESULT hr = D3DX11CompileFromFileA("./Shader/DeferredShading.fx", pDefines, 0, "PostFXCS", "cs_5_0", 0, 0, 0, &pShaderBlob, &pErrorBlob, 0);
    if(pErrorBlob)
    {
        std::string error((char*)pErrorBlob->GetBufferPointer(), (char*)pErrorBlob->GetBufferPointer() + pErrorBlob->GetBufferSize());
        if(FAILED(hr))
        {
            LI_ERROR(error);
            return S_FALSE;
        }
        else
        {
            LI_WARNING(error);
        }
    }
    RETURN_IF_FAILED(hr);
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), 0, &m_pPostFXCS));

    pShaderBlob = 0;
    pErrorBlob = 0;
    hr = D3DX11CompileFromFileA("./Shader/BloomCS.hlsl", pDefines, 0, "BlurHorCS", "cs_5_0", 0, 0, 0, &pShaderBlob, &pErrorBlob, 0);
    if(pErrorBlob)
    {
        std::string error((char*)pErrorBlob->GetBufferPointer(), (char*)pErrorBlob->GetBufferPointer() + pErrorBlob->GetBufferSize());
        if(FAILED(hr))
        {
            LI_ERROR(error);
            return S_FALSE;
        }
        else
        {
            LI_WARNING(error);
        }
    }
    RETURN_IF_FAILED(hr);
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), 0, &m_pBlurHor));

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
    static float pColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_base.BindAllRenderTargets();
    m_base.ClearDepthStencil(1.0f, 0);
    m_base.ClearColor(pColor);
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
    m_dsTest.Bind();
    m_enlightened.BindAllRenderTargets();
    m_base.BindAllShaderResources(0, TARGET_PS);

    m_screenQuad.Draw();

    LostIsland::g_pGraphics->ReleaseShaderResources(0, m_base.GetCount());
    LostIsland::g_pGraphics->ReleaseRenderTarget();

    unsigned int width = LostIsland::g_pGraphics->GetWidth();
    unsigned int height = LostIsland::g_pGraphics->GetHeight();
    
    LostIsland::g_pGraphics->BindBackbufferToUA(0);
    m_enlightened.BindAllShaderResources(0, TARGET_CS);
    LostIsland::g_pGraphics->GetContext()->CSSetShader(m_pBlurHor, 0, 0);
    LostIsland::g_pGraphics->GetContext()->Dispatch(width / 128 + 1, height, 1);
    LostIsland::g_pGraphics->ReleaseShaderResources(0, m_enlightened.GetCount(), TARGET_CS);
    LostIsland::g_pGraphics->ReleaseUnorderedAccess(0, 1);
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