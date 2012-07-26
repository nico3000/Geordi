#include "StdAfx.h"
#include "TerrainData.h"

#define OCTREE_LOADED 1
#define TILE(_x, _y, _z) m_pData[(_z) * m_pGridSize[0] * m_pGridSize[1] + (_y) * m_pGridSize[0] + (_x)]
#define LAST_USED(_x, _y, _z) m_pLastUsed[(_z) * m_pGridSize[0] * m_pGridSize[1] + (_y) * m_pGridSize[0] + (_x)]

TerrainData::TerrainData(std::string p_octreeFolder):
    m_pData(NULL), m_pLastUsed(NULL), m_activeOctrees(0), m_octreeFolder(p_octreeFolder)
{
}


TerrainData::~TerrainData(void)
{
    this->SaveAllTiles();
    SAFE_DELETE_ARRAY(m_pData);
    SAFE_DELETE_ARRAY(m_pLastUsed);
}


bool TerrainData::Init(unsigned short p_octreeSize, unsigned short p_gridSizeX, unsigned short p_gridSizeY, unsigned short p_gridSizeZ, unsigned short p_maxActiveOctrees)
{
    bool result = Octree::InitMemoryPool(p_octreeSize * p_octreeSize); // TODO: educated guess :P
    if(!result)
    {
        return false;
    }
    else
    {
        m_octreeSize = p_octreeSize;
        m_pGridSize[0] = p_gridSizeX;
        m_pGridSize[1] = p_gridSizeY;
        m_pGridSize[2] = p_gridSizeZ;
        m_maxActiveOctrees = p_maxActiveOctrees;
        m_pData = new Octree[p_gridSizeX * p_gridSizeY * p_gridSizeZ];
        m_pLastUsed = new LONGLONG[p_gridSizeX * p_gridSizeY * p_gridSizeZ];
        ZeroMemory(m_pData, p_gridSizeX * p_gridSizeY * p_gridSizeZ * sizeof(Octree));
        ZeroMemory(m_pLastUsed, p_gridSizeX * p_gridSizeY * p_gridSizeZ * sizeof(LONGLONG));
        return true;
    }
}


void TerrainData::UseTile(int tileX, int tileY, int tileZ)
{
    if(!this->IsTileActive(tileX, tileY, tileZ))
    {
        while(m_activeOctrees >= m_maxActiveOctrees)
        {
            int minX = -1, minY = -1, minZ = -1;
            for(int x=0; x < m_pGridSize[0]; ++x)
            {
                for(int y=0; y < m_pGridSize[1]; ++y)
                {
                    for(int z=0; z < m_pGridSize[2]; ++z)
                    {
                        if(this->IsTileActive(x, y, z) && (minX == -1 || LAST_USED(x, y, z) < LAST_USED(minX, minY, minZ)))
                        {
                            minX = x;
                            minY = y;
                            minZ = z;
                        }
                    }
                }
            }
            this->UnuseTile(minX, minY, minZ);
        }
        this->LoadTileFromDisk(tileX, tileY, tileZ);
        TILE(tileX, tileY, tileZ).GetFlags() |= OCTREE_LOADED;
        ++m_activeOctrees;
        //std::cout << std::endl << "use" << std::endl;
    }
    static LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    LAST_USED(tileX, tileY, tileZ) = now.QuadPart;
}


void TerrainData::UnuseTile(int tileX, int tileY, int tileZ)
{
    if(this->IsTileActive(tileX, tileY, tileZ))
    {
        //std::cout << std::endl << "unuse" << std::endl;
        this->SaveTileToDisk(tileX, tileY, tileZ);
        TILE(tileX, tileY, tileZ).Clear();
        --m_activeOctrees;
    }
}


bool TerrainData::IsTileActive(int tileX, int tileY, int tileZ) const
{
    return (TILE(tileX, tileY, tileZ).ReadFlag() & OCTREE_LOADED) == OCTREE_LOADED;
}


void TerrainData::Test(void)
{
    this->GenerateTestData();
    this->SaveAllTiles();

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


void TerrainData::SaveAllTiles(void) const
{
    for(int x=0; x < m_pGridSize[0]; ++x)
    {
        for(int y=0; y < m_pGridSize[1]; y++)
        {
            for(int z=0; z < m_pGridSize[2]; ++z)
            {
                this->SaveTileToDisk(x, y, z);
            }
            
        }
    }
}


void TerrainData::SaveTileToDisk(int tileX, int tileY, int tileZ) const
{
    if(this->IsTileActive(tileX, tileY, tileZ))
    {
        std::stringstream strStream;
        strStream << "./" << m_octreeFolder << "/terrain." << tileX << "." << tileY << "." << tileZ << ".oct";
        std::fstream fileStream;
        fileStream.open(strStream.str().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        if(fileStream.is_open())
        {
            //m_ppData[tileX][tileY].PrintUsage();
            //std::cout << std::endl;
            TILE(tileX, tileY, tileZ).Save(fileStream);
            fileStream.close();
        }
        else
        {
            LI_ERROR("could not open file " + strStream.str() + " for saving");
        }
    }
}


void TerrainData::LoadTileFromDisk(int tileX, int tileY, int tileZ)
{
    std::stringstream strStream;
    strStream << "./" << m_octreeFolder << "/terrain." << tileX << "." << tileY << "." << tileZ << ".oct";
    std::fstream fileStream;
    fileStream.open(strStream.str().c_str(), std::ios::in | std::ios::binary);
    if(fileStream.is_open())
    {
        TILE(tileX, tileY, tileZ).Init(fileStream);
        fileStream.close();
    }
    else
    {
        TILE(tileX, tileY, tileZ).Init(m_octreeSize);
        //std::cout << "could not open file " << strStream.str() << " for loading, creating new one" << std::endl;
    }
    
}


short TerrainData::Density2Value(float p_density)
{
    //return p_density >= 0 ? 1 : -1;
    return (short)(CLAMP(p_density, -1.0f, 1.0f) * (float)SHORT_MAX);
}


float TerrainData::Value2Density(short p_value)
{
    //return p_value >= 0 ? 1.0f : -1.0f;
    return (float)CLAMP(p_value, -SHORT_MAX, SHORT_MAX) / (float)SHORT_MAX;
}


void TerrainData::SetDensity(int x, int y, int z, float p_density, bool p_autoOptimizeStructure /* = true */)
{
    int tileX = x / m_octreeSize;
    int tileY = y / m_octreeSize;
    int tileZ = z / m_octreeSize;
    int octreeX = x % m_octreeSize;
    int octreeY = y % m_octreeSize;
    int octreeZ = z % m_octreeSize;
    if(tileX < m_pGridSize[0] && tileY < m_pGridSize[1] && tileZ < m_pGridSize[2])
    {
        this->UseTile(tileX, tileY, tileZ);
        TILE(tileX, tileY, tileZ).SetValue(octreeX, octreeY, octreeZ, TerrainData::Density2Value(p_density), p_autoOptimizeStructure);
    }
}


float TerrainData::GetDensity(int x, int y, int z)
{
    int tileX = x / m_octreeSize;
    int tileY = y / m_octreeSize;
    int tileZ = z / m_octreeSize;
    int octreeX = x % m_octreeSize;
    int octreeY = y % m_octreeSize;
    int octreeZ = z % m_octreeSize;
    if(tileX < m_pGridSize[0] && tileY < m_pGridSize[1] && tileZ < m_pGridSize[2])
    {
        this->UseTile(tileX, tileY, tileZ);
        short val = TILE(tileX, tileY, tileZ).GetValue(octreeX, octreeY, octreeZ);
        return TerrainData::Value2Density(val);
    }
    else
    {
        return 0.0f;
    }
}


float TerrainData::GetDensityLinear(float p_x, float p_y, float p_z)
{
    float r = p_x - floor(p_x);
    float s = p_y - floor(p_y);
    float t = p_z - floor(p_z);
    int sampleX = (int)floor(p_x) % (m_pGridSize[0] * m_octreeSize);
    int sampleY = (int)floor(p_y) % (m_pGridSize[1] * m_octreeSize);
    int sampleZ = (int)floor(p_z) % (m_pGridSize[2] * m_octreeSize);

    float y0 = MIX(this->GetDensity(sampleX,     sampleY, sampleZ),     this->GetDensity(sampleX,     sampleY + 1, sampleZ),     s);
    float y1 = MIX(this->GetDensity(sampleX + 1, sampleY, sampleZ),     this->GetDensity(sampleX + 1, sampleY + 1, sampleZ),     s);
    float y2 = MIX(this->GetDensity(sampleX + 1, sampleY, sampleZ + 1), this->GetDensity(sampleX + 1, sampleY + 1, sampleZ + 1), s);
    float y3 = MIX(this->GetDensity(sampleX,     sampleY, sampleZ + 1), this->GetDensity(sampleX,     sampleY + 1, sampleZ + 1), s);

    float x0 = MIX(y0, y1, r);
    float x1 = MIX(y3, y2, r);

    return MIX(x0, x1, t);
}


void TerrainData::GenerateTestData(void)
{
    //m_pData->SetValue(0,0,0,1);
    //m_pData->SetValue(1,1,1,2);
    //m_pData->SetValue(2,2,2,3);
    //m_pData->SetValue(3,3,3,4);
    //return;

    
    Grid3D noise;
    noise.Init(32);
    noise.LoadNoise();

    for(int gridX=0; gridX < m_pGridSize[0]; ++gridX)
    {
        for(int gridY=0; gridY < m_pGridSize[1]; ++gridY)
        {
            for(int gridZ=0; gridZ < m_pGridSize[2]; ++gridZ)
            {
                for(int x=0; x < m_octreeSize; ++x) 
                {
                    for(int y=0; y < m_octreeSize; ++y) 
                    {
                        for(int z=0; z < m_octreeSize; ++z) 
                        {
                            float worldX = 2.0f * (float)(gridX * m_octreeSize + x) / (float)(m_pGridSize[0] * m_octreeSize) - 1.0f;
                            float worldY = 2.0f * (float)(gridY * m_octreeSize + y) / (float)(m_pGridSize[1] * m_octreeSize) - 1.0f;
                            float worldZ = 2.0f * (float)(gridZ * m_octreeSize + z) / (float)(m_pGridSize[2] * m_octreeSize) - 1.0f;

                            //float radius = sqrt(worldX * worldX + worldY * worldY + worldZ * worldZ);
                            //float density = 0.75f - radius;
                            //float density = worldY - 0.25f * sin(100.0f * worldX) * cos(10.0f * worldZ + 5.0f * worldY) + 0.4f * sin(10.0f * worldY + worldX);
                            float density = 16.0f * worldY;
                            //density += 1.0f * noise.SampleLinear(noise.GetSize() * worldX, noise.GetSize() * worldY, noise.GetSize() * worldZ);
                            density += 4.1f * noise.SampleLinear(0.24f * noise.GetSize() * worldX, 0.26f * noise.GetSize() * worldY, 0.23f * noise.GetSize() * worldZ);
                            density += 7.9f * noise.SampleLinear(0.127f * noise.GetSize() * worldX, 0.123f * noise.GetSize() * worldY, 0.135f * noise.GetSize() * worldZ);
                            density += 16.1f * noise.SampleLinear(0.0635f * noise.GetSize() * worldX, 0.0725f * noise.GetSize() * worldY, 0.0615f * noise.GetSize() * worldZ);
                            //density += 0.5f * noise.SampleLinear(2.0f *   noise.GetSize() * worldX, 2.0f *   noise.GetSize() * worldY, 2.0f *   noise.GetSize() * worldZ);
                            //density *= 16.0f;

                            this->SetDensity(gridX * m_octreeSize + x, gridY * m_octreeSize + y, gridZ * m_octreeSize + z, density, false);
                        }
                    }
                }
                std::ostringstream str;
                str << "optimizing tile [" << gridX << "," << gridY << "," << gridZ << "] of [" << m_pGridSize[0] << "," << m_pGridSize[1] << "," << m_pGridSize[2] << "]";
                LI_INFO(str.str());
                this->UseTile(gridX, gridY, gridZ);
                TILE(gridX, gridY, gridZ).OptimizeStructure();

                OutputDebugStringA(str.str().c_str());
            }
        }
    }

//     const static unsigned char size = 248;
//     const static unsigned char images = 22;
//     for(int z=0; z < size / 2; ++z)
//     {
//         int image1 = images * z / (size / 2);
//         int image2 = image1 + 1;
//         float t = (float)images * (float)z / (0.5f * (float)size) - (float)image1;
//         std::ostringstream filename1, filename2;
//         filename1 << "D:\\Visual Studio 2010\\Geordi\\Assets\\kopf\\" << (image1 < 10 ? "0" : "") << image1 << ".BMP.n";
//         filename2 << "D:\\Visual Studio 2010\\Geordi\\Assets\\kopf\\" << (image2 < 10 ? "0" : "") << image2 << ".BMP.n";
//         std::ifstream file1(filename1.str(), std::ios::binary | std::ios::in);
//         std::ifstream file2(filename2.str(), std::ios::binary | std::ios::in);
//         if(!file1.is_open() || !file1.good())
//         {
//             LI_ERROR("error: " + filename1.str());
//         }
//         if(!file2.is_open() || !file2.good())
//         {
//             LI_ERROR("error: " + filename2.str());
//         }
// 
//         if((unsigned char)file1.get() != size || (unsigned char)file1.get() != size)
//         {
//             std::ostringstream error;
//             error << filename1.str() << " wrong size!"; 
//             LI_ERROR(error.str());
//         }
//         if((unsigned char)file2.get() != size || (unsigned char)file2.get() != size)
//         {
//             std::ostringstream error;
//             error << filename2.str() << " wrong size!"; 
//             LI_ERROR(error.str());
//         }
//         
//         for(int y=0; y < size; ++y)
//         {
//             for(int x=0; x < size; ++x)
//             {
//                 float red1 = (float)(unsigned char)file1.get() / 255.0f - 0.15f;
//                 float red2 = (float)(unsigned char)file2.get() / 255.0f - 0.15f;
//                 file1.get(); file1.get(); file1.get();
//                 file2.get(); file2.get(); file2.get();
//                 this->SetDensity(x, size - 1 - y, z, -MIX(red1, red2, t));
//                 std::ostringstream info;
//                 info << red1 << " " << red2 << " " << t;
//                 //LI_INFO(info.str());
//             }
//         }
// 
//         std::stringstream info;
//         info << z << "/" << (int)size << " (" << 0.1f * (float)(int)(1e+3f * (float)z / (float)size) << "%)";
//         LI_INFO(info.str());
//     }

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


bool TerrainData::FillGrid(Grid3D& p_grid, unsigned short p_startX, unsigned short p_startY, unsigned short p_startZ, unsigned short p_offset /* = 1 */)
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