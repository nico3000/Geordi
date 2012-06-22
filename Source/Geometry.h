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

};


typedef std::shared_ptr<Geometry> StrongGeometryPtr;