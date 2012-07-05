#include "StdAfx.h"
#include "RootNode.h"

#include "ScreenQuad.h"
#include "BloomEffect.h"

RootNode::RootNode(void) :
m_pBase(new RenderTarget), m_pEnlightened(new RenderTarget)
{
    m_effects.push_back(std::shared_ptr<IPostEffect>(new BloomEffectPS(m_pEnlightened, 0, LostIsland::g_pGraphics->GetBackbuffer())));
}


RootNode::~RootNode(void)
{
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
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
    };
    DXGI_FORMAT enlightenedFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
    if(!m_pBase->Init2D(width, height, 3, RenderTarget::RTV_DSV_SRV, pBaseFormats, sampleDesc) || !m_pEnlightened->Init2D(width, height, 1, RenderTarget::RTV_SRV_UAV, &enlightenedFormat))
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
    if(!m_dsTest.Load("./Shader/MergeLightingMS.hlsl", "ScreenQuadVS", 0, sampleDesc.Count == 1 ? "TexOutPS" : "TexOutMSPS", pDefines))
    {
        return S_FALSE;
    }
    if(!ScreenQuad::CreateInputLayoutForShader(m_dsTest))
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

    for(auto iter=m_effects.begin(); iter != m_effects.end(); ++iter)
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
    m_pBase->BindAllRenderTargets();
    m_pBase->ClearDepthStencil(1.0f, 0);
    m_pBase->ClearColor(pColor);
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
    m_pEnlightened->BindAllRenderTargets();
    //LostIsland::g_pGraphics->GetBackbuffer().lock()->BindAllRenderTargets();
    m_pBase->BindAllShaderResources(0, TARGET_PS);

    std::shared_ptr<ScreenQuad> pScreenQuad = ScreenQuad::GetScreenQuad().lock();
    if(!pScreenQuad)
    {
        return S_FALSE;
    }
    pScreenQuad->Draw();

    LostIsland::g_pGraphics->ReleaseShaderResources(0, m_pBase->GetCount(), TARGET_PS);
    LostIsland::g_pGraphics->ReleaseRenderTarget();

    for(auto iter=m_effects.begin(); iter != m_effects.end(); ++iter)
    {
        (*iter)->VExecute();
    }

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