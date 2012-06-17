#include "StdAfx.h"
#include "ConstantBuffer.h"


ConstantBuffer::ConstantBuffer(void):
m_byteWidth(0), m_pData(0), m_pBuffer(0)
{
}


ConstantBuffer::~ConstantBuffer(void)
{
    SAFE_RELEASE(m_pBuffer);
}


bool ConstantBuffer::Build(void* p_pData, unsigned int p_byteWidth)
{
    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = m_byteWidth = p_byteWidth;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.Usage = D3D11_USAGE_DYNAMIC;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = m_pData = p_pData;
    
    HRESULT hr = LostIsland::g_pGraphics->GetDevice()->CreateBuffer(&desc, &data, &m_pBuffer);
    RETURN_IF_FAILED(hr);

    return true;
}


void ConstantBuffer::Bind(unsigned int p_slot, int p_target)
{
    if(p_target & TARGET_VS)
    {
        LostIsland::g_pGraphics->GetContext()->VSSetConstantBuffers(p_slot, 1, &m_pBuffer);
    }
    if(p_target & TARGET_GS)
    {
        LostIsland::g_pGraphics->GetContext()->GSSetConstantBuffers(p_slot, 1, &m_pBuffer);
    }
    if(p_target & TARGET_PS)
    {
        LostIsland::g_pGraphics->GetContext()->PSSetConstantBuffers(p_slot, 1, &m_pBuffer);
    }
}


bool ConstantBuffer::Update(void)
{
    D3D11_MAPPED_SUBRESOURCE resource;
    HRESULT hr = LostIsland::g_pGraphics->GetContext()->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    RETURN_IF_FAILED(hr);
    memcpy(resource.pData, m_pData, m_byteWidth);
    LostIsland::g_pGraphics->GetContext()->Unmap(m_pBuffer, 0);
    return SUCCEEDED(hr);
}