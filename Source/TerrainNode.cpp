#include "StdAfx.h"
#include "TerrainNode.h"
#include "Scene.h"


TerrainNode::~TerrainNode(void)
{
}


HRESULT TerrainNode::VOnRestore(void)
{
    if(!m_program.Load("./Shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS"))
    {
        return S_FALSE;
    }
    if(!m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements))
    {
        return S_FALSE;
    }
    if(!m_pTerrain->Init(32, 4, 2, 4, 4))
    {
        LI_ERROR("TerrainData initialization failed");
        return S_FALSE;
    }
    m_pTerrain->GenerateTestData();

    std::vector<VertexBuffer::SimpleVertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int i = 0;

    for(int x=0; x < 4 * 32 - 1; ++x)
    {
        for(int y=0; y < 2 * 32 - 1; ++y)
        {
            for(int z=0; z < 4 * 32 - 1; ++z)
            {
                float base = m_pTerrain->GetDensity(x, y, z);
                float px = m_pTerrain->GetDensity(x+1, y, z);
                float py = m_pTerrain->GetDensity(x, y+1, z);
                float pz = m_pTerrain->GetDensity(x, y, z+1);

                if(base != px && base * px <= 0)
                {
                    float t = base / (base - px);
                    VertexBuffer::SimpleVertex vertex;
                    vertex.positionMC.x = (float)x + t;
                    vertex.positionMC.y = (float)y;
                    vertex.positionMC.z = (float)z;
                    vertex.vertexColor = XMFLOAT4(2.0f, 0.8f, 0.8f, 1.0f);
                    vertices.push_back(vertex);
                    indices.push_back(i++);
                }
                if(base != py && base * py <= 0)
                {
                    float t = base / (base - py);
                    VertexBuffer::SimpleVertex vertex;
                    vertex.positionMC.x = (float)x;
                    vertex.positionMC.y = (float)y + t;
                    vertex.positionMC.z = (float)z;
                    vertex.vertexColor = XMFLOAT4(0.8f, 2.0f, 0.8f, 1.0f);
                    vertices.push_back(vertex);
                    indices.push_back(i++);
                }
                if(base != pz && base * pz <= 0)
                {
                    float t = base / (base - pz);
                    VertexBuffer::SimpleVertex vertex;
                    vertex.positionMC.x = (float)x;
                    vertex.positionMC.y = (float)y;
                    vertex.positionMC.z = (float)z + t;
                    vertex.vertexColor = XMFLOAT4(0.8f, 0.8f, 2.0f, 1.0f);
                    vertices.push_back(vertex);
                    indices.push_back(i++);
                }
            }
        }
    }

    if(i == 0)
    {
        //LI_ERROR("no geometry created");
        return S_FALSE;
    }
    Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
    pIndexBuffer->Build(&indices[0], (unsigned int)indices.size(), D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    m_pGeometry->SetIndices(pIndexBuffer);
    Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
    pVertexBuffer->Build(&vertices[0], (unsigned int)vertices.size(), sizeof(VertexBuffer::SimpleVertex));
    m_pGeometry->SetVertices(pVertexBuffer);

    return S_OK;
}


HRESULT TerrainNode::VOnLostDevice(void)
{
    return S_OK;
}


HRESULT TerrainNode::VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis)
{
    return S_OK;
}


HRESULT TerrainNode::VPreRender(Scene* p_pScene)
{
    return S_OK;
}


HRESULT TerrainNode::VRender(Scene* p_pScene)
{
    Pose::ModelMatrixData data;
    XMStoreFloat4x4(&data.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));
    p_pScene->PushModelMatrices(data, true);
    m_program.Bind();
    m_pGeometry->Draw();
    return S_OK;
}


HRESULT TerrainNode::VPostRender(Scene* p_pScene)
{
    p_pScene->PopModelMatrices(false);
    return S_OK;
}
