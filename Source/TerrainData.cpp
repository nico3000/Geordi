#include "StdAfx.h"
#include "TerrainData.h"
#include <iostream>
#include <fstream>

using namespace LostIsland;
#define OCTREE_LOADED 1
#define TEST_DIR "octree"
#define TILE(_x, _y, _z) m_pData[(_z) * m_pGridSize[0] * m_pGridSize[1] + (_y) * m_pGridSize[0] + (_x)]
#define LAST_USED(_x, _y, _z) m_pLastUsed[(_z) * m_pGridSize[0] * m_pGridSize[1] + (_y) * m_pGridSize[0] + (_x)]

TerrainData::TerrainData(VOID):
    m_pData(NULL), m_pLastUsed(NULL), m_activeOctrees(0)
{
}


TerrainData::~TerrainData(void)
{
    SAFE_DELETE_ARRAY(m_pData);
    SAFE_DELETE_ARRAY(m_pLastUsed);
}


BOOL TerrainData::Init(USHORT p_octreeSize, USHORT p_gridSizeX, USHORT p_gridSizeY, USHORT p_gridSizeZ, USHORT p_maxActiveOctrees)
{
    BOOL result = Octree::InitMemoryPool(p_octreeSize * p_octreeSize); // TODO: educated guess :P
    if(!result)
    {
        return FALSE;
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
        this->SetDimension(-0.5f * METRES_PER_UNIT * (FLOAT)(p_gridSizeX * p_octreeSize),
                           -0.5f * METRES_PER_UNIT * (FLOAT)(p_gridSizeY * p_octreeSize),
                           -0.5f * METRES_PER_UNIT * (FLOAT)(p_gridSizeZ * p_octreeSize),
                           +0.5f * METRES_PER_UNIT * (FLOAT)(p_gridSizeX * p_octreeSize),
                           +0.5f * METRES_PER_UNIT * (FLOAT)(p_gridSizeY * p_octreeSize),
                           +0.5f * METRES_PER_UNIT * (FLOAT)(p_gridSizeZ * p_octreeSize));
        return TRUE;
    }
}


VOID TerrainData::UseTile(INT tileX, INT tileY, INT tileZ)
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


VOID TerrainData::UnuseTile(INT tileX, INT tileY, INT tileZ)
{
    if(this->IsTileActive(tileX, tileY, tileZ))
    {
        //std::cout << std::endl << "unuse" << std::endl;
        this->SaveTileToDisk(tileX, tileY, tileZ);
        TILE(tileX, tileY, tileZ).Clear();
        --m_activeOctrees;
    }
}


BOOL TerrainData::IsTileActive(INT tileX, INT tileY, INT tileZ) CONST
{
    return (TILE(tileX, tileY, tileZ).ReadFlag() & OCTREE_LOADED) == OCTREE_LOADED;
}


VOID TerrainData::Test(VOID)
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
    //            CHAR val = 0;
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


VOID TerrainData::SaveAllTiles(VOID) CONST
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


VOID TerrainData::SaveTileToDisk(INT tileX, INT tileY, INT tileZ) CONST
{
    if(this->IsTileActive(tileX, tileY, tileZ))
    {
        std::stringstream strStream;
        strStream << TEST_DIR << "/terrain." << tileX << "." << tileY << "." << tileZ << ".oct";
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
            std::cout << "could not open file " << strStream.str() << " for saving" << std::endl;
        }
    }
}


VOID TerrainData::LoadTileFromDisk(INT tileX, INT tileY, INT tileZ)
{
    std::stringstream strStream;
    strStream << TEST_DIR << "/r" << tileX << "." << tileY << ".oct";
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


VOID TerrainData::SetDimension(FLOAT p_minX, FLOAT p_minY, FLOAT p_minZ, FLOAT p_maxX, FLOAT p_maxY, FLOAT p_maxZ)
{
    m_minX = p_minX;
    m_minY = p_minY;
    m_minZ = p_minZ;
    m_maxX = p_maxX;
    m_maxY = p_maxY;
    m_maxZ = p_maxZ;
    std::cout << "x: " << m_minX << " - " << m_maxX << std::endl
              << "y: " << m_minY << " - " << m_maxY << std::endl
              << "z: " << m_minZ << " - " << m_maxZ << std::endl;
}


CHAR TerrainData::Density2Value(FLOAT p_density) CONST
{
    p_density = 1.0f + CLAMP(p_density, -1.0f, +1.0f);
    return (CHAR)MIX((FLOAT)CHAR_MIN, (FLOAT)CHAR_MAX, 0.5f*p_density);
}


FLOAT TerrainData::Value2Density(SHORT p_value) CONST
{
    return 2.0f * LERP(p_value, (FLOAT)CHAR_MIN, (FLOAT)CHAR_MAX) - 1.0f;
}


VOID TerrainData::SetDensity(INT x, INT y, INT z, FLOAT p_density)
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


VOID TerrainData::Grid2World(INT p_gridX, INT p_gridY, INT p_gridZ, FLOAT& p_worldX, FLOAT& p_worldY, FLOAT& p_worldZ) CONST
{
    p_worldX = m_minX + (m_maxX - m_minX) * LERP((FLOAT)p_gridX, 0.0f, (FLOAT)(m_pGridSize[0] * m_octreeSize));
    p_worldY = m_minY + (m_maxY - m_minY) * LERP((FLOAT)p_gridY, 0.0f, (FLOAT)(m_pGridSize[1] * m_octreeSize));
    p_worldZ = m_minZ + (m_maxZ - m_minZ) * LERP((FLOAT)p_gridZ, 0.0f, (FLOAT)(m_pGridSize[2] * m_octreeSize));
}


VOID TerrainData::World2Grid(FLOAT p_worldX, FLOAT p_worldY, FLOAT p_worldZ, INT& p_gridX, INT& p_gridY, INT& p_gridZ) CONST
{
    p_gridX = (INT)((FLOAT)(m_pGridSize[0] * m_octreeSize) * LERP(p_worldX, m_minX, m_maxX));
    p_gridY = (INT)((FLOAT)(m_pGridSize[1] * m_octreeSize) * LERP(p_worldY, m_minY, m_maxY));
    p_gridZ = (INT)((FLOAT)(m_pGridSize[2] * m_octreeSize) * LERP(p_worldZ, m_minZ, m_maxZ));
}


VOID TerrainData::GenerateTestData(VOID)
{
    //m_pData->SetValue(0,0,0,1);
    //m_pData->SetValue(1,1,1,2);
    //m_pData->SetValue(2,2,2,3);
    //m_pData->SetValue(3,3,3,4);
    //return;

    std::cout << "generating " << (m_pGridSize[0] * m_octreeSize) << "x" << (m_pGridSize[1] * m_octreeSize) << "x" << (m_pGridSize[2] * m_octreeSize) << " octree..." << std::endl;
    INT id = g_pTimer->Tick(IMMEDIATE);

    DOUBLE nextPercentageOutput = 0;
    LONG lastStop = 0;
    ULONGLONG current = 0;
    ULONGLONG target = (ULONGLONG)(m_pGridSize[0] * m_octreeSize) * (m_pGridSize[1] * m_octreeSize) * (m_pGridSize[2] * m_octreeSize);

    for(INT x=0; x < (m_pGridSize[0] * m_octreeSize); ++x) 
    {
        for(INT y=0; y < (m_pGridSize[1] * m_octreeSize); ++y) 
        {
            for(INT z=0; z < (m_pGridSize[2] * m_octreeSize); ++z) 
            {
                FLOAT worldX, worldY, worldZ;
                this->Grid2World(x, y, z, worldX, worldY, worldZ);
                
                FLOAT density = 10.0f * sin(6.282f * LERP(worldX, m_minX, m_maxX)) * cos(6.282f * LERP(worldZ, m_minZ, m_maxZ));
                this->SetDensity(x, y, z, density);

                DOUBLE percentage = (DOUBLE)++current / (DOUBLE)target;
                LONG elapsed = g_pTimer->Tock(id, KEEPRUNNING);
                if(percentage >= nextPercentageOutput || lastStop + 250 < elapsed)
                {
                    lastStop = elapsed;
                    DOUBLE progressPerTime = percentage / (DOUBLE)elapsed;
                    INT estimated = (INT)((1.0 - percentage) / progressPerTime);
                    if(percentage >= nextPercentageOutput)
                    {
                        nextPercentageOutput = min(nextPercentageOutput + 0.01f, 1.0f);
                    }

                    std::wostringstream str;
                    str << L"Terrain generation: ";
                    for(INT i=0; i < 20; ++i)
                    {
                        if((DOUBLE)i / 20.0 < percentage)
                        {
                            str << "*";
                        }
                        else
                        {
                            str << "-";
                        }
                    }
                    str << " " << (1e-4 * (DOUBLE)(INT)(1e+6 * percentage)) << L"% (time elapsed / remaining: " << (elapsed / 1000) << L"s / " << (estimated / 1000) << L"s)";
                    //Logger::ShowStatus(str.str());
                }
            }
        }
    }

    std::cout << std::endl << "generation took " << (1e-3 * (DOUBLE)g_pTimer->Tock(id, ERASE)) << " secs" << std::endl << std::endl;
}


VOID TerrainData::PrintOctFileContents(string p_filename) CONST
{
    std::fstream str;
    str.open(p_filename, std::ios::in | std::ios::binary);
    if(str.is_open())
    {
        USHORT size;
        str.read((CHAR*)&size, sizeof(USHORT));
        std::cout << "octree size: " << size << std::endl;

        while(str.good())
        {
            static CHAR value;
            static CHAR flags;
            static INT pSons[8];
            static INT currentPos;
            currentPos = (INT)str.tellg();
            str.read(&value, sizeof(CHAR));
            str.read(&flags, sizeof(CHAR));
            std::cout << currentPos << ": value: " << (INT)value << ", flags: " << (INT)flags;
            str.read((CHAR*)pSons, sizeof(INT));
            if(pSons[0] != 0)
            {
                str.read((CHAR*)(pSons + 1), 7 * sizeof(INT));
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