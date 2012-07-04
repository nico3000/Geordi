#include "StdAfx.h"
#include "RenderTarget.h"


RenderTarget::RenderTarget(void)
{
    m_ppRTV = 0;
    m_ppSRV = 0;
    m_ppUAV = 0;
    m_pDSV = 0;
    m_count = 0;
    m_pFormats = 0;
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
    SAFE_DELETE(m_pFormats);
    m_count = 0;
}


bool RenderTarget::Init2DMS(unsigned int p_width, unsigned int p_height, unsigned int p_count, View p_viewsToCreate, const DXGI_FORMAT* p_pFormats,  const DXGI_SAMPLE_DESC& p_sampleDesc)
{
    if(p_viewsToCreate & UAV)
    {
        LI_ERROR("multisampled unordered access views are not supported");
        return false;
    }

    this->Destroy();
    this->Reset(p_count, p_width, p_height, p_pFormats);
    m_sampleDesc = p_sampleDesc;
    
    unsigned int bindFlags = 0;
    if(p_viewsToCreate & RTV)
    {
        bindFlags |= D3D11_BIND_RENDER_TARGET;
    }
    if(p_viewsToCreate & SRV)
    {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    for(unsigned int i=0; i < m_count; ++i)
    {
        ID3D11Texture2D* pTex = this->CreateTexture2D(p_pFormats[i], bindFlags);
        if(!pTex)
        {
            return false;
        }
        if((p_viewsToCreate & RTV) && !this->CreateRTV2D(pTex, i))
        {
            return false;
        }
        if((p_viewsToCreate & SRV) && !this->CreateSRV2D(pTex, i))
        {
            return false;
        }
        SAFE_RELEASE(pTex);
    }
    if((p_viewsToCreate & DSV) && !this->CreateDSV2D())
    {
        return false;
    }
    return true;
}


bool RenderTarget::Init2D(unsigned int p_width, unsigned int p_height, unsigned int p_count, View p_viewsToCreate, const DXGI_FORMAT* p_pFormats)
{
    this->Destroy();
    this->Reset(p_count, p_width, p_height, p_pFormats);

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
        ID3D11Texture2D* pTex = this->CreateTexture2D(p_pFormats[i], bindFlags);
        if(!pTex)
        {
            return false;
        }
        if((p_viewsToCreate & RTV) && !this->CreateRTV2D(pTex, i))
        {
            return false;
        }
        if((p_viewsToCreate & SRV) && !this->CreateSRV2D(pTex, i))
        {
            return false;
        }
        if((p_viewsToCreate & UAV) && !this->CreateUAV2D(pTex, i))
        {
            return false;
        }
        SAFE_RELEASE(pTex);
    }
    if((p_viewsToCreate & DSV) && !this->CreateDSV2D())
    {
        return false;
    }
    return true;
}


bool RenderTarget::Init2D(ID3D11Texture2D* p_pResource, View p_viewsToCreate)
{
    D3D11_TEXTURE2D_DESC texDesc;
    p_pResource->GetDesc(&texDesc);

    this->Destroy();
    this->Reset(1, texDesc.Width, texDesc.Height, &texDesc.Format);

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
    if((p_viewsToCreate & RTV) && !this->CreateRTV2D(p_pResource, 0))
    {
        return false;
    }
    if((p_viewsToCreate & SRV) && !this->CreateSRV2D(p_pResource, 0))
    {
        return false;
    }
    if((p_viewsToCreate & UAV) && !this->CreateUAV2D(p_pResource, 0))
    {
        return false;
    }
    if((p_viewsToCreate & DSV) && !this->CreateDSV2D())
    {
        return false;
    }
    return true;
}


ID3D11Texture2D* RenderTarget::CreateTexture2D(DXGI_FORMAT p_format, unsigned int p_bindFlags) const
{
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.ArraySize = 1;
    texDesc.BindFlags = p_bindFlags;
    texDesc.CPUAccessFlags = 0;
    texDesc.Format = p_format;
    texDesc.Width = m_width;
    texDesc.Height = m_height;
    texDesc.MipLevels = 1;
    texDesc.MiscFlags = 0;
    texDesc.SampleDesc = m_sampleDesc;
    texDesc.Usage = D3D11_USAGE_DEFAULT;

    ID3D11Texture2D* pTex = 0;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateTexture2D(&texDesc, 0, &pTex));
    return pTex;
}


bool RenderTarget::CreateRTV2D(ID3D11Texture2D* p_pResource, unsigned int p_index)
{
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = m_pFormats[p_index];
    rtvDesc.ViewDimension = m_sampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateRenderTargetView(p_pResource, &rtvDesc, &m_ppRTV[p_index]));
    return true;
}


bool RenderTarget::CreateSRV2D(ID3D11Texture2D* p_pResource, unsigned int p_index)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = m_pFormats[p_index];
    srvDesc.ViewDimension = m_sampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateShaderResourceView(p_pResource, &srvDesc, &m_ppSRV[p_index]));
    return true;
}


bool RenderTarget::CreateUAV2D(ID3D11Texture2D* p_pResource, unsigned int p_index)
{
    if(m_sampleDesc.Count > 1)
    {
        LI_ERROR("Unordered access views for multisampled textures are not supported");
        return false;
    }
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = m_pFormats[p_index];
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateUnorderedAccessView(p_pResource, &uavDesc, &m_ppUAV[p_index]));
    return true;
}


bool RenderTarget::CreateDSV2D(void)
{
    ID3D11Texture2D* pTex = this->CreateTexture2D(DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL);
    if(!pTex)
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = m_sampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    RETURN_IF_FAILED(LostIsland::g_pGraphics->GetDevice()->CreateDepthStencilView(pTex, &dsvDesc, &m_pDSV));
    SAFE_RELEASE(pTex);
    return true;
}


void RenderTarget::Reset(unsigned int p_count, unsigned int p_width, unsigned int p_height, const DXGI_FORMAT* p_pFormats)
{
    m_count = p_count;
    m_width = p_width;
    m_height = p_height;
    m_ppRTV = new ID3D11RenderTargetView*[m_count];
    m_ppSRV = new ID3D11ShaderResourceView*[m_count];
    m_ppUAV = new ID3D11UnorderedAccessView*[m_count];
    m_pFormats = new DXGI_FORMAT[m_count];
    for(unsigned int i=0; i < m_count; ++i)
    {
        m_ppRTV[i] = 0;
        m_ppSRV[i] = 0;
        m_ppUAV[i] = 0;
        m_pFormats[i] = p_pFormats[i];
    }
    m_pDSV = 0;
    m_sampleDesc.Count = 1;
    m_sampleDesc.Quality = 0;
}


void RenderTarget::BindAllRenderTargets(void)
{
    LostIsland::g_pGraphics->GetContext()->OMSetRenderTargets(m_count, m_ppRTV, m_pDSV);

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)m_width;
    viewport.Height = (float)m_height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    LostIsland::g_pGraphics->GetContext()->RSSetViewports(1, &viewport);
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


void RenderTarget::BindSingleShaderResource(unsigned int p_nr, unsigned int p_slot, ShaderTarget p_target)
{
    if(p_target & TARGET_VS)
    {
        LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(p_slot, 1, &m_ppSRV[p_nr]);
    }
    if(p_target & TARGET_GS)
    {
        LostIsland::g_pGraphics->GetContext()->GSSetShaderResources(p_slot, 1, &m_ppSRV[p_nr]);
    }
    if(p_target & TARGET_PS)
    {
        LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(p_slot, 1, &m_ppSRV[p_nr]);
    }
    if(p_target & TARGET_CS)
    {
        LostIsland::g_pGraphics->GetContext()->CSSetShaderResources(p_slot, 1, &m_ppSRV[p_nr]);
    }
}


void RenderTarget::BindAllUnorderedAccess(unsigned int p_startSlot)
{
    unsigned int *pCounts = new unsigned int[m_count];
    ZeroMemory(pCounts, m_count * sizeof(unsigned int));
    LostIsland::g_pGraphics->GetContext()->CSSetUnorderedAccessViews(p_startSlot, m_count, m_ppUAV, pCounts);
    SAFE_DELETE(pCounts);
}


void RenderTarget::BindSingleUnorderedAccess(unsigned int p_nr, unsigned int p_slot)
{
    unsigned int count = 0;
    LostIsland::g_pGraphics->GetContext()->CSSetUnorderedAccessViews(p_slot, 1, &m_ppUAV[p_nr], &count);
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