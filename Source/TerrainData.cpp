#include "StdAfx.h"
#include "TerrainData.h"


TerrainData::TerrainData(std::string p_octreeFolder):
m_octreeFolder(p_octreeFolder)
{
}


TerrainData::~TerrainData(void)
{
    this->SaveAllOctrees();
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        SAFE_DELETE(*iter);
    }
}


bool TerrainData::Init(unsigned short p_octreeSize, unsigned short p_maxActiveOctrees)
{
    bool result = Octree::InitMemoryPool(p_maxActiveOctrees * p_octreeSize * p_octreeSize); // TODO: educated guess :P
    if(!result)
    {
        return false;
    }
    else
    {
        m_octreeSize = p_octreeSize;
        m_maxActiveOctrees = p_maxActiveOctrees;
        m_fileTileInfo.Init(-512, -512, -512, 1024);
        return true;
    }
}


void TerrainData::PushTileToBack(int p_x, int p_y, int p_z)
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
    while(m_loadedTiles.size() >= m_maxActiveOctrees)
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


bool TerrainData::SaveOctree(Octree* pTree) const
{
    bool success = false;
    if(pTree)
    {
        pTree->OptimizeStructure();
        if(!pTree->IsEmpty())
        {
            std::stringstream octreeStream;
            octreeStream << "./" << m_octreeFolder << "/terrain." << pTree->GetMinX() << "." << pTree->GetMinY() << "." << pTree->GetMinZ() << ".oct";
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


bool TerrainData::LoadOctree(int p_x, int p_y, int p_z)
{
    int tileStartX = (int)floor((float)p_x / (float)m_octreeSize) * m_octreeSize;
    int tileStartY = (int)floor((float)p_y / (float)m_octreeSize) * m_octreeSize;
    int tileStartZ = (int)floor((float)p_z / (float)m_octreeSize) * m_octreeSize;

    Octree* pTree = new Octree;

    bool success = true;
    std::stringstream octreeStream;
    octreeStream << "./" << m_octreeFolder << "/terrain." << tileStartX << "." << tileStartY << "." << tileStartZ << ".oct";
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


void TerrainData::SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure /* = true */)
{
    this->PushTileToBack(p_x, p_y, p_z);
    m_loadedTiles.back()->SetValue(p_x, p_y, p_z, (short)(CLAMP(p_density, -1.0f, 1.0f) * (float)SHORT_MAX), p_autoOptimizeStructure);
}


float TerrainData::GetDensity(int p_x, int p_y, int p_z)
{
    this->PushTileToBack(p_x, p_y, p_z);
    return m_loadedTiles.back()->GetValue(p_x, p_y, p_z) / (float)SHORT_MAX;
}


void TerrainData::SaveAllOctrees(void) const
{
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        this->SaveOctree(*iter);
    }
}


void TerrainData::OptimizeAllOctrees(void) const
{
    for(auto iter=m_loadedTiles.begin(); iter != m_loadedTiles.end(); ++iter)
    {
        (*iter)->OptimizeStructure();
    }
}









void TerrainData::Test(void)
{
    this->GenerateTestData();
    this->SaveAllOctrees();

    //std::fstream str;
    //str.open(TEST_DIR + string("/r0.0.oct"), std::ios::binary | std::ios::in);
    //m_ppData[0][0].Init(str);

    Octree::GetMemoryPool().PrintInfo();
    //m_pData->PrintUsage();
    //m_ppData[0][0].PrintTree();
    //m_pData->PrintStructure();
    

    //Octree tree;
    //int size = 256;
    //tree.Init(size);
    //for(int x=0; x < size; ++x)
    //{
    //    for(int y=0; y < size; ++y)
    //    {
    //        for(int z=0; z < size; ++z)
    //        {
    //            char val = 0;
    //            if(x >= size/2)
    //            {
    //                val |= 1;
    //            }
    //            if(y >= size/2)
    //            {
    //                val |= 2;
    //            }
    //            if(z >= size/2)
    //            {
    //                val |= 4;
    //            }
    //            tree.SetValue(x, y, z, val);
    //        }
    //    }
    //}
    //tree.SetValue(0, 0, 0, 1);
    ////tree.PrintTree();

    //std::fstream str;
    //str.open(TEST_DIR + string("/test.oct"), std::ios::binary | std::ios::out | std::ios::trunc);
    //if(str.is_open())
    //{
    //    tree.Save(str);
    //    str.close();
    //}

    //str.open(TEST_DIR + string("/test.oct"), std::ios::binary | std::ios::in);
    //if(str.is_open())
    //{
    //    tree.Init(str);
    //    str.close();
    //    //tree.PrintTree();
    //}

    //this->PrintOctFileContents(TEST_DIR + string("/test.oct"));
}


// float TerrainData::GetDensityLinear(float p_x, float p_y, float p_z)
// {
//     float r = p_x - floor(p_x);
//     float s = p_y - floor(p_y);
//     float t = p_z - floor(p_z);
//     int sampleX = (int)floor(p_x) % (m_pGridSize[0] * m_octreeSize);
//     int sampleY = (int)floor(p_y) % (m_pGridSize[1] * m_octreeSize);
//     int sampleZ = (int)floor(p_z) % (m_pGridSize[2] * m_octreeSize);
// 
//     float y0 = MIX(this->GetDensity(sampleX,     sampleY, sampleZ),     this->GetDensity(sampleX,     sampleY + 1, sampleZ),     s);
//     float y1 = MIX(this->GetDensity(sampleX + 1, sampleY, sampleZ),     this->GetDensity(sampleX + 1, sampleY + 1, sampleZ),     s);
//     float y2 = MIX(this->GetDensity(sampleX + 1, sampleY, sampleZ + 1), this->GetDensity(sampleX + 1, sampleY + 1, sampleZ + 1), s);
//     float y3 = MIX(this->GetDensity(sampleX,     sampleY, sampleZ + 1), this->GetDensity(sampleX,     sampleY + 1, sampleZ + 1), s);
// 
//     float x0 = MIX(y0, y1, r);
//     float x1 = MIX(y3, y2, r);
// 
//     return MIX(x0, x1, t);
// }


void TerrainData::GenerateTestData(void)
{
    
     Octree octree;
 
//     octree.Init(0, 0, 0, 4);
//     octree.SetValue(0,0,0,1);
//     octree.SetValue(1,1,1,2);
//     octree.SetValue(2,2,2,3);
//     octree.SetValue(3,3,3,4);
//     std::fstream str("./octree_test/test.oct", std::ios::binary | std::ios::out | std::ios::trunc);
//     if(str.is_open())
//     {
//         LI_INFO("offen");
//         if(!octree.Save(str))
//         {
//             LI_ERROR("saving failed");
//         }
//         str.close();
//     }
//     else
//     {
//         LI_ERROR("stream could not be opened");
//     }
    

//     std::fstream str("./octree_test/test.oct", std::ios::binary | std::ios::in);
//     octree.Init(str);
//     std::ostringstream testStream;
//     testStream << octree.GetValue(0,0,0) << " ";
//     testStream << octree.GetValue(1,1,1) << " ";
//     testStream << octree.GetValue(2,2,2) << " ";
//     testStream << octree.GetValue(3,3,3) << " ";
//     OutputDebugStringA(testStream.str().c_str());

//    octree.PrintTree();

//     std::ostringstream info;
//     std::fstream str("./octree_test/test.oct", std::ios::binary | std::ios::in);
//     if(str.is_open())
//     {
//         short x, y, z;
//         unsigned short size;
//         str.read((char*)&x, sizeof(short));
//         str.read((char*)&y, sizeof(short));
//         str.read((char*)&z, sizeof(short));
//         str.read((char*)&size, sizeof(unsigned short));
//         info << "minX=" << x << " minY=" << y << " minZ=" << z << ", size=" << size << std::endl;
//         while(!str.eof())
//         {
//             int val;
//             str.read((char*)&val, sizeof(int));
// 
//             unsigned int index;
//             str.read((char*)&index, sizeof(unsigned int));
// 
//             info << "node: " << val << ", offsets: " << index;
//             if(index != 0)
//             {
//                 for(int i=1; i < 8; ++i)
//                 {
//                     str.read((char*)&index, sizeof(unsigned int));
//                     info << " " << index;
//                 }
//             }
//             info << std::endl;
//         }
//         OutputDebugStringA(info.str().c_str());
//         str.close();
//     }
//     else
//     {
//         LI_ERROR("stream not open");
//     }

     //return;
    
    Grid3D noise[3];
    noise[0].Init(16);
    noise[1].Init(16);
    noise[2].Init(16);
    noise[0].LoadNoise();
    noise[1].LoadNoise();
    noise[2].LoadNoise();
    static const int presetOctreeCount = 4;
    
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
    ULONGLONG wholesize = m_octreeSize * presetOctreeCount;
    wholesize = 2 * wholesize * 2 * wholesize * 2 * wholesize;
    
    Octree tree;
    for(int ox=-presetOctreeCount; ox < presetOctreeCount; ++ox)
    {
        for(int oy=-presetOctreeCount; oy < presetOctreeCount; ++oy)
        {
            for(int oz=-presetOctreeCount; oz < presetOctreeCount; ++oz)
            {
                tree.Clear();
                tree.Init(ox * m_octreeSize, oy * m_octreeSize, oz * m_octreeSize, m_octreeSize);
                for(int x=ox * m_octreeSize; x < (ox + 1) * m_octreeSize; ++x)
                {
                    for(int y=oy * m_octreeSize; y < (oy + 1) * m_octreeSize; ++y)
                    {
                        for(int z=oz * m_octreeSize; z < (oz + 1) * m_octreeSize; ++z)
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

                            float density = worldY;

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
                            int densityToStore = (short)(CLAMP(density, -1.0f, 1.0f) * (float)SHORT_MAX);
                            tree.SetValue(x, y, z, densityToStore, true);

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
                if(!tree.IsEmpty())
                {
                    this->SaveOctree(&tree);
                }
            }
        }
    }
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


bool TerrainData::FillGrid(Grid3D& p_grid, int p_startX, int p_startY, int p_startZ, int p_offset /* = 1 */)
{
    char geo = 0; // 0 undefined, 1 only positive values, 2 only negative values, 3 positive and negative values
    for(int x=0; x < p_grid.GetSize(); ++x)
    {
        for(int y=0; y < p_grid.GetSize(); ++y)
        {
            for(int z=0; z < p_grid.GetSize(); ++z)
            {
                float density = this->GetDensity(p_startX + p_offset * x, p_startY + p_offset * y, p_startZ + p_offset * z);
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
                p_grid.SetValue(x, y, z, density);
            }
        }
    }
    return geo == 3;
}