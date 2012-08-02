#include "StdAfx.h"
#include "TerrainNode.h"
#include "Scene.h"
#include "MarchingCubeGrid.h"


TerrainNode::TerrainNode(std::shared_ptr<TerrainData> p_pTerrain, int p_chunksize, int p_smallradius) :
    m_pTerrain(p_pTerrain), m_chunksize(p_chunksize), m_smallradius(p_smallradius)
{
    m_tempGrid.Init(p_chunksize + 2);
    m_blockData.Init(-512, -512, -512, 1024);
    MarchingCubeGrid::Init();
}



TerrainNode::~TerrainNode(void)
{
}


HRESULT TerrainNode::VOnRestore(void)
{
    if(!m_program.Load(L"./Shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS"))
    {
        return S_FALSE;
    }
    if(!m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements))
    {
        return S_FALSE;
    }

    return S_OK;
}


HRESULT TerrainNode::VOnLostDevice(void)
{
    return S_OK;
}


HRESULT TerrainNode::VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis)
{
//     static unsigned long elapsed = 0;
//     elapsed += p_deltaMillis;
//     if(elapsed < 1000)
//     {
//         return S_OK;
//     }
//     elapsed = 0;

    const XMFLOAT3& pos = p_pScene->GetCurrentCamera()->GetPosition();
    int posX = (int)floor(32.0f * pos.x / (float)m_chunksize);
    int posY = (int)floor(32.0f * pos.y / (float)m_chunksize);
    int posZ = (int)floor(32.0f * pos.z / (float)m_chunksize);

    int distance = 0;
    int freeBlock = -1;
    for(int i=0; i < NUM_BLOCKS; ++i)
    {
        int currentDistance = !m_blocks[i].pGeometry ? INT_MAX : max(abs(m_blocks[i].x - posX), max(abs(m_blocks[i].y - posY), abs(m_blocks[i].z - posZ)));
        if(currentDistance > distance)
        {
            distance = currentDistance;
            freeBlock = i;
            if(!m_blocks[i].pGeometry)
            {
                break;
            }
        }
    }
    if(freeBlock == -1)
    {
        return S_OK;
    }
    
    int toFillX, toFillY, toFillZ;
    boolean found = false;
    for(int dx=-(int)m_smallradius; dx <= m_smallradius; ++dx)
    {
        for(int dy=-(int)m_smallradius; dy <= m_smallradius; ++dy)
        {
            for(int dz=-(int)m_smallradius; dz <= m_smallradius; ++dz)
            {
                int currentDistance = max(abs(dx), max(abs(dy), abs(dz)));
                int x = posX + dx;
                int y = posY + dy;
                int z = posZ + dz;
                if(currentDistance < distance)
                {
                    short flags = m_blockData.GetValue(x, y, z);
                    if(flags == 0 || flags == 3) // undefined or (not empty and unused)
                    {
                        toFillX = x;
                        toFillY = y;
                        toFillZ = z;
                        found = true;
                        distance = currentDistance;
                    }
                }
            }
        }
    }
    if(found)
    {
        bool hasGeometry = m_pTerrain->FillGrid(m_tempGrid, m_chunksize * toFillX, m_chunksize * toFillY, m_chunksize * toFillZ);
        if(hasGeometry)
        {
            if(m_blocks[freeBlock].pGeometry)
            {
                m_blockData.SetValue(m_blocks[freeBlock].x, m_blocks[freeBlock].y, m_blocks[freeBlock].z, 3); // not empty and unused
            }
            m_blockData.SetValue(toFillX, toFillY, toFillZ, 1); // not empty and used
            static MarchingCubeGrid grid;
            bool check = grid.ConstructData(m_tempGrid, XMFLOAT3(toFillX * m_chunksize, toFillY * m_chunksize, toFillZ * m_chunksize), 1.0f, true);
            if(!check)
            {
                //LI_INFO("should not happen: no mc geometry created");
            }
            else
            {
                m_blocks[freeBlock].pGeometry = grid.CreateGeometry();
                m_blocks[freeBlock].x = toFillX;
                m_blocks[freeBlock].y = toFillY;
                m_blocks[freeBlock].z = toFillZ;
            }
            return S_OK;            
        }
        else
        {
            m_blockData.SetValue(toFillX, toFillY, toFillZ, 2); // empty (and unused)
            return S_OK;            
        }

        // visualize chunks
        VertexBuffer::SimpleVertex pVertices[8] = {
            XMFLOAT3(toFillX * m_chunksize,       toFillY * m_chunksize,       toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, toFillY * m_chunksize,       toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, (toFillY + 1) * m_chunksize, toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3(toFillX * m_chunksize,       (toFillY + 1) * m_chunksize, toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3(toFillX * m_chunksize,       toFillY * m_chunksize,       (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, toFillY * m_chunksize,       (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, (toFillY + 1) * m_chunksize, (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
            XMFLOAT3(toFillX * m_chunksize,       (toFillY + 1) * m_chunksize, (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,1,0,1),
        };
        unsigned int pIndices[] = {
            0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0xffffffff,
            3, 7, 0xffffffff,
            2, 6, 0xffffffff,
            1, 5, 
        };
        Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
        pVertexBuffer->Build(pVertices, ARRAYSIZE(pVertices), sizeof(VertexBuffer::SimpleVertex));
        Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
        pIndexBuffer->Build(pIndices, ARRAYSIZE(pIndices), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        std::shared_ptr<Geometry> pGeo(new Geometry);
        pGeo->SetIndices(pIndexBuffer);
        pGeo->SetVertices(pVertexBuffer);
        m_empty.push_back(pGeo);
        // end visualize chunks
    }
    else
    {
        //LI_INFO("only empty blocks nearby");
    }

    return S_OK;
}


HRESULT TerrainNode::VPreRender(Scene* p_pScene)
{
    return S_OK;
}


HRESULT TerrainNode::VRender(Scene* p_pScene)
{
    m_program.Bind();
    Pose::ModelMatrixData data;
    XMVECTOR det;
    XMStoreFloat4x4(&data.model, XMMatrixScaling(1.0f / 32.0f, 1.0f / 32.0f, 1.0f / 32.0f));
    XMStoreFloat4x4(&data.modelInv, XMMatrixInverse(&det, XMLoadFloat4x4(&data.model)));
    p_pScene->PushModelMatrices(data, true);

    for(int i=0; i < NUM_BLOCKS; ++i)
    {    
        if(m_blocks[i].pGeometry)
        {
            m_blocks[i].pGeometry->Draw();
        }
    }
    for(auto iter=m_empty.begin(); iter != m_empty.end(); ++iter)
    {
        (*iter)->Draw();
    }

    p_pScene->PopModelMatrices(false);
    return S_OK;
}


HRESULT TerrainNode::VPostRender(Scene* p_pScene)
{
    return S_OK;
}
