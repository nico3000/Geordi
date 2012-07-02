#include "StdAfx.h"
#include "RenderTarget.h"


RenderTarget::RenderTarget(void)
{
    m_ppRTV = 0;
    m_ppSRV = 0;
    m_ppUAV = 0;
    m_pDSV = 0;
    m_count = 0;
}


RenderTarget::~RenderTarget(void)
{
    this->Destroy();
}


void RenderTarget::Destroy(void)
{
    SAFE_RELEASE(m_pDSV);
    for(unsigned int i=0; i < m_count; ++i)
    {
        SAFE_RELEASE(m_ppRTV[i]);
        SAFE_RELEASE(m_ppSRV[i]);
        SAFE_RELEASE(m_ppUAV[i]);
    }
    SAFE_DELETE(m_ppRTV);
    SAFE_DELETE(m_ppSRV);
    SAFE_DELETE(m_ppUAV);
    m_count = 0;
}


bool RenderTarget::Init2D(unsigned int p_width, unsigned int p_height, unsigned int p_count, View p_viewsToCreate, const DXGI_FORMAT* p_pFormats,  const DXGI_SAMPLE_DESC& p_sampleDesc)
{
    if(p_sampleDesc.Count > 1 && (p_viewsToCreate & UAV))
    {
        LI_ERROR("multisampled unordered access views are not supported");
    }

    this->Destroy();
    m_count = p_count;
    m_ppRTV = new ID3D11RenderTargetView*[m_count];
    m_ppSRV = new ID3D11ShaderResourceView*[m_count];
    m_ppUAV = new ID3D11UnorderedAccessView*[m_count];
    for(unsigned int i=0; i < m_count; ++i)
    {
        m_ppRTV[i] = 0;
        m_ppSRV[i] = 0;
        m_ppUAV[i] = 0;
    }
    
    unsigned int bindFlags = 0;
    if(p_viewsToCreate & RTV)
    {
        bindFlags |= D3D11_BIND_RENDER_TARGET;
    }
    if(p_viewsToCreate & SRV)
    {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if(p_viewsToCreate & UAV)
    {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }
    for(unsigned int i=0; i < m_count; ++i)
    {
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.ArraySize = 1;
        texDesc.BindFlags = bindFlags;
        texDesc.CPUAccessFlags = 0;
        texDesc.Format = p_pFormats[i];
        texDesc.Width = p_width;
        texDesc.Height = p_height;
        texDesc.MipLevels = 1;
        texDesc.MiscFlags = 0;
        texDesc.SampleDesc = p_sampleDesc;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        ID3D11Texture2D* pTex = 0;
        RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateTexture2D(&texDesc, 0, &pTex));

        if(p_viewsToCreate & RTV)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = p_pFormats[i];
            rtvDesc.ViewDimension = p_sampleDesc.Count == 1 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
            rtvDesc.Texture2D.MipSlice = 0;
            RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateRenderTargetView(pTex, &rtvDesc, &m_ppRTV[i]));
        }

        if(p_viewsToCreate & SRV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = p_pFormats[i];
            srvDesc.ViewDimension = p_sampleDesc.Count == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateShaderResourceView(pTex, &srvDesc, &m_ppSRV[i]));
        }

        if(p_viewsToCreate & UAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            uavDesc.Format = p_pFormats[i];
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = 0;
            RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateUnorderedAccessView(pTex, &uavDesc, &m_ppUAV[i]));
        }

        SAFE_RELEASE(pTex);
    }

    m_pDSV = 0;
    if(p_viewsToCreate & DSV)
    {
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.ArraySize = 1;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        texDesc.CPUAccessFlags = 0;
        texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        texDesc.Width = p_width;
        texDesc.Height = p_height;
        texDesc.MipLevels = 1;
        texDesc.MiscFlags = 0;
        texDesc.SampleDesc = p_sampleDesc;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        ID3D11Texture2D* pTex = 0;
        RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateTexture2D(&texDesc, 0, &pTex));

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags = 0;
        dsvDesc.Format = texDesc.Format;
        dsvDesc.ViewDimension = p_sampleDesc.Count == 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
        dsvDesc.Texture2D.MipSlice = 0;
        RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateDepthStencilView(pTex, &dsvDesc, &m_pDSV));

        SAFE_RELEASE(pTex);
    }

    return true;
}


void RenderTarget::BindAllRenderTargets(void)
{
    LostIsland::g_pGraphics->GetContext()->OMSetRenderTargets(m_count, m_ppRTV, m_pDSV);
}


void RenderTarget::BindAllShaderResources(unsigned int p_startSlot, ShaderTarget p_target)
{
    if(p_target & TARGET_VS)
    {
        LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(p_startSlot, m_count, m_ppSRV);
    }
    if(p_target & TARGET_GS)
    {
        LostIsland::g_pGraphics->GetContext()->GSSetShaderResources(p_startSlot, m_count, m_ppSRV);
    }
    if(p_target & TARGET_PS)
    {
        LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(p_startSlot, m_count, m_ppSRV);
    }
    if(p_target & TARGET_CS)
    {
        LostIsland::g_pGraphics->GetContext()->CSSetShaderResources(p_startSlot, m_count, m_ppSRV);
    }
}


void RenderTarget::ClearColor(const float p_pColor[4])
{
    for(unsigned int i=0; i < m_count; ++i)
    {
        LostIsland::g_pGraphics->GetContext()->ClearRenderTargetView(m_ppRTV[i], p_pColor);
    }
}


void RenderTarget::ClearDepthStencil(float p_depth, unsigned int p_stencil)
{
    LostIsland::g_pGraphics->GetContext()->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, p_depth, p_stencil);
}