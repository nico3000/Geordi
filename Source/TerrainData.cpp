#include "StdAfx.h"
#include "TerrainData.h"
#include <iostream>
#include <fstream>

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


bool TerrainData::Init(USHORT p_octreeSize, USHORT p_gridSizeX, USHORT p_gridSizeY, USHORT p_gridSizeZ, USHORT p_maxActiveOctrees)
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
        this->SetDimension(-0.5f * METRES_PER_UNIT * (float)(p_gridSizeX * p_octreeSize),
                           -0.5f * METRES_PER_UNIT * (float)(p_gridSizeY * p_octreeSize),
                           -0.5f * METRES_PER_UNIT * (float)(p_gridSizeZ * p_octreeSize),
                           +0.5f * METRES_PER_UNIT * (float)(p_gridSizeX * p_octreeSize),
                           +0.5f * METRES_PER_UNIT * (float)(p_gridSizeY * p_octreeSize),
                           +0.5f * METRES_PER_UNIT * (float)(p_gridSizeZ * p_octreeSize));
        return true;
    }
}


void TerrainData::UseTile(INT tileX, INT tileY, INT tileZ)
{
    if(!this->IsTileActive(tileX, tileY, tileZ))
    {
        while(m_activeOctrees >= m_maxActiveOctrees)
        {
            INT minX = -1, minY = -1, minZ = -1;
            for(INT x=0; x < m_pGridSize[0]; ++x)
            {
                for(INT y=0; y < m_pGridSize[1]; ++y)
                {
                    for(INT z=0; z < m_pGridSize[2]; ++z)
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


void TerrainData::UnuseTile(INT tileX, INT tileY, INT tileZ)
{
    if(this->IsTileActive(tileX, tileY, tileZ))
    {
        //std::cout << std::endl << "unuse" << std::endl;
        this->SaveTileToDisk(tileX, tileY, tileZ);
        TILE(tileX, tileY, tileZ).Clear();
        --m_activeOctrees;
    }
}


bool TerrainData::IsTileActive(INT tileX, INT tileY, INT tileZ) const
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
    //INT size = 256;
    //tree.Init(size);
    //for(INT x=0; x < size; ++x)
    //{
    //    for(INT y=0; y < size; ++y)
    //    {
    //        for(INT z=0; z < size; ++z)
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
    for(INT x=0; x < m_pGridSize[0]; ++x)
    {
        for(INT y=0; y < m_pGridSize[1]; y++)
        {
            for(INT z=0; z < m_pGridSize[2]; ++z)
            {
                this->SaveTileToDisk(x, y, z);
            }
            
        }
    }
}


void TerrainData::SaveTileToDisk(INT tileX, INT tileY, INT tileZ) const
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


void TerrainData::LoadTileFromDisk(INT tileX, INT tileY, INT tileZ)
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


void TerrainData::SetDimension(float p_minX, float p_minY, float p_minZ, float p_maxX, float p_maxY, float p_maxZ)
{
    m_minX = p_minX;
    m_minY = p_minY;
    m_minZ = p_minZ;
    m_maxX = p_maxX;
    m_maxY = p_maxY;
    m_maxZ = p_maxZ;
}


char TerrainData::Density2Value(float p_density) const
{
    p_density = 1.0f + CLAMP(p_density, -1.0f, +1.0f);
    return (char)MIX((float)CHAR_MIN, (float)CHAR_MAX, 0.5f*p_density);
}


float TerrainData::Value2Density(SHORT p_value) const
{
    return 2.0f * LERP(p_value, (float)CHAR_MIN, (float)CHAR_MAX) - 1.0f;
}


void TerrainData::SetDensity(INT x, INT y, INT z, float p_density)
{
    INT tileX = x / m_octreeSize;
    INT tileY = y / m_octreeSize;
    INT tileZ = z / m_octreeSize;
    INT octreeX = x % m_octreeSize;
    INT octreeY = y % m_octreeSize;
    INT octreeZ = z % m_octreeSize;
    if(tileX < m_pGridSize[0] && tileY < m_pGridSize[1] && tileZ < m_pGridSize[2])
    {
        this->UseTile(tileX, tileY, tileZ);
        TILE(tileX, tileY, tileZ).SetValue(octreeX, octreeY, octreeZ, this->Density2Value(p_density));
    }
}


float TerrainData::GetDensity(INT x, INT y, INT z)
{
    INT tileX = x / m_octreeSize;
    INT tileY = y / m_octreeSize;
    INT tileZ = z / m_octreeSize;
    INT octreeX = x % m_octreeSize;
    INT octreeY = y % m_octreeSize;
    INT octreeZ = z % m_octreeSize;
    if(tileX < m_pGridSize[0] && tileY < m_pGridSize[1] && tileZ < m_pGridSize[2])
    {
        this->UseTile(tileX, tileY, tileZ);
        char val = TILE(tileX, tileY, tileZ).GetValue(octreeX, octreeY, octreeZ);
        return this->Value2Density(val);
    }
    else
    {
        return 0.0f;
    }
}


void TerrainData::Grid2World(INT p_gridX, INT p_gridY, INT p_gridZ, float& p_worldX, float& p_worldY, float& p_worldZ) const
{
    p_worldX = m_minX + (m_maxX - m_minX) * LERP((float)p_gridX, 0.0f, (float)(m_pGridSize[0] * m_octreeSize));
    p_worldY = m_minY + (m_maxY - m_minY) * LERP((float)p_gridY, 0.0f, (float)(m_pGridSize[1] * m_octreeSize));
    p_worldZ = m_minZ + (m_maxZ - m_minZ) * LERP((float)p_gridZ, 0.0f, (float)(m_pGridSize[2] * m_octreeSize));
}


void TerrainData::World2Grid(float p_worldX, float p_worldY, float p_worldZ, INT& p_gridX, INT& p_gridY, INT& p_gridZ) const
{
    p_gridX = (INT)((float)(m_pGridSize[0] * m_octreeSize) * LERP(p_worldX, m_minX, m_maxX));
    p_gridY = (INT)((float)(m_pGridSize[1] * m_octreeSize) * LERP(p_worldY, m_minY, m_maxY));
    p_gridZ = (INT)((float)(m_pGridSize[2] * m_octreeSize) * LERP(p_worldZ, m_minZ, m_maxZ));
}


void TerrainData::GenerateTestData(void)
{
    //m_pData->SetValue(0,0,0,1);
    //m_pData->SetValue(1,1,1,2);
    //m_pData->SetValue(2,2,2,3);
    //m_pData->SetValue(3,3,3,4);
    //return;

    for(INT x=0; x < (m_pGridSize[0] * m_octreeSize); ++x) 
    {
        for(INT y=0; y < (m_pGridSize[1] * m_octreeSize); ++y) 
        {
            for(INT z=0; z < (m_pGridSize[2] * m_octreeSize); ++z) 
            {
                float worldX, worldY, worldZ;
                this->Grid2World(x, y, z, worldX, worldY, worldZ);
                
                float density = worldY - sin(worldX) * sin(worldZ);
                this->SetDensity(x, y, z, density);
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
        USHORT size;
        str.read((char*)&size, sizeof(USHORT));
        std::cout << "octree size: " << size << std::endl;

        while(str.good())
        {
            static char value;
            static char flags;
            static INT pSons[8];
            static INT currentPos;
            currentPos = (INT)str.tellg();
            str.read(&value, sizeof(char));
            str.read(&flags, sizeof(char));
            std::cout << currentPos << ": value: " << (INT)value << ", flags: " << (INT)flags;
            str.read((char*)pSons, sizeof(INT));
            if(pSons[0] != 0)
            {
                str.read((char*)(pSons + 1), 7 * sizeof(INT));
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