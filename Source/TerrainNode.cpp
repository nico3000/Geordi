#include "StdAfx.h"
#include "TerrainNode.h"
#include "Scene.h"
#include "MarchingCubeGrid.h"

#define GEOMETRY_UNKNOWN (0)
#define GEOMETRY_EMPTY (1)
#define GEOMETRY_LOADED (2)
#define GEOMETRY_NOTLOADED (3)


TerrainNode::TerrainNode(std::shared_ptr<TerrainData> p_pTerrain, int p_chunksize, int p_smallradius):
m_pTerrain(p_pTerrain), m_chunksize(p_chunksize), m_scale(0.25f)
{
    m_tempGrid.Init(p_chunksize + 2);
    for(int i=0; i < NUM_LEVELS; ++i)
    {
        m_pGeometryData[i].Init(-512, -512, -512, 1024);
    }
    MarchingCubeGrid::Init();
}



TerrainNode::~TerrainNode(void)
{
    for(int i=0; i < NUM_LEVELS; ++i)
    {
        m_pGeometryData[i].Clear();
    }
}


bool TerrainNode::IsValid(int p_blockX, int p_blockY, int p_blockZ, int p_camX, int p_camY, int p_camZ, int p_level) const
{
    bool inRange = IS_IN_RANGE(p_blockX, p_camX - 2, p_camX + 2)
        && IS_IN_RANGE(p_blockY, p_camY - 2, p_camY + 2)
        && IS_IN_RANGE(p_blockZ, p_camZ - 2, p_camZ + 2);
    if(inRange && p_level != 0)
    {
        return !IS_IN_RANGE(p_blockX, p_camX - 1, p_camX + 1)
            || !IS_IN_RANGE(p_blockY, p_camY - 1, p_camY + 1)
            || !IS_IN_RANGE(p_blockZ, p_camZ - 1, p_camZ + 1);
    }
    else
    {
        return inRange;
    }
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

    const XMFLOAT3& campos = p_pScene->GetCurrentCamera()->GetPosition();

    for(int level=0; level < NUM_LEVELS; ++level)
    {
        float levelBlockSize = (float)(1 << level) * m_scale * (float)m_chunksize;
        int camX = (int)floor(campos.x / levelBlockSize + 0.5f);
        int camY = (int)floor(campos.y / levelBlockSize + 0.5f);
        int camZ = (int)floor(campos.z / levelBlockSize + 0.5f);
        camX += camX % 2;
        camY += camY % 2;
        camZ += camZ % 2;


        BlockList& blocks = m_pBlockLists[level];
        for(auto iter=blocks.begin(); iter != blocks.end();)
        {
            if(!this->IsValid((*iter).x, (*iter).y, (*iter).z, camX, camY, camZ, level))
            {
                if(m_pGeometryData[level].GetValue((*iter).x, (*iter).y, (*iter).z) == GEOMETRY_LOADED)
                {
                    m_pGeometryData[level].SetValue((*iter).x, (*iter).y, (*iter).z, GEOMETRY_NOTLOADED);   
                }
                iter = blocks.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        for(int dx=-2; dx < 2; ++dx)
        {
            for(int dy=-2; dy < 2; ++dy)
            {
                for(int dz=-2; dz < 2; ++dz)
                {
                    if(this->IsValid(camX + dx, camY + dy, camZ + dz, camX, camY, camZ, level))
                    {
                        int val = m_pGeometryData->GetValue(camX + dx, camY + dy, camZ + dz);
                        bool wasGeometryCreated = false;
                        switch(val)
                        {
                        case GEOMETRY_UNKNOWN:
                            wasGeometryCreated = this->CreateBlock(level, camX + dx, camY + dy, camZ + dz);
                            m_pGeometryData->SetValue(camX + dx, camY + dy, camZ + dz, wasGeometryCreated ? GEOMETRY_LOADED : GEOMETRY_EMPTY);
                            return S_OK;
                            //break;
                        case GEOMETRY_NOTLOADED:
                            wasGeometryCreated = this->CreateBlock(level, camX + dx, camY + dy, camZ + dz);
                            if(!wasGeometryCreated)
                            {
                                LI_INFO("weird");
                            }
                            m_pGeometryData->SetValue(camX + dx, camY + dy, camZ + dz, wasGeometryCreated ? GEOMETRY_LOADED : GEOMETRY_NOTLOADED);
                            return S_OK;
                            //break;
                        case GEOMETRY_LOADED: /* do nothing */ break;
                        case GEOMETRY_EMPTY: /* do nothing */ break;
                        }
                    }
                }
            }
        }
    }
    return S_OK;
}


bool TerrainNode::CreateBlock(int p_level, int p_x, int p_y, int p_z)
{
    static MarchingCubeGrid grid;
    XMFLOAT4 color(1.0f, 0.5f, 0.5f, 1.0f);
    int offset = 1 << p_level;
    int startX = p_x * offset * m_chunksize;
    int startY = p_y * offset * m_chunksize;
    int startZ = p_z * offset * m_chunksize;
    
    bool hasGeometry = m_pTerrain->FillGrid(m_tempGrid, startX, startY, startZ, offset);
    if(hasGeometry)
    {    
        hasGeometry = grid.ConstructData(m_tempGrid, XMFLOAT3((float)startX, (float)startY, (float)startZ), m_scale, true);
    }
    if(hasGeometry)
    {
         TerrainBlock block = { p_x, p_y, p_z, p_level, grid.CreateGeometry() };
         m_pBlockLists[p_level].push_back(block);
         color.x = 0.5f;
         color.y = 1.0f;
    }

    // visualize chunks
    VertexBuffer::SimpleVertex pVertices[8] = {
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.01f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.01f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.01f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.01f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.99f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.99f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.99f))), XMFLOAT3(0,1,0), color,
        XMFLOAT3(m_scale * (float)(m_chunksize * offset * (p_x + 0.01f)), m_scale * (float)(m_chunksize * offset * (p_y + 0.99f)), m_scale * (float)(m_chunksize * offset * (p_z + 0.99f))), XMFLOAT3(0,1,0), color,
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
    TerrainBlock block2 = { p_x, p_y, p_z, p_level, pGeo };
    m_pBlockLists[p_level].push_back(block2);
    // end visualize chunks

    return hasGeometry;
}


HRESULT TerrainNode::VPreRender(Scene* p_pScene)
{
    return S_OK;
}


HRESULT TerrainNode::VRender(Scene* p_pScene)
{
    m_program.Bind();
    Pose::ModelMatrixData data;
    XMStoreFloat4x4(&data.model, XMMatrixIdentity());
    XMStoreFloat4x4(&data.modelInv, XMMatrixIdentity());
    p_pScene->PushModelMatrices(data, true);

    for(int level=0; level < NUM_LEVELS; ++level)
    {
        for(auto iter=m_pBlockLists[level].begin(); iter != m_pBlockLists[level].end(); ++iter)
        {    
            (*iter).pGeometry->Draw();
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
