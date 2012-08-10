#include "StdAfx.h"
#include "VertexBuffer.h"


D3D11_INPUT_ELEMENT_DESC VertexBuffer::sm_pSimpleVertexElementDesc[3] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
unsigned int VertexBuffer::sm_simpleVertexNumElements = 3;

D3D11_INPUT_ELEMENT_DESC VertexBuffer::sm_pScreenQuadVertexElementDesc[1] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
unsigned int VertexBuffer::sm_screenQuadVertexNumElements = 1;


VertexBuffer::VertexBuffer(void):
m_pBuffer(0)
{
}


VertexBuffer::~VertexBuffer(void)
{
    SAFE_RELEASE(m_pBuffer);
}


bool VertexBuffer::Build(const void* p_data, unsigned int p_numVertices, unsigned int p_byteStride)
{
    SAFE_RELEASE(m_pBuffer);

    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = p_numVertices * p_byteStride;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = m_byteStride = p_byteStride;
    desc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = p_data;

    HRESULT hr = LostIsland::g_pGraphics->GetDevice()->CreateBuffer(&desc, &data, &m_pBuffer);
    RETURN_IF_FAILED(hr);

    return true;
}


void VertexBuffer::Bind(void)
{
    static unsigned int offset = 0;
    LostIsland::g_pGraphics->GetContext()->IASetVertexBuffers(0, 1, &m_pBuffer, &m_byteStride, &offset);
}