#pragma once
class RenderTarget
{
private:
    unsigned int m_count;
    unsigned int m_width;
    unsigned int m_height;
    ID3D11ShaderResourceView** m_ppSRV;
    ID3D11RenderTargetView** m_ppRTV;
    ID3D11UnorderedAccessView** m_ppUAV;
    ID3D11DepthStencilView* m_pDSV;
    DXGI_FORMAT* m_pFormats;
    DXGI_SAMPLE_DESC m_sampleDesc;

    void Reset(unsigned int p_count, unsigned int p_width, unsigned int p_height, const DXGI_FORMAT* p_pFormats);
    ID3D11Texture2D* CreateTexture2D(DXGI_FORMAT p_format, unsigned int p_bindFlags) const;
    bool CreateRTV2D(ID3D11Texture2D* p_pResource, unsigned int p_index);
    bool CreateSRV2D(ID3D11Texture2D* p_pResource, unsigned int p_index);
    bool CreateUAV2D(ID3D11Texture2D* p_pResource, unsigned int p_index);
    bool CreateDSV2D(void);

public:
    enum View {
        RTV = 1 << 0,
        DSV = 1 << 1,
        SRV = 1 << 2,
        UAV = 1 << 3,
        RTV_DSV = RTV | DSV,
        RTV_SRV = RTV | SRV,
        RTV_UAV = RTV | UAV,
        RTV_DSV_SRV = RTV | DSV | SRV,
        RTV_DSV_UAV = RTV | DSV | SRV,
        RTV_SRV_UAV = RTV | SRV | UAV,
        RTV_DSV_SRV_UAV = RTV | DSV | SRV | UAV,
    };

    RenderTarget(void);
    ~RenderTarget(void);

    bool Init2DMS(unsigned int p_width, unsigned int p_height, unsigned int p_count, View p_viewsToCreate, const DXGI_FORMAT* p_pFormats, const DXGI_SAMPLE_DESC& p_sampleDesc);
    bool Init2D(unsigned int p_width, unsigned int p_height, unsigned int p_count, View p_viewsToCreate, const DXGI_FORMAT* p_pFormats);
    bool Init2D(ID3D11Texture2D* p_pResource, View p_viewsToCreate);
    void Destroy(void);
    void BindSingleShaderResource(unsigned int p_nr, unsigned int p_slot, ShaderTarget p_target);
    void BindAllShaderResources(unsigned int p_startSlot, ShaderTarget p_target);
    void BindSingleUnorderedAccess(unsigned int p_nr, unsigned int p_slot);
    void BindAllUnorderedAccess(unsigned int p_startSlot);
    void BindAllRenderTargets(void);
    void ClearColor(const float p_pColor[4]);
    void ClearDepthStencil(float p_depth, unsigned int p_stencil);

    unsigned int GetCount(void) const { return m_count; }
    DXGI_FORMAT GetFormat(unsigned int p_index) const { return m_pFormats[p_index]; }

};

