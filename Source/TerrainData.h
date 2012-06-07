#pragma once
#include "Octree.h"
#define METRES_PER_UNIT 0.5f

class TerrainData
{
private:
    Octree* m_pData;
    LONGLONG* m_pLastUsed;
    USHORT m_pGridSize[3];
    USHORT m_octreeSize;
    USHORT m_activeOctrees;
    USHORT m_maxActiveOctrees;
    FLOAT m_minX;
    FLOAT m_minY;
    FLOAT m_minZ;
    FLOAT m_maxX;
    FLOAT m_maxY;
    FLOAT m_maxZ;

    CHAR Density2Value(FLOAT p_density) CONST;
    FLOAT Value2Density(SHORT p_value) CONST;
    VOID Grid2World(INT p_gridX, INT p_gridY, INT p_gridZ, FLOAT& p_worldX, FLOAT& p_worldY, FLOAT& p_worldZ) CONST;
    VOID World2Grid(FLOAT p_worldX, FLOAT p_worldY, FLOAT p_worldZ, INT& p_gridX, INT& p_gridY, INT& p_gridZ) CONST;
    VOID UseTile(INT tileX, INT tileY, INT tileZ);
    VOID UnuseTile(INT tileX, INT tileY, INT tileZ);
    BOOL IsTileActive(INT tileX, INT tileY, INT tileZ) CONST;

public:
    TerrainData(VOID);
    ~TerrainData(VOID);

    BOOL Init(USHORT p_octreeSize, USHORT p_gridSizeX, USHORT p_gridSizeY, USHORT p_gridSizeZ, USHORT p_maxActiveOctrees);
    VOID SetDimension(FLOAT p_minX, FLOAT p_minY, FLOAT p_minZ, FLOAT p_maxX, FLOAT p_maxY, FLOAT p_maxZ);
    VOID SetDensity(INT x, INT y, INT z, FLOAT p_density);
    VOID SaveAllTiles(VOID) CONST;
    VOID SaveTileToDisk(INT tileX, INT tileY, INT tileZ) CONST;
    VOID LoadTileFromDisk(INT tileX, INT tileY, INT tileZ);

    VOID Test(VOID);
    VOID GenerateTestData(VOID);
    VOID PrintOctFileContents(string p_filename) CONST;
};

