#include "StdAfx.h"
#include "TerrainData.h"


TerrainData::TerrainData(std::string p_octreeFolder):
m_pLoadedTiles(0), m_pLastUsed(0), m_octreeFolder(p_octreeFolder)
{
}


TerrainData::~TerrainData(void)
{
    this->SaveAllOctrees();
    for(int i=0; i < m_maxActiveOctrees; ++i)
    {
        SAFE_DELETE(m_pLoadedTiles[i]);
    }
    SAFE_DELETE(m_pLoadedTiles);
    SAFE_DELETE(m_pLastUsed);
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
        m_pLoadedTiles = new Octree*[m_maxActiveOctrees];
        m_pLastUsed = new LONGLONG[m_maxActiveOctrees];
        m_fileTileInfo.Init(-512, -512, -512, 1024);
        ZeroMemory(m_pLoadedTiles, m_maxActiveOctrees * sizeof(Octree*));
        ZeroMemory(m_pLastUsed, m_maxActiveOctrees * sizeof(LONGLONG));
        return true;
    }
}


int TerrainData::GetTileForPosition(int p_x, int p_y, int p_z) const
{
	int index = -1;
	LONGLONG lastUsed = LONGLONG_MAX;
	bool alienOctree = true;
	for(int i=0; i < m_maxActiveOctrees; ++i)
	{
		if(m_pLoadedTiles[i])
		{
			if(m_pLoadedTiles[i]->IsIn(p_x, p_y, p_z))
			{
				index = i;
				alienOctree = false; // already loaded
				break;
			}
			else if(m_pLastUsed[i] < lastUsed)
			{
                alienOctree = true;
				lastUsed = m_pLastUsed[i];
				index = i;
			}
		}
		else
		{
            alienOctree = false;
			lastUsed = LONGLONG_MIN; // empty slot found
			index = i;
		}
	}

	if(index == -1)
	{
		LI_ERROR("This should not happen!");
	}
	if(alienOctree)
	{
		this->SaveOctree(index);
		m_pLoadedTiles[index]->Clear();
		SAFE_DELETE(m_pLoadedTiles[index]);
	}
	if(!m_pLoadedTiles[index])
	{
		this->LoadOctree(index, p_x, p_y, p_z);
	}

    static LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    m_pLastUsed[index] = now.QuadPart;

	return index;
}


bool TerrainData::SaveOctree(int p_index) const
{
	Octree* pToSave = m_pLoadedTiles[p_index];
	bool success = false;
	if(pToSave)
	{
        pToSave->OptimizeStructure();
        if(!pToSave->IsEmpty())
        {
            std::stringstream octreeStream;
            octreeStream << "./" << m_octreeFolder << "/terrain." << pToSave->GetMinX() << "." << pToSave->GetMinY() << "." << pToSave->GetMinZ() << ".oct";
            std::fstream fileStream(octreeStream.str().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
            if(fileStream.is_open())
            {
                if(!pToSave->Save(fileStream))
                {
                    LI_ERROR("Failed to save octree to stream: " + octreeStream.str());
                }
                else
                {
                    success = true;
                }
                fileStream.close();
            }
            else
            {
                LI_ERROR("Failed to open octree stream for writing: " + octreeStream.str());
            }
        }
        else
        {
            LI_INFO("Saving of empty octree skipped");
        }
	}
	else
	{
		LI_ERROR("Trying to save empty octree tile");
	}
	return success;
}


bool TerrainData::LoadOctree(int p_index, int p_x, int p_y, int p_z) const
{
    int tileStartX = (int)floor((float)p_x / (float)m_octreeSize) * m_octreeSize;
    int tileStartY = (int)floor((float)p_y / (float)m_octreeSize) * m_octreeSize;
    int tileStartZ = (int)floor((float)p_z / (float)m_octreeSize) * m_octreeSize;

    if(!m_pLoadedTiles[p_index])
    {
        m_pLoadedTiles[p_index] = new Octree;
    }

	bool success = true;
	std::stringstream octreeStream;
	octreeStream << "./" << m_octreeFolder << "/terrain." << tileStartX << "." << tileStartY << "." << tileStartZ << ".oct";
	std::fstream fileStream(octreeStream.str().c_str(), std::ios::in | std::ios::binary);
	if(fileStream.is_open())
	{
		if(!m_pLoadedTiles[p_index]->Init(fileStream))
		{
			LI_ERROR("Failed to load octree from stream: " + octreeStream.str());
            success = false;
		}
		fileStream.close();
	}
    else
    {
        m_pLoadedTiles[p_index]->Init(tileStartX, tileStartY, tileStartZ, m_octreeSize);
    }
	return success;
}


void TerrainData::SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure /* = true */) const
{
	int index = this->GetTileForPosition(p_x, p_y, p_z);
	m_pLoadedTiles[index]->SetValue(p_x, p_y, p_z, (short)(CLAMP(p_density, -1.0f, 1.0f) * (float)SHORT_MAX), p_autoOptimizeStructure);
}


float TerrainData::GetDensity(int p_x, int p_y, int p_z) const
{
    int index = this->GetTileForPosition(p_x, p_y, p_z);
    return (float)m_pLoadedTiles[index]->GetValue(p_x, p_y, p_z) / (float)SHORT_MAX;
}


void TerrainData::SaveAllOctrees(void) const
{
    for(int i=0; i < m_maxActiveOctrees; ++i)
    {
        if(m_pLoadedTiles[i])
        {
            this->SaveOctree(i);
        }
    }
}


void TerrainData::OptimizeAllOctrees(void) const
{
    for(int i=0; i < m_maxActiveOctrees; ++i)
    {
        if(m_pLoadedTiles[i])
        {
            this->m_pLoadedTiles[i]->OptimizeStructure();
        }
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
    static const int presetSize = 256;
    int i=0;
    int lastPercentage = -1;

    float cosa = cos(1.0f);
    float sina = sin(1.0f);

    for(int x=-presetSize; x < presetSize; ++x) 
    {
        for(int y=-presetSize; y < presetSize; ++y) 
        {
            for(int z=-presetSize; z < presetSize; ++z) 
            {
                float worldX = (float)x / 2.0f;
                float worldY = (float)y / 2.0f;
                float worldZ = (float)z / 2.0f;
                
                //float radius = sqrt(worldX * worldX + worldY * worldY + worldZ * worldZ);
                //float density = 0.75f - radius;
                //float density = worldY - 0.25f * sin(100.0f * worldX) * cos(10.0f * worldZ + 5.0f * worldY) + 0.4f * sin(10.0f * worldY + worldX);
                float density = worldY;
                //density -= 0.25f * noise.SampleLinear(4.0f * worldX, 4.0f * worldY, 4.0f * worldZ);

                XMFLOAT3 warp(noise[0].SampleLinear(worldX, worldY, worldZ, 0.004f, 8.0f), 
                              noise[1].SampleLinear(worldX, worldY, worldZ, 0.004f, 16.0f), 
                              noise[2].SampleLinear(worldX, worldY, worldZ, 0.004f, 8.0f));
                worldX += warp.x;
                worldY += warp.y;
                //worldZ += warp.z;

                //density += noise[0].SampleLinear(worldX, worldY, worldZ, 4.03f, 0.25f);
                //density += noise[1].SampleLinear(worldX, worldY, worldZ, 1.96f, 0.50f);
                density += noise[2].SampleLinear(worldX, worldY, worldZ, 1.01f, 1.00f);
                density += noise[0].SampleLinear(worldX, worldY, worldZ, 0.53f, 2.00f);
                density += noise[1].SampleLinear(worldX, worldY, worldZ, 0.23f, 4.00f);
                density += noise[2].SampleLinear(worldX, worldY, worldZ, 0.126f, 8.00f);
                density += noise[0].SampleLinear(worldX, worldY, worldZ, 0.0624f, 16.00f);
                density += noise[1].SampleLinear(worldX, worldY, worldZ, 0.03137f, 32.00f);


				//density += noise.SampleLinear(worldX, worldY, worldZ, 0.50f, 1.96f);
                //density += noise.SampleLinear(worldX, worldY, worldZ, 1.00f, 1.01f);
                //density += 4.1f * noise.SampleLinear(0.24f * noise.GetSize() * worldX, 0.26f * noise.GetSize() * worldY, 0.23f * noise.GetSize() * worldZ);
                //density += 7.9f * noise.SampleLinear(0.127f * noise.GetSize() * worldX, 0.123f * noise.GetSize() * worldY, 0.135f * noise.GetSize() * worldZ);
                //density += 16.1f * noise.SampleLinear(0.0635f * noise.GetSize() * worldX, 0.0725f * noise.GetSize() * worldY, 0.0615f * noise.GetSize() * worldZ);
                //density += 0.5f * noise.SampleLinear(2.0f *   noise.GetSize() * worldX, 2.0f *   noise.GetSize() * worldY, 2.0f *   noise.GetSize() * worldZ);
                //density *= 16.0f;

                this->SetDensity(x, y, z, density, false);

                float percentage = 100.0f * (float)++i / (float)(2*presetSize * 2*presetSize * 2*presetSize);
                if((int)(percentage) > lastPercentage)
                {
                    lastPercentage = percentage;
                    std::ostringstream percentageStream;
                    percentageStream << (int)(percentage) << "%\n";
                    OutputDebugStringA(percentageStream.str().c_str());
                }
            }
        }
    }
    this->SaveAllOctrees();
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


bool TerrainData::FillGrid(Grid3D& p_grid, int p_startX, int p_startY, int p_startZ, int p_offset /* = 1 */) const
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