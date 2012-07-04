#include "StdAfx.h"
#include "BloomEffect.h"


BloomEffect::BloomEffect(std::weak_ptr<RenderTarget> p_pInput, unsigned int p_inputIndex) :
m_pInput(p_pInput), m_inputIndex(p_inputIndex)
{
    m_pBlurHor = 0;
    m_pBlurVer = 0;
}


BloomEffect::~BloomEffect(void)
{
    SAFE_RELEASE(m_pBlurHor);
    SAFE_RELEASE(m_pBlurVer);
}


HRESULT BloomEffect::VOnRestore(void)
{
    SAFE_RELEASE(m_pBlurHor);
    SAFE_RELEASE(m_pBlurVer);

    m_temp.Destroy();
    unsigned int width = LostIsland::g_pGraphics->GetWidth();
    unsigned int height = LostIsland::g_pGraphics->GetHeight();
    DXGI_FORMAT pFormat[2] = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT };
    if(!m_temp.Init2D(width, height, 2, RenderTarget::RTV_SRV_UAV, pFormat))
    {
        return S_FALSE;
    }
    m_pBlurHor = LostIsland::g_pGraphics->CompileComputeShader("./Shader/BloomCS.hlsl", "BlurHorCS");
    m_pBlurVer = LostIsland::g_pGraphics->CompileComputeShader("./Shader/BloomCS.hlsl", "BlurVerCS");
    if(!m_pBlurHor || !m_pBlurVer)
    {
        return S_FALSE;
    }
    return S_OK;
}


HRESULT BloomEffect::VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis)
{

    return S_OK;
}


void BloomEffect::VExecute(void)
{
    std::shared_ptr<RenderTarget> pInput = m_pInput.lock();
    std::shared_ptr<RenderTarget> pBackbuffer = LostIsland::g_pGraphics->GetBackbuffer().lock();
    if(pInput && pBackbuffer)
    {
        unsigned int width = LostIsland::g_pGraphics->GetWidth();
        unsigned int height = LostIsland::g_pGraphics->GetHeight();

        LostIsland::g_pGraphics->ReleaseRenderTarget();
        pInput->BindSingleShaderResource(m_inputIndex, 0, TARGET_CS);
        m_temp.BindAllUnorderedAccess(0);

        // begin dispatch
        LostIsland::g_pGraphics->GetContext()->CSSetShader(m_pBlurHor, 0, 0);
        LostIsland::g_pGraphics->GetContext()->Dispatch((unsigned int)ceil((float)width / 128.0f), height, 1);
        // end dispatch

        LostIsland::g_pGraphics->ReleaseShaderResources(0, 1, TARGET_CS);
        LostIsland::g_pGraphics->ReleaseUnorderedAccess(0, m_temp.GetCount());
        pBackbuffer->BindSingleUnorderedAccess(0, 0);
        m_temp.BindAllShaderResources(0, TARGET_CS);

        // begin dispatch
        LostIsland::g_pGraphics->GetContext()->CSSetShader(m_pBlurVer, 0, 0);
        LostIsland::g_pGraphics->GetContext()->Dispatch(width, (unsigned int)ceil((float)height / 128.0f), 1);
        // end dispatch

        LostIsland::g_pGraphics->ReleaseShaderResources(0, m_temp.GetCount(), TARGET_CS);
        LostIsland::g_pGraphics->ReleaseUnorderedAccess(0, 1);
        //pBackbuffer->BindAllRenderTargets();
    }
}