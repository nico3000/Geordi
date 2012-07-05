#include "StdAfx.h"
#include "BloomEffect.h"
#include "ScreenQuad.h"


BloomEffectCS::BloomEffectCS(std::weak_ptr<RenderTarget> p_pInput, unsigned int p_inputIndex, std::weak_ptr<RenderTarget> p_pOutput) :
m_pInput(p_pInput), m_inputIndex(p_inputIndex), m_pOutput(p_pOutput)
{
    m_pBlurHor = 0;
    m_pBlurVer = 0;
}


BloomEffectCS::~BloomEffectCS(void)
{
    SAFE_RELEASE(m_pBlurHor);
    SAFE_RELEASE(m_pBlurVer);
}


HRESULT BloomEffectCS::VOnRestore(void)
{
    SAFE_RELEASE(m_pBlurHor);
    SAFE_RELEASE(m_pBlurVer);

    m_temp.Destroy();
    unsigned int width = LostIsland::g_pGraphics->GetWidth();
    unsigned int height = LostIsland::g_pGraphics->GetHeight();
    DXGI_FORMAT pFormat[1] = { DXGI_FORMAT_R16G16B16A16_FLOAT };
    if(!m_temp.Init2D(width, height, 1, RenderTarget::RTV_SRV_UAV, pFormat))
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


HRESULT BloomEffectCS::VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis)
{

    return S_OK;
}


void BloomEffectCS::VExecute(void)
{
    std::shared_ptr<RenderTarget> pInput = m_pInput.lock();
    std::shared_ptr<RenderTarget> pOutput = m_pOutput.lock();
    if(pInput && pOutput)
    {
        unsigned int width = LostIsland::g_pGraphics->GetWidth();
        unsigned int height = LostIsland::g_pGraphics->GetHeight();

        LostIsland::g_pGraphics->ReleaseRenderTarget();
        pInput->BindSingleShaderResource(m_inputIndex, 0, TARGET_CS);
        m_temp.BindSingleUnorderedAccess(0, 0);

        // begin dispatch
        LostIsland::g_pGraphics->GetContext()->CSSetShader(m_pBlurHor, 0, 0);
        LostIsland::g_pGraphics->GetContext()->Dispatch((unsigned int)ceil((float)width / 128.0f), height, 1);
        // end dispatch

        LostIsland::g_pGraphics->ReleaseUnorderedAccess(0, 1);
        LostIsland::g_pGraphics->ReleaseShaderResources(0, 1, TARGET_CS);
        pOutput->BindSingleUnorderedAccess(0, 0);
        m_temp.BindSingleShaderResource(0, 0, TARGET_CS);
        pInput->BindSingleShaderResource(m_inputIndex, 1, TARGET_CS);

        // begin dispatch
        LostIsland::g_pGraphics->GetContext()->CSSetShader(m_pBlurVer, 0, 0);
        LostIsland::g_pGraphics->GetContext()->Dispatch(width, (unsigned int)ceil((float)height / 128.0f), 1);
        // end dispatch

        LostIsland::g_pGraphics->ReleaseShaderResources(0, 2, TARGET_CS);
        LostIsland::g_pGraphics->ReleaseUnorderedAccess(0, 1);
        //pBackbuffer->BindAllRenderTargets();
    }
}


BloomEffectPS::BloomEffectPS(std::weak_ptr<RenderTarget> p_pInput, unsigned int p_inputIndex, std::weak_ptr<RenderTarget> p_pOutput) :
m_pInput(p_pInput), m_inputIndex(p_inputIndex), m_pOutput(p_pOutput)
{

}


BloomEffectPS::~BloomEffectPS(void)
{

}


HRESULT BloomEffectPS::VOnRestore(void)
{
    if(!m_blurHor.Load("./Shader/BloomPS.hlsl", "ScreenQuadVS", 0, "BlurHorPS"))
    {
        return S_FALSE;
    }
    if(!ScreenQuad::CreateInputLayoutForShader(m_blurHor))
    {
        return S_FALSE;
    }
    if(!m_blurVer.Load("./Shader/BloomPS.hlsl", "ScreenQuadVS", 0, "BlurVerPS"))
    {
        return S_FALSE;
    }
    if(!ScreenQuad::CreateInputLayoutForShader(m_blurVer))
    {
        return S_FALSE;
    }

    m_temp.Destroy();
    unsigned int width = LostIsland::g_pGraphics->GetWidth();
    unsigned int height = LostIsland::g_pGraphics->GetHeight();
    DXGI_FORMAT pFormat[1] = { DXGI_FORMAT_R16G16B16A16_FLOAT };
    if(!m_temp.Init2D(width, height, 1, RenderTarget::RTV_SRV, pFormat))
    {
        return S_FALSE;
    }
    return S_OK;
}


HRESULT BloomEffectPS::VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis)
{
    return S_OK;
}


void BloomEffectPS::VExecute(void)
{
    std::shared_ptr<RenderTarget> pInput = m_pInput.lock();
    std::shared_ptr<RenderTarget> pOutput = m_pOutput.lock();
    std::shared_ptr<ScreenQuad> pScreenQuad = ScreenQuad::GetScreenQuad().lock();
    if(pInput && pOutput && pScreenQuad)
    {
        pInput->BindSingleShaderResource(m_inputIndex, 0, TARGET_PS);
        
        m_blurHor.Bind();
        m_temp.BindAllRenderTargets();

        pScreenQuad->Draw();

        LostIsland::g_pGraphics->ReleaseRenderTarget();

        m_blurVer.Bind();
        m_temp.BindSingleShaderResource(0, 1, TARGET_PS);
        pOutput->BindAllRenderTargets();

        pScreenQuad->Draw();

        LostIsland::g_pGraphics->ReleaseRenderTarget();

        LostIsland::g_pGraphics->ReleaseShaderResources(0, 2, TARGET_PS);
    }
}