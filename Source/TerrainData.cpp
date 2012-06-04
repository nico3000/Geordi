#include "StdAfx.h"
#include "TerrainData.h"
#include <iostream>
#include <fstream>

using namespace LostIsland;
#define OCTREE_LOADED 1
#define TEST_DIR "octree"

TerrainData::TerrainData(VOID):
    m_ppData(NULL), m_ppLastUsed(NULL), m_octreeSize(0), m_gridSize(0), m_activeOctrees(0)
{
}


TerrainData::~TerrainData(void)
{
    SAFE_DELETE(m_ppData);
}


BOOL TerrainData::Init(USHORT p_octreeSize, USHORT p_gridSize, USHORT p_maxActiveOctrees)
{
    BOOL result = Octree::InitMemoryPool(p_octreeSize * p_octreeSize); // TODO: educated guess :P
    if(!result)
    {
        return FALSE;
    }
    else
    {
        m_octreeSize = p_octreeSize;
        m_gridSize = p_gridSize;
        m_maxActiveOctrees = p_maxActiveOctrees;
        m_ppLastUsed = new LONGLONG*[p_gridSize];
        m_ppData = new Octree*[p_gridSize];
        for(INT i=0; i < p_gridSize; ++i)
        {
            m_ppData[i] = new Octree[p_gridSize];
            m_ppLastUsed[i] = new LONGLONG[p_gridSize];
            ZeroMemory(m_ppData[i], p_gridSize * sizeof(Octree));
            ZeroMemory(m_ppLastUsed[i], p_gridSize * sizeof(LONGLONG));
        }
        this->SetDimension(-0.5f * UNITS_PER_GRID * (FLOAT)(p_gridSize * p_octreeSize),
                           -0.5f * UNITS_PER_GRID * (FLOAT)p_octreeSize,
                           -0.5f * UNITS_PER_GRID * (FLOAT)(p_gridSize * p_octreeSize),
                           +0.5f * UNITS_PER_GRID * (FLOAT)(p_gridSize * p_octreeSize),
                           +0.5f * UNITS_PER_GRID * (FLOAT)p_octreeSize,
                           +0.5f * UNITS_PER_GRID * (FLOAT)(p_gridSize * p_octreeSize));
        return TRUE;
    }
}


VOID TerrainData::UseTile(INT tileX, INT tileY)
{
    if(m_ppData[tileX][tileY].GetFlags() != OCTREE_LOADED)
    {
        while(m_activeOctrees >= m_maxActiveOctrees)
        {
            INT minX = -1, minY = -1;
            for(INT x=0; x < m_gridSize; ++x)
            {
                for(INT y=0; y < m_gridSize; ++y)
                {
                    if((m_ppData[x][y].GetFlags() & OCTREE_LOADED) == OCTREE_LOADED)
                    {
                        if(minX == -1 || m_ppLastUsed[x][y] < m_ppLastUsed[minX][minY])
                        {
                            minX = x;
                            minY = y;
                        }
                    }
                }
            }
            this->UnuseTile(minX, minY);
        }
        this->LoadTileFromDisk(tileX, tileY);
        m_ppData[tileX][tileY].GetFlags() |= OCTREE_LOADED;
        ++m_activeOctrees;
    }
    static LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    m_ppLastUsed[tileX][tileY] = now.QuadPart;
}


VOID TerrainData::UnuseTile(INT tileX, INT tileY)
{
    if((m_ppData[tileX][tileY].GetFlags() & OCTREE_LOADED) == OCTREE_LOADED)
    {
        this->SaveTileToDisk(tileX, tileY);
        m_ppData[tileX][tileY].Clear();
        m_ppData[tileX][tileY].GetFlags() = 0;
        --m_activeOctrees;
    }
}


VOID TerrainData::Test(VOID)
{
    this->GenerateTestData();
    this->SaveAllTiles();
    //this->PrintOctFileContents("test.oct");

    Octree::GetMemoryPool().PrintInfo();
    //m_pData->PrintUsage();
    //m_pData->PrintTree();
    //m_pData->PrintStructure();
}


VOID TerrainData::SaveAllTiles(VOID) CONST
{
    for(INT x=0; x < m_gridSize; ++x)
    {
        for(INT y=0; y < m_gridSize; y++)
        {
            this->SaveTileToDisk(x, y);
        }
    }
}


VOID TerrainData::SaveTileToDisk(INT x, INT y) CONST
{
    std::stringstream strStream;
    strStream << TEST_DIR << "/r" << x << "." << y << ".oct";
    std::fstream fileStream;
    fileStream.open(strStream.str().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if(fileStream.is_open())
    {
        m_ppData[x][y].Save(fileStream);
        fileStream.close();
    }
    else
    {
        std::cout << "could not open file " << strStream.str() << " for saving" << std::endl;
    }
}


VOID TerrainData::LoadTileFromDisk(INT x, INT y)
{
    std::stringstream strStream;
    strStream << TEST_DIR << "/r" << x << "." << y << ".oct";
    std::fstream fileStream;
    fileStream.open(strStream.str().c_str(), std::ios::in | std::ios::binary);
    if(fileStream.is_open())
    {
        m_ppData[x][y].Init(fileStream);
        fileStream.close();
        
    }
    else
    {
        m_ppData[x][y].Init(m_octreeSize);
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


VOID TerrainData::SetDensity(FLOAT p_worldX, FLOAT p_worldY, FLOAT p_worldZ, FLOAT p_density)
{
    INT x, y, z;
    this->World2Grid(p_worldX, p_worldY, p_worldZ, x, y, z);
    INT gridX = x / m_octreeSize;
    INT gridY = y / m_octreeSize;
    INT octreeX = x % m_octreeSize;
    INT octreeY = y % m_octreeSize;
    this->UseTile(gridX, gridY);
    m_ppData[gridX][gridY].SetValue(octreeX, octreeY, z, this->Density2Value(p_density));
}


VOID TerrainData::Grid2World(INT p_gridX, INT p_gridY, INT p_gridZ, FLOAT& p_worldX, FLOAT& p_worldY, FLOAT& p_worldZ) CONST
{
    p_worldX = m_minX + (m_maxX - m_minX) * LERP((FLOAT)p_gridX, 0.0f, (FLOAT)(m_gridSize * m_octreeSize));
    p_worldY = m_minY + (m_maxY - m_minY) * LERP((FLOAT)p_gridY, 0.0f, (FLOAT)(m_gridSize * m_octreeSize));
    p_worldZ = m_minZ + (m_maxZ - m_minZ) * LERP((FLOAT)p_gridZ, 0.0f, (FLOAT)m_octreeSize);
}


VOID TerrainData::World2Grid(FLOAT p_worldX, FLOAT p_worldY, FLOAT p_worldZ, INT& p_gridX, INT& p_gridY, INT& p_gridZ) CONST
{
    p_gridX = (INT)((FLOAT)(m_gridSize * m_octreeSize) * LERP(p_worldX, m_minX, m_maxX));
    p_gridY = (INT)((FLOAT)(m_gridSize * m_octreeSize) * LERP(p_worldY, m_minY, m_maxY));
    p_gridZ = (INT)((FLOAT)m_octreeSize * LERP(p_worldZ, m_minZ, m_maxZ));
}


VOID TerrainData::GenerateTestData(VOID)
{
    //m_pData->SetValue(0,0,0,1);
    //m_pData->SetValue(1,1,1,2);
    //m_pData->SetValue(2,2,2,3);
    //m_pData->SetValue(3,3,3,4);
    //return;

    std::cout << "generating " << (m_gridSize * m_octreeSize) << "^3 octree..." << std::endl;
    INT id = g_timer.Tick(IMMEDIATE);
    LONG lastOutput = 0;

    ULONGLONG current = 0;
    ULONGLONG target = (ULONGLONG)(m_gridSize * m_octreeSize) * (ULONGLONG)(m_gridSize * m_octreeSize) * (ULONGLONG)(m_gridSize * m_octreeSize);

    FLOAT worldX = m_minX;
    FLOAT dx = (m_maxX - m_minX) / (FLOAT)(m_gridSize * m_octreeSize);
    for(INT x=0; x < (m_gridSize * m_octreeSize); ++x) 
    {
        FLOAT worldY = m_minY;
        FLOAT dy = (m_maxY - m_minY) / (FLOAT)m_octreeSize;
        for(INT y=0; y < m_octreeSize; ++y) 
        {
            FLOAT worldZ = m_minZ;
            FLOAT dz = (m_maxZ - m_minZ) / (FLOAT)(m_gridSize * m_octreeSize);
            for(INT z=0; z < (m_gridSize * m_octreeSize); ++z) 
            {
                FLOAT density = 1.0f;
                this->SetDensity(worldX, worldY, worldZ, density);
                worldZ += dz;

                current++;
                LONG elapsed = g_timer.Tock(id, KEEPRUNNING);
                if(elapsed > lastOutput)
                {
                    lastOutput += 250;
                    DOUBLE percentage = (DOUBLE)current / (DOUBLE)target;
                    DOUBLE progressPerTime = percentage / (DOUBLE)elapsed;
                    INT estimated = (INT)((1.0 - percentage) / progressPerTime);
                    std::cout << '\r' << (1e-4 * (DOUBLE)(INT)(1e+6 * percentage)) << "% (time elapsed / remaining: " << (elapsed / 1000) << "s / " << (estimated / 1000) << "s)        ";
                    std::cout.flush();
                }
            }
            worldY += dy;
        }
        worldX += dx;
    }

    std::cout << std::endl << "generation took " << (1e-3 * (DOUBLE)g_timer.Tock(id, ERASE)) << " secs" << std::endl << std::endl;
}


VOID TerrainData::PrintOctFileContents(string p_filename) CONST
{
    std::fstream str;
    str.open(p_filename, std::ios::in | std::ios::binary);
    if(str.is_open())
    {
        INT size;
        INT numInnerNodes;
        INT numLeafNodes;
        str.read((CHAR*)&size, sizeof(INT));
        str.read((CHAR*)&numInnerNodes, sizeof(INT));
        str.read((CHAR*)&numLeafNodes, sizeof(INT));
        std::cout << "octree size: " << size << ", inner nodes: " << numInnerNodes << ", leafs: " << numLeafNodes << std::endl;

        INT count = 0;
        INT iCount = 0;
        CHAR value;
        CHAR flags;
        INT pPointer[8];
        for(INT i=0; i < numInnerNodes && str.good(); ++i)
        {
            str.read(&value, sizeof(CHAR));
            str.read(&flags, sizeof(CHAR));
            str.read((CHAR*)&pPointer, 8 * sizeof(INT));
            std::cout << "inner node #" << iCount++ << "(" << count++ << "). value: " << (INT)value << ", flags: " << (INT)flags << ", pointers: " << pPointer[0] << " " << pPointer[1] << " " << pPointer[2] << " " << pPointer[3] << " " << pPointer[4] << " " << pPointer[5] << " " << pPointer[6] << " " << pPointer[7] << std::endl;
        }
        INT lCount = 0;
        for(INT i=0; i < numLeafNodes && str.good(); ++i)
        {
            str.read(&value, sizeof(CHAR));
            str.read(&flags, sizeof(CHAR));
            std::cout << "leaf node #" << lCount++ << "(" << count++ << "). value: " << (INT)value << ", flags: " << (INT)flags << std::endl;
        }
        std::cout << "unknown bytes: ";
        while(str.good())
        {
            std::cout << (INT)str.get();
        }
        std::cout << std::endl;
        str.close();
    }
    else
    {
        std::cout << "error opening file " << p_filename << std::endl;
    }
}