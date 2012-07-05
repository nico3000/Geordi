#include "StdAfx.h"
#include "ScreenQuad.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

std::shared_ptr<ScreenQuad> ScreenQuad::sm_pInstance(0);


ScreenQuad::~ScreenQuad(void)
{
}


void ScreenQuad::Draw(void)
{
    m_pGeometry->Draw();
}


bool ScreenQuad::CreateInputLayoutForShader(ShaderProgram& p_shader)
{
    return p_shader.CreateInputLayout(VertexBuffer::sm_pScreenQuadVertexElementDesc, VertexBuffer::sm_screenQuadVertexNumElements);
}


std::weak_ptr<ScreenQuad> ScreenQuad::GetScreenQuad(void)
{
    if(!sm_pInstance)
    {
        sm_pInstance.reset(new ScreenQuad);

        unsigned int pIndices[4] = { 0, 1, 2, 3 };
        Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
        if(!pIndexBuffer->Build(pIndices, ARRAYSIZE(pIndices), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP))
        {
            sm_pInstance = 0;
            return sm_pInstance;
        }
        sm_pInstance->m_pGeometry->SetIndices(pIndexBuffer);

        VertexBuffer::ScreenQuadVertex pVertices[4] = {
            XMFLOAT2(-1.0f, -1.0f),
            XMFLOAT2(+1.0f, -1.0f),
            XMFLOAT2(-1.0f, +1.0f),
            XMFLOAT2(+1.0f, +1.0f),
        };
        Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
        if(!pVertexBuffer->Build(pVertices, ARRAYSIZE(pVertices), sizeof(VertexBuffer::ScreenQuadVertex)))
        {
            sm_pInstance = 0;
            return sm_pInstance;
        }
        sm_pInstance->m_pGeometry->SetVertices(pVertexBuffer);
    }
    return sm_pInstance;
}
