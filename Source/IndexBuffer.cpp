#include "StdAfx.h"
#include "IndexBuffer.h"


IndexBuffer::IndexBuffer(void):
m_pBuffer(0)
{
}


IndexBuffer::~IndexBuffer(void)
{
    SAFE_RELEASE(m_pBuffer);
}


bool IndexBuffer::Build(const unsigned int* p_data, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology)
{
    SAFE_RELEASE(m_pBuffer);

    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.ByteWidth = p_indexCount * sizeof(unsigned int);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = sizeof(unsigned int);
    desc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = p_data;

    HRESULT hr = LostIsland::g_pGraphics->GetDevice()->CreateBuffer(&desc, &data, &m_pBuffer);
    RETURN_IF_FAILED(hr);

    m_topology = p_topology;
    m_indexCount = p_indexCount;

    return true;
}


void IndexBuffer::Bind(void) const
{
    LostIsland::g_pGraphics->GetContext()->IASetIndexBuffer(m_pBuffer, DXGI_FORMAT_R32_UINT, 0);
    LostIsland::g_pGraphics->GetContext()->IASetPrimitiveTopology(m_topology);
}
