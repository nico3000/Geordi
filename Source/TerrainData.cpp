#include "StdAfx.h"
#include "TerrainData.h"

#define HEIGHT_MASK 0x0000ffff
#define MATERIAL_MASK 0x000f0000
#define MAX_ACTIVE_OCTREES_PER_LEVEL 8

//////////////////////////////////////////////////////////////////////////
//                              TerrainData                             //
//////////////////////////////////////////////////////////////////////////
TerrainData::TerrainData(void)
{

}


TerrainData::~TerrainData(void)
{
    this->SaveAllData();
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


bool TerrainData::Init(std::string p_terrainFolder, unsigned char p_levels, unsigned short p_octreeSize)
{
    bool success = true;
    if(!Octree::InitMemoryPool(MAX_ACTIVE_OCTREES_PER_LEVEL * p_octreeSize * p_octreeSize)) // TODO: educated guess :P
    {
        LI_ERROR("Failed to initialize memory pool for octrees");
    }
    for(unsigned char level=0; success && level < p_levels; ++level)
    {
        m_levels.push_back(LevelData());
        if(!m_levels.back().Init(p_terrainFolder, level, p_octreeSize))
        {
            std::ostringstream info;
            info << "Failed to initialize level " << level;
            LI_ERROR(info.str());
            success = false;
        }
    }
    return success;
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


//////////////////////////////////////////////////////////////////////////
//                               LevelData                              //
//////////////////////////////////////////////////////////////////////////

LevelData::LevelData(void)
{

}


LevelData::~LevelData(void)
{
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        this->SaveOctree(*iter);
        SAFE_DELETE(*iter);
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
    if(!m_loadedTiles.empty() && m_loadedTiles.back()->IsIn(p_x, p_y, p_z))
    {
        return;
    }
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        if((*iter)->IsIn(p_x, p_y, p_z))
        {
            Octree* pTree = *iter;
            m_loadedTiles.erase(iter);
            m_loadedTiles.push_back(pTree);
            return;
        }
    }
    while(m_loadedTiles.size() > MAX_ACTIVE_OCTREES_PER_LEVEL)
    {
        if(!this->SaveOctree(*m_loadedTiles.begin()))
        {
            LI_ERROR("Saving octree failed");
        }
        m_loadedTiles.front()->Clear();
        SAFE_DELETE(*m_loadedTiles.begin());
        m_loadedTiles.pop_front();
    }
    this->LoadOctree(p_x, p_y, p_z);
}


bool LevelData::SaveOctree(Octree* pTree) const
{
    bool success = false;
    if(pTree)
    {
        pTree->OptimizeStructure();
        if(!pTree->IsEmpty())
        {
            std::stringstream octreeStream;
            octreeStream << "./" << m_terrainFolder << "/terrain." << m_level << "." << pTree->GetMinX() << "." << pTree->GetMinY() << "." << pTree->GetMinZ() << ".oct";
            std::fstream fileStream(octreeStream.str().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
            if(fileStream.is_open())
            {
                if(!pTree->Save(fileStream))
                {
                    LI_ERROR("Failed to save octree to stream: " + octreeStream.str());
                }
                else
                {
                    success = true;
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
            success = true;
            //LI_INFO("Saving of empty octree skipped");
        }
    }
    else
    {
        LI_ERROR("Trying to save empty octree tile");
    }
    return success;
}


bool LevelData::LoadOctree(int p_x, int p_y, int p_z)
{
    int tileStartX = (int)floor((float)p_x / (float)m_octreeSize) * m_octreeSize;
    int tileStartY = (int)floor((float)p_y / (float)m_octreeSize) * m_octreeSize;
    int tileStartZ = (int)floor((float)p_z / (float)m_octreeSize) * m_octreeSize;

    Octree* pTree = new Octree;

    bool success = true;
    std::stringstream octreeStream;
    octreeStream << "./" << m_terrainFolder << "/terrain." << m_level << "." << tileStartX << "." << tileStartY << "." << tileStartZ << ".oct";
    std::fstream fileStream(octreeStream.str().c_str(), std::ios::in | std::ios::binary);
    if(fileStream.is_open())
    {
        if(!pTree->Init(fileStream))
        {
            LI_ERROR("Failed to load octree from stream: " + octreeStream.str());
            success = false;
        }
        fileStream.close();
    }
    else
    {
        pTree->Init(tileStartX, tileStartY, tileStartZ, m_octreeSize);
    }

    if(success)
    {
        m_loadedTiles.push_back(pTree);
    }
    return success;
}


int LevelData::GetRawValue(int p_x, int p_y, int p_z)
{
    this->PushTileToBack(p_x, p_y, p_z);
    return m_loadedTiles.back()->GetValue(p_x, p_y, p_z);
}


void LevelData::SetRawValue(int p_x, int p_y, int p_z, int p_value, bool p_autoOptimizeStructure /* = true */)
{
    this->PushTileToBack(p_x, p_y, p_z);
    m_loadedTiles.back()->SetValue(p_x, p_y, p_z, p_value, p_autoOptimizeStructure);
}


void LevelData::SaveAllOctrees(void) const
{
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        this->SaveOctree(*iter);
    }
}


void LevelData::OptimizeAllOctrees(void) const
{
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        (*iter)->OptimizeStructure();
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
    static const int presetSize = 256;
    
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
    ULONGLONG wholesize = 2 * presetSize;
    wholesize = wholesize * wholesize * wholesize;
    
    for(int x=-presetSize; x < presetSize; ++x)
    {
        for(int y=-presetSize; y < presetSize; ++y)
        {
            for(int z=-presetSize; z < presetSize; ++z)
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
                            

//                             XMFLOAT3 warp(noise[0].SampleLinear(worldX, worldY, worldZ, 0.004f, 8.0f), 
//                                 noise[1].SampleLinear(worldX, worldY, worldZ, 0.004f, 16.0f), 
//                                 noise[2].SampleLinear(worldX, worldY, worldZ, 0.004f, 8.0f));
//                             worldX += warp.x;
//                             worldY += warp.y;
//                             worldZ += warp.z;

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
//                             if((int)(percentage) > lastPercentage)
//                             {
//                                 lastPercentage = (int)percentage;
//                                 std::ostringstream percentageStream;
//                                 percentageStream << (int)(percentage) << "%\n";
//                                 OutputDebugStringA(percentageStream.str().c_str());
//                             }
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
