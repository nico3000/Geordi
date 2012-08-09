#include "StdAfx.h"
#include "TerrainNode.h"
#include "Scene.h"
#include "MarchingCubeGrid.h"

#define GEOMETRY_UNKNOWN OCTREE_DEFAULT_VALUE
#define GEOMETRY_EMPTY (1)
#define GEOMETRY_LOADED (2)
#define GEOMETRY_NOTLOADED (3)


TerrainNode::TerrainBlock::TerrainBlock(int p_x, int p_y, int p_z, int p_level, TerrainNode* p_pTerrainNode):
m_x(p_x), m_y(p_y), m_z(p_z), m_level(p_level), m_isRefined(false), m_pTerrainNode(p_pTerrainNode)
{
    if(p_level != 0)
    {
        m_pRefined[0].reset(new TerrainBlock(2 * p_x,     2 * p_y,     2 * p_z,     p_level - 1, p_pTerrainNode));
        m_pRefined[1].reset(new TerrainBlock(2 * p_x,     2 * p_y,     2 * p_z + 1, p_level - 1, p_pTerrainNode));
        m_pRefined[2].reset(new TerrainBlock(2 * p_x,     2 * p_y + 1, 2 * p_z,     p_level - 1, p_pTerrainNode));
        m_pRefined[3].reset(new TerrainBlock(2 * p_x,     2 * p_y + 1, 2 * p_z + 1, p_level - 1, p_pTerrainNode));
        m_pRefined[4].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y,     2 * p_z,     p_level - 1, p_pTerrainNode));
        m_pRefined[5].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y,     2 * p_z + 1, p_level - 1, p_pTerrainNode));
        m_pRefined[6].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y + 1, 2 * p_z,     p_level - 1, p_pTerrainNode));
        m_pRefined[7].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y + 1, 2 * p_z + 1, p_level - 1, p_pTerrainNode));
    }    
    m_geometry = p_pTerrainNode->m_pGeometryData->GetValue(m_x, m_y, m_z);
}


TerrainNode::TerrainBlock::~TerrainBlock(void)
{
}


bool TerrainNode::TerrainBlock::SetPointOfReference(int p_x, int p_y, int p_z)
{
    bool done = true;
    int x = (int)floor((float)p_x / (float)(1 << m_level));
    int y = (int)floor((float)p_y / (float)(1 << m_level));
    int z = (int)floor((float)p_z / (float)(1 << m_level));
    if(m_level != 0 && MAX3(abs(x - m_x), abs(y - m_y), abs(z - m_z)) < LOD_RADIUS)
    {
        // refine block
        m_isRefined = true;
        for(int i=0; i < 8; ++i)
        {
            done &= m_pRefined[i]->SetPointOfReference(p_x, p_y, p_z);
        }
        if(done && m_pGeometry)
        {
            this->ReleaseGeometry(false);
        }
    }
    else
    {
        // do not refine
        m_isRefined = false;
        if(m_level != 0)
        {
            for(int i=0; i < 8; ++i)
            {
                m_pRefined[i]->ReleaseGeometry(true);
            }
        }
        done = this->BuildGeometry();
    }
    return done;
}


void TerrainNode::TerrainBlock::ReleaseGeometry(bool p_releaseChildren)
{
    if(m_geometry == GEOMETRY_LOADED)
    {
        m_pGeometry.reset((Geometry*)0);
        m_pWireframe.reset((Geometry*)0);
        m_geometry = GEOMETRY_NOTLOADED;
        m_pTerrainNode->m_pGeometryData->SetValue(m_x, m_y, m_z, m_geometry);
    }
    else if(p_releaseChildren && m_level != 0)
    {
        for(int i=0; i < 8; ++i)
        {
            m_pRefined[i]->ReleaseGeometry(true);
        }
    }
}


bool TerrainNode::TerrainBlock::BuildGeometry(void)
{
    bool changed = false;
    bool done = true;
    if(m_geometry == GEOMETRY_UNKNOWN && m_pTerrainNode->m_currentBlocksPerFrame < m_pTerrainNode->m_maxBlocksPerFrame)
    {
        ++m_pTerrainNode->m_currentBlocksPerFrame;

        int startX = (m_x - 0) * m_pTerrainNode->m_chunksize;
        int startY = (m_y - 0) * m_pTerrainNode->m_chunksize;
        int startZ = (m_z - 0) * m_pTerrainNode->m_chunksize;

        bool hasGeometry = m_pTerrainNode->m_pTerrain->FillGrid(m_pTerrainNode->m_weightGrid, m_pTerrainNode->m_materialGrid, startX - 1, startY - 1, startZ - 1, m_level);
        if(hasGeometry)
        {    
            int offset = 1 << m_level;
            hasGeometry = m_pTerrainNode->m_tempMCGrid.ConstructData(m_pTerrainNode->m_weightGrid, m_pTerrainNode->m_materialGrid, XMFLOAT3(
                (float)(m_x * m_pTerrainNode->m_chunksize),
                (float)(m_y * m_pTerrainNode->m_chunksize),
                (float)(m_z * m_pTerrainNode->m_chunksize)),
                (float)offset * m_pTerrainNode->m_scale);
        }
        if(hasGeometry)
        {
            m_pBackup = m_pTerrainNode->m_tempMCGrid.CreateGeometry(m_level == 0);
            m_geometry = GEOMETRY_NOTLOADED;
        }
        else
        {
            m_geometry = GEOMETRY_EMPTY;
        }
        changed = true;
    }
    else
    {
        done = m_geometry != GEOMETRY_UNKNOWN;
    }
    if(m_geometry == GEOMETRY_NOTLOADED && m_pBackup)
    {
        m_pGeometry.reset(new Geometry(*m_pBackup));
        m_pTerrainNode->m_blockList.push_back(m_pGeometry);
        m_geometry = GEOMETRY_LOADED;
        changed = true;
    }
    if(changed)
    {
        m_pTerrainNode->m_pGeometryData->SetValue(m_x, m_y, m_z, m_geometry);
    }
    return done;


//         if(hasGeometry)
//         {
//             // wireframe
//             XMFLOAT4 pColors[] =
//             {
//                 XMFLOAT4(1.0f, 0.0f, 0.5f, 1.0f),
//                 XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
//                 XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
//                 XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
//                 XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
//             };
// 
//             float scale = m_pTerrainNode->m_scale;
//             int chunksize = m_pTerrainNode->m_chunksize;
//             VertexBuffer::SimpleVertex pVertices[8] = {
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.01f)), scale * (float)(chunksize * offset * (m_y + 0.01f)), scale * (float)(chunksize * offset * (m_z + 0.01f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.99f)), scale * (float)(chunksize * offset * (m_y + 0.01f)), scale * (float)(chunksize * offset * (m_z + 0.01f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.99f)), scale * (float)(chunksize * offset * (m_y + 0.99f)), scale * (float)(chunksize * offset * (m_z + 0.01f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.01f)), scale * (float)(chunksize * offset * (m_y + 0.99f)), scale * (float)(chunksize * offset * (m_z + 0.01f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.01f)), scale * (float)(chunksize * offset * (m_y + 0.01f)), scale * (float)(chunksize * offset * (m_z + 0.99f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.99f)), scale * (float)(chunksize * offset * (m_y + 0.01f)), scale * (float)(chunksize * offset * (m_z + 0.99f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.99f)), scale * (float)(chunksize * offset * (m_y + 0.99f)), scale * (float)(chunksize * offset * (m_z + 0.99f))), XMFLOAT3(0,1,0), pColors[m_level],
//                 XMFLOAT3(scale * (float)(chunksize * offset * (m_x + 0.01f)), scale * (float)(chunksize * offset * (m_y + 0.99f)), scale * (float)(chunksize * offset * (m_z + 0.99f))), XMFLOAT3(0,1,0), pColors[m_level],
//             };
//             unsigned int pIndices[] = {
//                 0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0xffffffff,
//                 3, 7, 0xffffffff,
//                 2, 6, 0xffffffff,
//                 1, 5, 
//             };
//             Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
//             pVertexBuffer->Build(pVertices, ARRAYSIZE(pVertices), sizeof(VertexBuffer::SimpleVertex));
//             Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
//             pIndexBuffer->Build(pIndices, ARRAYSIZE(pIndices), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
//             m_pWireframe.reset(new Geometry);
//             m_pWireframe->SetIndices(pIndexBuffer);
//             m_pWireframe->SetVertices(pVertexBuffer);
//             m_pTerrainNode->m_wireframeList.push_back(m_pWireframe);
//             // end wireframe
//         }
}


TerrainNode::TerrainNode(std::shared_ptr<TerrainData> p_pTerrain, int p_chunksize, int p_smallradius):
m_pTerrain(p_pTerrain), m_chunksize(p_chunksize), m_scale(0.25f), m_maxBlocksPerFrame(1), m_pDiffuseTex(0), m_pBumpTex(0), m_pNormalTex(0)
{
    m_weightGrid.Init(p_chunksize + 3);
    m_materialGrid.Init(p_chunksize + 3);
    int size = NUM_BLOCKS * (1 << NUM_LEVELS);
    for(int i=0; i < NUM_LEVELS; ++i)
    {
        m_pGeometryData[i].Init(-size / 2, -size / 2, -size / 2, size);
        size /= 2;
    }
    MarchingCubeGrid::Init();
    for(int x=0; x < NUM_BLOCKS; ++x)
    {
        for(int y=0; y < NUM_BLOCKS; ++y)
        {
            for(int z=0; z < NUM_BLOCKS; ++z)
            {
                m_pTest[z * NUM_BLOCKS * NUM_BLOCKS + y * NUM_BLOCKS + x].reset(new TerrainBlock(x - NUM_BLOCKS / 2, y - NUM_BLOCKS / 2, z - NUM_BLOCKS / 2, NUM_LEVELS - 1, this));
            }
        }
    }
}



TerrainNode::~TerrainNode(void)
{
    for(int i=0; i < NUM_LEVELS; ++i)
    {
        m_pGeometryData[i].Clear();
    }
    SAFE_RELEASE(m_pDiffuseTex);
    SAFE_RELEASE(m_pBumpTex);
    SAFE_RELEASE(m_pNormalTex);
}


HRESULT TerrainNode::VOnRestore(void)
{
    if(!m_program.Load(L"./Shader/Terrain.hlsl", "TerrainVS", 0, "TerrainNormalMappingPS"))
    {
        return S_FALSE;
    }
    if(!m_program.CreateInputLayout(MarchingCubeGrid::sm_pTerrainVertexElementDesc, MarchingCubeGrid::sm_terrainVertexNumElements))
    {
        return S_FALSE;
    }
    D3DX11_IMAGE_LOAD_INFO info;
    info.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    info.CpuAccessFlags = D3DX11_DEFAULT;
    info.Width = D3DX11_DEFAULT;
    info.Height = D3DX11_DEFAULT;
    info.Depth = D3DX11_DEFAULT;
    info.Filter = D3DX11_DEFAULT;
    info.FirstMipLevel = D3DX11_DEFAULT;
    info.Format = DXGI_FORMAT_FROM_FILE;
    info.MipFilter = D3DX11_DEFAULT;
    info.MipLevels = D3DX11_DEFAULT;
    info.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    info.pSrcInfo = 0;       
    RETURN_IF_FAILED(D3DX11CreateShaderResourceViewFromFileA(LostIsland::g_pGraphics->GetDevice(), "../Assets/polishedstone_diffuse.jpg", &info, 0, &m_pDiffuseTex, 0));
    RETURN_IF_FAILED(D3DX11CreateShaderResourceViewFromFileA(LostIsland::g_pGraphics->GetDevice(), "../Assets/polishedstone_bump.jpg", &info, 0, &m_pBumpTex, 0));
    RETURN_IF_FAILED(D3DX11CreateShaderResourceViewFromFileA(LostIsland::g_pGraphics->GetDevice(), "../Assets/polishedstone_normal.jpg", &info, 0, &m_pNormalTex, 0));
    LostIsland::g_pGraphics->GetContext()->GenerateMips(m_pDiffuseTex);
    LostIsland::g_pGraphics->GetContext()->GenerateMips(m_pBumpTex);
    LostIsland::g_pGraphics->GetContext()->GenerateMips(m_pNormalTex);

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
//     if(elapsed < 100)
//     {
//         return S_OK;
//     }
//     elapsed = 0;

    //const XMFLOAT3& campos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    const XMFLOAT3& campos = p_pScene->GetCurrentCamera()->GetPosition();

    static int lastCamPos[3] = { 0, 0, 0, };
    static bool done = false;

    float levelZeroBlockSize = m_scale * (float)m_chunksize;
    int camX = (int)floor(campos.x / levelZeroBlockSize);
    int camY = (int)floor(campos.y / levelZeroBlockSize);
    int camZ = (int)floor(campos.z / levelZeroBlockSize);

     if(!done || camX != lastCamPos[0] || camY != lastCamPos[1] || camZ != lastCamPos[2])
     {
         m_currentBlocksPerFrame = 0;
         done = true;

        for(int i=0; i < NUM_BLOCKS * NUM_BLOCKS * NUM_BLOCKS/* && m_currentBlocksPerFrame < m_maxBlocksPerFrame*/; ++i)
        {
            done &= m_pTest[i]->SetPointOfReference(camX, camY, camZ);
        }
        lastCamPos[0] = camX;
        lastCamPos[1] = camY;
        lastCamPos[2] = camZ;
     }   
    return S_OK;
}


HRESULT TerrainNode::VPreRender(Scene* p_pScene)
{
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(0, 1, &m_pDiffuseTex);
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(1, 1, &m_pBumpTex);
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(2, 1, &m_pNormalTex);
    return S_OK;
}


HRESULT TerrainNode::VRender(Scene* p_pScene)
{
    m_program.Bind();
    Pose::ModelMatrixData data;
    XMStoreFloat4x4(&data.model, XMMatrixIdentity());
    XMStoreFloat4x4(&data.modelInv, XMMatrixIdentity());
    p_pScene->PushModelMatrices(data, true);

    auto iter = m_blockList.begin();
    while(iter != m_blockList.end()) {
        std::shared_ptr<Geometry> pGeo = (*iter).lock();
        if(pGeo)
        {
            pGeo->Draw();
            ++iter;
        }
        else
        {
            iter = m_blockList.erase(iter);
        }
    }
    static int frames = 0;
    if(++frames == 1000)
    {
        frames = 0;
        std::ostringstream info;
        info << m_blockList.size() << " blocks";
        LI_INFO(info.str());
    }

    iter = m_wireframeList.begin();
    while(iter != m_wireframeList.end()) {
        std::shared_ptr<Geometry> pGeo = (*iter).lock();
        if(pGeo)
        {
            pGeo->Draw();
            ++iter;
        }
        else
        {
            iter = m_wireframeList.erase(iter);
        }
    }

    p_pScene->PopModelMatrices(false);
    return S_OK;
}


HRESULT TerrainNode::VPostRender(Scene* p_pScene)
{
    return S_OK;
}
