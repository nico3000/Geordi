#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Geometry
{
private:
    std::shared_ptr<IndexBuffer> m_pIndices;
    std::shared_ptr<VertexBuffer> m_pVertices;

public:
    typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;
    typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;

    Geometry(void);
    ~Geometry(void);

    void Draw(void);

    void SetIndices(IndexBufferPtr p_pIndices) { m_pIndices = p_pIndices; }
    void SetVertices(VertexBufferPtr p_pVertices) { m_pVertices = p_pVertices; }
    bool IsReady(void) const { return m_pIndices != 0 && m_pVertices != 0; }
    void Destroy(void) { m_pIndices.reset((IndexBuffer*)0); m_pVertices.reset((VertexBuffer*)0); }
    
    static bool GenerateNormals(float* p_pVertices, unsigned int p_vertexStride, unsigned int p_posOffset, unsigned int p_normalOffset, const unsigned int* p_pIndices, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology);
    static bool GenerateNormals(VertexBuffer::SimpleVertex* p_pVertices, const unsigned int* p_pIndices, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology);

};


typedef std::shared_ptr<Geometry> StrongGeometryPtr;
