#include "StdAfx.h"
#include "Geometry.h"


Geometry::Geometry(void) :
m_pIndices(0), m_pVertices(0)
{
}


Geometry::Geometry(const Geometry& p_toCopy):
m_pIndices(p_toCopy.m_pIndices), m_pVertices(p_toCopy.m_pVertices)
{
}


Geometry::~Geometry(void)
{
}


void Geometry::Draw(void)
{
    m_pIndices->Bind();
    m_pVertices->Bind();
    LostIsland::g_pGraphics->GetContext()->DrawIndexed(m_pIndices->GetIndexCount(), 0, 0);
}


bool Geometry::GenerateNormals(float* p_pVertices, unsigned int p_vertexStride, unsigned int p_posOffset, unsigned int p_normalOffset, const unsigned int* p_pIndices, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology)
{
    XMFLOAT3 pos0, pos1, pos2;
    unsigned int i0, i1, i2;
    unsigned int i = 0;
    unsigned int maxVertex = 0;
    while(i < p_indexCount)
    {
        switch(p_topology)
        {
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            i0 = p_pIndices[i++];
            i1 = p_pIndices[i++];
            i2 = p_pIndices[i++];
            break;
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            LI_WARNING("TODO: normal generation for D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP");
            return false;
        default:
            LI_ERROR("normal generation not supported for specified primitive topology");
            return false;
        }
        memcpy(&pos0, &p_pVertices[i0 * p_vertexStride + p_posOffset], 3 * sizeof(float));
        memcpy(&pos1, &p_pVertices[i1 * p_vertexStride + p_posOffset], 3 * sizeof(float));
        memcpy(&pos2, &p_pVertices[i2 * p_vertexStride + p_posOffset], 3 * sizeof(float));
        
        XMVECTOR a = XMLoadFloat3(&pos1) - XMLoadFloat3(&pos0);
        XMVECTOR b = XMLoadFloat3(&pos2) - XMLoadFloat3(&pos0);
        XMStoreFloat3(&pos0, XMVector3Cross(b, a));
        p_pVertices[i0 * p_vertexStride + p_normalOffset + 0] += pos0.x;
        p_pVertices[i0 * p_vertexStride + p_normalOffset + 1] += pos0.y;
        p_pVertices[i0 * p_vertexStride + p_normalOffset + 2] += pos0.z;
        p_pVertices[i1 * p_vertexStride + p_normalOffset + 0] += pos0.x;
        p_pVertices[i1 * p_vertexStride + p_normalOffset + 1] += pos0.y;
        p_pVertices[i1 * p_vertexStride + p_normalOffset + 2] += pos0.z;
        p_pVertices[i2 * p_vertexStride + p_normalOffset + 0] += pos0.x;
        p_pVertices[i2 * p_vertexStride + p_normalOffset + 1] += pos0.y;
        p_pVertices[i2 * p_vertexStride + p_normalOffset + 2] += pos0.z;

        maxVertex = max(i0, max(i1, i2));
    }
    for(unsigned int v=0; v < maxVertex; ++v)
    {
        float& x = p_pVertices[v * p_vertexStride + p_normalOffset + 0];
        float& y = p_pVertices[v * p_vertexStride + p_normalOffset + 1];
        float& z = p_pVertices[v * p_vertexStride + p_normalOffset + 2];
        float len = sqrt(x * x + y * y + z * z);
        x /= len;
        y /= len;
        z /= len;
    }
    return true;
}


bool Geometry::GenerateNormals(VertexBuffer::SimpleVertex* p_pVertices, const unsigned int* p_pIndices, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology)
{
    return GenerateNormals((float*)p_pVertices, sizeof(VertexBuffer::SimpleVertex) / sizeof(float), 0, 3, p_pIndices, p_indexCount, p_topology);
}