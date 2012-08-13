#include "StdAfx.h"
#include "TerrainData.h"
#include "Geometry.h"

#define HEIGHT_MASK 0x0000ffff
#define MATERIAL_MASK 0x000f0000
#define MAX_ACTIVE_OCTREES_PER_LEVEL 8

#define GEOMETRY_UNKNOWN OCTREE_DEFAULT_VALUE
#define GEOMETRY_EMPTY 1
#define GEOMETRY_NOTEMPTY 2

#define LOD_RADIUS 4
#define MAX_SIZE (1 << 14)

static int g_geometriesPerFrame = 0;

//////////////////////////////////////////////////////////////////////////
//                              TerrainData                             //
//////////////////////////////////////////////////////////////////////////
TerrainData::TerrainData(void):
m_chunksize(0), m_pChunkData(0), m_scale(0.25f), m_terrainFolder(""), m_referenceX(0), m_referenceY(0), m_referenceZ(0), m_referenceChanged(false)
{
}


TerrainData::~TerrainData(void)
{
    this->SaveAllData();
    for(int level=0; level < m_levels.size(); ++level)
    {
        std::ostringstream filename;
        filename << "./" << m_terrainFolder << "/chunks." << level << "." << m_chunksize << ".oct";
        std::fstream dataStream(filename.str(), std::ios::out | std::ios::binary | std::ios::trunc);
        bool success = m_pChunkData[level].Save(dataStream);
        if(!success)
        {
            LI_WARNING("Failed to save chunk data to " + filename.str());
        }
        m_pChunkData[level].Clear();
    }
    SAFE_DELETE_ARRAY(m_pChunkData);
}


int TerrainData::Implode(float p_density, int p_material)
{
    return (int)(CLAMP(p_density, -1.0, 1.0) * (float)SHORT_MAX) | (p_material << 16);
}


void TerrainData::Explode(int p_value, float& p_density, int& p_material)
{
    p_material = p_value >> 16;
    p_density = (float)(short)(p_value & 0x0000ffff) / (float)SHORT_MAX;
}


bool TerrainData::Init(std::string p_terrainFolder, unsigned char p_levels, unsigned short p_octreeSize, int p_chunksize)
{
    if(!Octree::InitMemoryPool(MAX_ACTIVE_OCTREES_PER_LEVEL * p_octreeSize * p_octreeSize)) // TODO: educated guess :P
    {
        LI_ERROR("Failed to initialize memory pool for octrees");
    }

    m_weightGrid.Init(p_chunksize + 3);
    m_materialGrid.Init(p_chunksize + 3);

    m_pChunkData = new Octree[p_levels];
    int size = MAX_SIZE;
    for(unsigned char level=0; level < p_levels; ++level)
    {
        std::ostringstream filename;
        filename << "./" << p_terrainFolder << "/chunks." << (int)level << "." << p_chunksize << ".oct";
        std::fstream dataStream(filename.str(), std::ios::in | std::ios::binary);
        bool success = false;
        if(dataStream.is_open())
        {
            if(m_pChunkData[level].Init(dataStream))
            {
                success = true;
            }
        }
        if(!success)
        {
            LI_INFO("could not load chunkfile " + filename.str());
            m_pChunkData[level].Init(-size / 2, -size / 2, -size / 2, size);
        }
        size /= 2;

        m_levels.push_back(LevelData());
        if(!m_levels.back().Init(p_terrainFolder, level, p_octreeSize))
        {
            std::ostringstream info;
            info << "Failed to initialize level " << level;
            LI_ERROR(info.str());
            return false;
        }
    }
    
    m_terrainFolder = p_terrainFolder;
    m_chunksize = p_chunksize;
    return true;
}


void TerrainData::SetRawValue(int p_x, int p_y, int p_z, int p_rawValue, bool p_autoOptimizeStructure)
{
    for(auto iter=m_levels.begin(); iter != m_levels.end(); ++iter)
    {
        (*iter).SetRawValue(p_x, p_y, p_z, p_rawValue, p_autoOptimizeStructure);
        if(p_x % 2 != 0 || p_y % 2 != 0 || p_z % 2 != 0) break;
        p_x /= 2;
        p_y /= 2;
        p_z /= 2;
    }
}


void TerrainData::SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure /* = true */)
{
    float density;
    int material;
    this->Explode(m_levels[0].GetRawValue(p_x, p_y, p_z), density, material);
    this->SetRawValue(p_x, p_y, p_z, this->Implode(p_density, material), p_autoOptimizeStructure);
}


float TerrainData::GetDensity(int p_x, int p_y, int p_z)
{
    float density;
    int material;
    this->Explode(m_levels[0].GetRawValue(p_x, p_y, p_z), density, material);
    return density;
}


void TerrainData::SetMaterial(int p_x, int p_y, int p_z, int p_material, bool p_autoOptimizeStructure /* = true */)
{
    float density;
    int material;
    this->Explode(m_levels[0].GetRawValue(p_x, p_y, p_z), density, material);
    this->SetRawValue(p_x, p_y, p_z, this->Implode(density, p_material), p_autoOptimizeStructure);
}


int TerrainData::GetMaterial(int p_x, int p_y, int p_z)
{
    float density;
    int material;
    this->Explode(m_levels[0].GetRawValue(p_x, p_y, p_z), density, material);
    return material;
}


bool TerrainData::FillGrid(Grid3D& p_weightGrid, Grid3D& p_materialGrid, int p_startX, int p_startY, int p_startZ, int p_level)
{
    return m_levels[p_level].FillGrid(p_weightGrid, p_materialGrid, p_startX, p_startY, p_startZ);
}


void TerrainData::SaveAllData(void) const
{
    for(auto iter=m_levels.begin(); iter != m_levels.end(); ++iter)
    {
        (*iter).SaveAllOctrees();
    }
}


void TerrainData::CreateTerrainBlock(int p_x, int p_y, int p_z)
{
    BlockInfo info = { std::shared_ptr<TerrainBlock>(new TerrainBlock(p_x, p_y, p_z, (int)m_levels.size() - 1, this)), false };
    m_blockVector.push_back(info);
}


void TerrainData::SetPointOfReference(float p_worldX, float p_worldY, float p_worldZ)
{
    float levelZeroBlockSize = m_scale * (float)m_chunksize;
    int newReferenceX = (int)floor(p_worldX / levelZeroBlockSize);
    int newReferenceY = (int)floor(p_worldY / levelZeroBlockSize);
    int newReferenceZ = (int)floor(p_worldZ / levelZeroBlockSize);
    if(m_referenceX != newReferenceX || m_referenceY != newReferenceY || m_referenceZ != newReferenceZ)
    {
        m_referenceX = newReferenceX;
        m_referenceY = newReferenceY;
        m_referenceZ = newReferenceZ;
        m_referenceChanged = true;
    }
}


void TerrainData::Update(unsigned long p_maxMillis)
{
    g_geometriesPerFrame = 0;
    static int timerID = LostIsland::g_pTimer->Tick(IMMEDIATE);
    LostIsland::g_pTimer->Tock(timerID, RESET);
    bool timeOut = false;
    for(auto iter=m_blockVector.begin(); iter != m_blockVector.end(); ++iter)
    {
        BlockInfo& info = (*iter);
        if(!timeOut)
        {
            timeOut = p_maxMillis < LostIsland::g_pTimer->Tock(timerID, KEEPRUNNING);
        }
        if(m_referenceChanged || !info.done)
        {
            if(!timeOut)
            {
                info.done = info.pBlock->Update(m_referenceX, m_referenceY, m_referenceZ, timerID, p_maxMillis);
            }
            else
            {
                info.done = false;
            }
        }
    }
    m_referenceChanged = false;

    std::ostringstream info;
    info << g_geometriesPerFrame << " chunks checked, remaining time: " << ((int)p_maxMillis - (int)LostIsland::g_pTimer->Tock(timerID, RESET)) << "ms";
    LI_INFO(info.str());
}


//////////////////////////////////////////////////////////////////////////
//                               LevelData                              //
//////////////////////////////////////////////////////////////////////////

LevelData::LevelData(void)
{

}


LevelData::~LevelData(void)
{
    while(!m_loadedTrees.empty())
    {
        this->SaveOctree(0);
        this->DestroyOctree(0);
    }
}


bool LevelData::Init(std::string p_terrainFolder, int p_level, unsigned short p_octreeSize)
{
    m_terrainFolder = p_terrainFolder;
    m_level = p_level;
    m_octreeSize = p_octreeSize;
    return true;
}


void LevelData::PushTileToBack(int p_x, int p_y, int p_z)
{
    if(!m_loadedTrees.empty() && m_loadedTrees.back().pTree->IsIn(p_x, p_y, p_z))
    {
        return;
    }
    for(auto iter=m_loadedTrees.begin(); iter != m_loadedTrees.end(); ++iter)
    {
        if((*iter).pTree->IsIn(p_x, p_y, p_z))
        {
            LoadedOctree& loaded = (*iter);
            m_loadedTrees.push_back(loaded);
            m_loadedTrees.erase(iter);
            return;
        }
    }
    while(m_loadedTrees.size() > MAX_ACTIVE_OCTREES_PER_LEVEL)
    {
        if(!this->SaveOctree(0))
        {
            LI_ERROR("Saving octree failed");
        }
        this->DestroyOctree(0);
        m_loadedTrees.pop_front();
    }
    this->LoadOctree(p_x, p_y, p_z);
}


void LevelData::DestroyOctree(unsigned int p_index)
{
    auto iter = m_loadedTrees.begin();
    while(p_index != 0)
    {
        ++iter;
        --p_index;
    }
    (*iter).pTree->Clear();
    SAFE_DELETE((*iter).pTree);
    m_loadedTrees.erase(iter);
}


bool LevelData::SaveOctree(unsigned int p_index) const
{
    bool success = true;
    auto iter = m_loadedTrees.begin();
    while(p_index != 0)
    {
        ++iter;
        --p_index;
    }
    const LoadedOctree& loaded = (*iter);
    if(loaded.changed)
    {
        Octree* pTree = loaded.pTree;
        pTree->OptimizeStructure();
        if(!pTree->IsEmpty())
        {
            std::stringstream octreeStream;
            octreeStream << "./" << m_terrainFolder << "/terrain." << m_level << "." << pTree->GetMinX() << "." << pTree->GetMinY() << "." << pTree->GetMinZ() << ".oct";
            std::fstream fileStream(octreeStream.str().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
            if(fileStream.is_open())
            {
                LI_INFO("saving octree \"" + octreeStream.str() + "\"");
                if(!pTree->Save(fileStream))
                {
                    LI_ERROR("Failed to save octree to stream: " + octreeStream.str());
                    success = false;
                }
            }
            else
            {
                LI_ERROR("Failed to open octree stream for writing: " + octreeStream.str());
            }
            fileStream.close();
        }
        else
        {
            LI_INFO("Saving of empty octree skipped");
        }
    }
    else
    {
        LI_INFO("Saving of non-changed octree skipped");
    }
    return success;
}


bool LevelData::LoadOctree(int p_x, int p_y, int p_z)
{
    int tileStartX = (int)floor((float)p_x / (float)m_octreeSize) * m_octreeSize;
    int tileStartY = (int)floor((float)p_y / (float)m_octreeSize) * m_octreeSize;
    int tileStartZ = (int)floor((float)p_z / (float)m_octreeSize) * m_octreeSize;

    LoadedOctree loaded;
    loaded.pTree = new Octree;
    loaded.changed = false;

    bool success = true;
    std::stringstream octreeStream;
    octreeStream << "./" << m_terrainFolder << "/terrain." << m_level << "." << tileStartX << "." << tileStartY << "." << tileStartZ << ".oct";
    std::fstream fileStream(octreeStream.str().c_str(), std::ios::in | std::ios::binary);
    if(fileStream.is_open())
    {
        if(!loaded.pTree->Init(fileStream))
        {
            LI_ERROR("Failed to load octree from stream: " + octreeStream.str());
            success = false;
        }
        LI_INFO("loading octree \"" + octreeStream.str() + "\"");
        fileStream.close();
    }
    else
    {
        loaded.pTree->Init(tileStartX, tileStartY, tileStartZ, m_octreeSize);
    }

    if(success)
    {
        m_loadedTrees.push_back(loaded);
    }
    return success;
}


int LevelData::GetRawValue(int p_x, int p_y, int p_z)
{
    this->PushTileToBack(p_x, p_y, p_z);
    return m_loadedTrees.back().pTree->GetValue(p_x, p_y, p_z);
}


void LevelData::SetRawValue(int p_x, int p_y, int p_z, int p_value, bool p_autoOptimizeStructure /* = true */)
{
    this->PushTileToBack(p_x, p_y, p_z);
    LoadedOctree& loaded = m_loadedTrees.back();
    loaded.changed |= loaded.pTree->SetValue(p_x, p_y, p_z, p_value, p_autoOptimizeStructure);
}


void LevelData::SaveAllOctrees(void) const
{
    for(int i=0; i < m_loadedTrees.size(); ++i)
    {
        this->SaveOctree(i);
    }
}


void LevelData::OptimizeAllOctrees(void) const
{
    for(auto iter=m_loadedTrees.begin(); iter != m_loadedTrees.end(); ++iter)
    {
        (*iter).pTree->OptimizeStructure();
    }
}


bool LevelData::FillGrid(Grid3D& p_weightGrid, Grid3D& p_materialGrid, int p_startX, int p_startY, int p_startZ)
{
    char geo = 0; // 0 undefined, 1 only positive values, 2 only negative values, 3 positive and negative values
    for(int x=0; x < p_weightGrid.GetSize(); ++x)
    {
        for(int y=0; y < p_weightGrid.GetSize(); ++y)
        {
            for(int z=0; z < p_weightGrid.GetSize(); ++z)
            {
                float density;
                int material;
                int raw = this->GetRawValue(p_startX + x, p_startY + y, p_startZ + z);
                TerrainData::Explode(raw, density, material);
                if(density > 0)
                {
                    switch(geo)
                    {
                    case 0: geo = 1; break;
                    case 1: break;
                    case 2: geo = 3; break;
                    case 3: break;
                    }
                }
                else if(density < 0)
                {
                    switch(geo)
                    {
                    case 0: geo = 2; break;
                    case 1: geo = 3; break;
                    case 2: break;
                    case 3: break;
                    }
                }
                p_weightGrid.SetValue(x, y, z, density);
                p_materialGrid.SetValue(x, y, z, (float)material);
            }
        }
    }
    return geo == 3;
}


//////////////////////////////////////////////////////////////////////////
//                              TerrainBlock                            //
//////////////////////////////////////////////////////////////////////////

TerrainBlock::TerrainBlock(int p_x, int p_y, int p_z, int p_level, TerrainData* p_pData):
m_x(p_x), m_y(p_y), m_z(p_z), m_level(p_level), m_isRefined(false), m_pData(p_pData), m_pPhysicsActor(0)
{
    if(p_level != 0)
    {
        m_pRefined[0].reset(new TerrainBlock(2 * p_x,     2 * p_y,     2 * p_z,     p_level - 1, p_pData));
        m_pRefined[1].reset(new TerrainBlock(2 * p_x,     2 * p_y,     2 * p_z + 1, p_level - 1, p_pData));
        m_pRefined[2].reset(new TerrainBlock(2 * p_x,     2 * p_y + 1, 2 * p_z,     p_level - 1, p_pData));
        m_pRefined[3].reset(new TerrainBlock(2 * p_x,     2 * p_y + 1, 2 * p_z + 1, p_level - 1, p_pData));
        m_pRefined[4].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y,     2 * p_z,     p_level - 1, p_pData));
        m_pRefined[5].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y,     2 * p_z + 1, p_level - 1, p_pData));
        m_pRefined[6].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y + 1, 2 * p_z,     p_level - 1, p_pData));
        m_pRefined[7].reset(new TerrainBlock(2 * p_x + 1, 2 * p_y + 1, 2 * p_z + 1, p_level - 1, p_pData));
    }
    m_flag = m_pData->m_pChunkData[p_level].GetValue(p_x, p_y, p_z);
}


TerrainBlock::~TerrainBlock(void)
{
}


bool TerrainBlock::Update(int p_referenceX, int p_referenceY, int p_referenceZ, int p_timerID, unsigned long p_maxMillis)
{
    int x = (int)floor((float)p_referenceX / (float)(1 << m_level));
    int y = (int)floor((float)p_referenceY / (float)(1 << m_level));
    int z = (int)floor((float)p_referenceZ / (float)(1 << m_level));
    if(m_level != 0 && MAX3(abs(x - m_x), abs(y - m_y), abs(z - m_z)) < LOD_RADIUS)
    {
        // refine block
        bool done = true;
        m_isRefined = true;
        for(int i=0; i < 8; ++i)
        {
            done &= m_pRefined[i]->Update(p_referenceX, p_referenceY, p_referenceZ, p_timerID, p_maxMillis);
        }
        if(done)
        {
            m_pGeometry.reset((Geometry*)0);
        }
        return done;
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
        return this->BuildGeometry(p_timerID, p_maxMillis);
    }
}


void TerrainBlock::ReleaseGeometry(bool p_releaseChildren)
{
    if(m_pGeometry)
    {
        m_pGeometry.reset((Geometry*)0);
    }
    else if(p_releaseChildren && m_level != 0)
    {
        for(int i=0; i < 8; ++i)
        {
            m_pRefined[i]->ReleaseGeometry(true);
        }
    }
}


bool TerrainBlock::UseGeometryFromBackup(void)
{
    if(!m_pGeometry && m_pBackup)
    {
        m_pGeometry.reset(new Geometry(*m_pBackup));
        m_pData->m_chunkList.push_back(m_pGeometry);
        return true;
    }
    else
    {
        return false;
    }
}


bool TerrainBlock::BuildGeometry(int p_timerID, unsigned long p_maxMillis)
{
    if(m_flag == GEOMETRY_EMPTY)
    {
        return true;
    }
    if(!m_pGeometry && !this->UseGeometryFromBackup())
    {
        if(p_maxMillis < LostIsland::g_pTimer->Tock(p_timerID, KEEPRUNNING))
        {
            return false;
        }
        else
        {
            ++g_geometriesPerFrame;
            std::ostringstream info;
            info << "now building " << m_x << "," << m_y << "," << m_z << " on level " <<m_level;
            LI_INFO(info.str());
            int startX = m_x * m_pData->m_chunksize;
            int startY = m_y * m_pData->m_chunksize;
            int startZ = m_z * m_pData->m_chunksize;

            bool hasGeometry = m_pData->FillGrid(m_pData->m_weightGrid, m_pData->m_materialGrid, startX - 1, startY - 1, startZ - 1, m_level);
            if(hasGeometry)
            {
                int offset = 1 << m_level;
                hasGeometry = m_pData->m_tempMCGrid.ConstructData(m_pData->m_weightGrid, m_pData->m_materialGrid, XMFLOAT3(
                    (float)(m_x * m_pData->m_chunksize),
                    (float)(m_y * m_pData->m_chunksize),
                    (float)(m_z * m_pData->m_chunksize)),
                    (float)offset * m_pData->m_scale, m_level);
            }
            if(hasGeometry)
            {
                m_pBackup = m_pData->m_tempMCGrid.CreateGeometry();
                this->UseGeometryFromBackup();
            }
            if(!hasGeometry && m_flag == GEOMETRY_NOTEMPTY)
            {
                LI_INFO("wtf");
            }
            //bool hasGeometry = false;
            if(m_flag == GEOMETRY_UNKNOWN)
            {
                m_flag = hasGeometry ? GEOMETRY_NOTEMPTY : GEOMETRY_EMPTY;
                m_pData->m_pChunkData[m_level].SetValue(m_x, m_y, m_z, m_flag);
            }
        }
    }
    return true;


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





void TerrainData::GenerateTestData(void)
{
    return;

    Grid3D noise[3];
    noise[0].Init(16);
    noise[1].Init(16);
    noise[2].Init(16);
    noise[0].LoadNoise();
    noise[1].LoadNoise();
    noise[2].LoadNoise();
    static const int presetOctreeRadius = 2;
    static const int octreeSize = m_levels[0].m_octreeSize;
    
    int lastPercentage = -1;
    int timerID = LostIsland::g_pTimer->Tick(IMMEDIATE);
    int outID = LostIsland::g_pTimer->Tick(IMMEDIATE);

    float cosa0 = cos(noise[0].GetValue(0));
    float sina0 = sin(noise[0].GetValue(0));
    float cosa1 = cos(noise[1].GetValue(0));
    float sina1 = sin(noise[1].GetValue(0));
    float cosa2 = cos(noise[2].GetValue(0));
    float sina2 = sin(noise[2].GetValue(0));
    float cosa3 = cos(noise[0].GetValue(1));
    float sina3 = sin(noise[0].GetValue(1));
    ULONGLONG i = 0;
    ULONGLONG wholesize = 2 * presetOctreeRadius * octreeSize;
    wholesize = wholesize * wholesize * wholesize;
    
    for(int ox=-presetOctreeRadius; ox < presetOctreeRadius; ++ox)
    {
        for(int oy=-presetOctreeRadius; oy < presetOctreeRadius; ++oy)
        {
            for(int oz=-presetOctreeRadius; oz < presetOctreeRadius; ++oz)
            {
                for(int x=ox * octreeSize; x < (ox + 1) * octreeSize; ++x)
                {
                    for(int y=oy * octreeSize; y < (oy + 1) * octreeSize; ++y)
                    {
                        for(int z=oz * octreeSize; z < (oz + 1) * octreeSize; ++z)
                        {
                            float worldX = (float)x / 2.0f;
                            float worldY = (float)y / 2.0f;
                            float worldZ = (float)z / 2.0f;

                            float rotatedX0 = cosa0 * worldX - sina0 * worldZ;
                            float rotatedZ0 = sina0 * worldX + cosa0 * worldZ;
                            float rotatedX1 = cosa1 * worldX - sina1 * worldZ;
                            float rotatedZ1 = sina1 * worldX + cosa1 * worldZ;
                            float rotatedX2 = cosa2 * worldX - sina2 * worldZ;
                            float rotatedZ2 = sina2 * worldX + cosa2 * worldZ;
                            float rotatedX3 = cosa3 * worldX - sina3 * worldZ;
                            float rotatedZ3 = sina3 * worldX + cosa3 * worldZ;

                            float density = -worldY;


                            XMFLOAT3 warp(noise[0].SampleLinear(worldX, worldY, worldZ, 0.004f, 8.0f), 
                                noise[1].SampleLinear(worldX, worldY, worldZ, 0.004f, 16.0f), 
                                noise[2].SampleLinear(worldX, worldY, worldZ, 0.004f, 8.0f));
                            worldX += warp.x;
                            worldY += warp.y;
                            worldZ += warp.z;

                            //density += noise[0].SampleLinear(worldX, worldY, worldZ, 4.03f, 0.25f);
                            density += noise[1].SampleLinear(worldX, worldY, worldZ, 1.96f, 0.50f);
                            density += noise[2].SampleLinear(worldX, worldY, worldZ, 1.01f, 1.00f);
                            density += noise[0].SampleLinear(rotatedX0, worldY, rotatedZ0, 0.53f, 2.00f);
                            density += noise[0].SampleLinear(worldX, worldY, worldZ, 0.23f, 4.00f);
                            density += noise[1].SampleLinear(rotatedX1, worldY, rotatedZ1, 0.126f, 8.00f);
                            density += noise[2].SampleLinear(worldX, worldY, worldZ, 0.0624f, 16.00f);
                            density += noise[0].SampleLinear(rotatedX2, worldY, rotatedZ2, 0.03137f, 32.00f);
                            density += noise[1].SampleLinear(worldX, worldY, worldZ, 0.015625f, 64.00f);
                            density += noise[2].SampleLinear(rotatedX3, worldY, rotatedZ3, 0.0078125f, 128.00f);

                            //this->SetDensity(x, y, z, density, false);
                            //int densityToStore = (short)(CLAMP(density, -1.0f, 1.0f) * (float)SHORT_MAX);
                            //tree.SetValue(x, y, z, densityToStore, true);
                            int material = worldY > 0 ? 0 : 1;
                            this->SetRawValue(x, y, z, this->Implode(density, material), false);

                            float percentage = 100.0f * (float)++i / (float)wholesize;
                            if(LostIsland::g_pTimer->Tock(outID, KEEPRUNNING) > 5000)
                            {
                                LostIsland::g_pTimer->Tock(outID, RESET);
                                float elapsed = (float)LostIsland::g_pTimer->Tock(timerID, KEEPRUNNING);
                                std::ostringstream stats;
                                float minutes = (1e-3f * (100.0f * elapsed / percentage - elapsed) / 60.0f);
                                float seconds = 60.0f * (minutes - floor(minutes));
                                stats << "Terrain Generation: " << ((float)(int)(100.0f * percentage) / 100.0f) << "%. " << floor(minutes) << " minutes and " << floor(seconds) << " seconds remaining\n";
                                OutputDebugStringA(stats.str().c_str());
                            }
                        }
                    }
                }
            }
        }
    }
    
    this->SaveAllData();
}


void TerrainData::PrintOctFileContents(std::string p_filename) const
{
    std::fstream str;
    str.open(p_filename, std::ios::in | std::ios::binary);
    if(str.is_open())
    {
        unsigned short size;
        str.read((char*)&size, sizeof(unsigned short));
        std::cout << "octree size: " << size << std::endl;

        while(str.good())
        {
            static char value;
            static char flags;
            static int pSons[8];
            static int currentPos;
            currentPos = (int)str.tellg();
            str.read(&value, sizeof(char));
            str.read(&flags, sizeof(char));
            std::cout << currentPos << ": value: " << (int)value << ", flags: " << (int)flags;
            str.read((char*)pSons, sizeof(int));
            if(pSons[0] != 0)
            {
                str.read((char*)(pSons + 1), 7 * sizeof(int));
                std::cout << ", sons: " << pSons[0] << " " << pSons[1] << " " << pSons[2] << " " << pSons[3] << " " << pSons[4] << " " << pSons[5] << " " << pSons[6] << " " << pSons[7] << std::endl;
            }
            else
            {
                std::cout << std::endl;
            }
        }

    }
    else
    {
        std::cout << "error opening file " << p_filename << std::endl;
    }
}
