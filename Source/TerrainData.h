#pragma once
#include "Octree.h"
#define UNITS_PER_GRID 2.0f

class TerrainData
{
private:
    Octree** m_ppData;
    LONGLONG** m_ppLastUsed;
    UINT m_activeOctrees;
    USHORT m_octreeSize;
    USHORT m_gridSize;
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
    VOID UseTile(INT tileX, INT tileY);
    VOID UnuseTile(INT tileX, INT tileY);

public:
    TerrainData(VOID);
    ~TerrainData(VOID);

    BOOL Init(USHORT p_octreeSize, USHORT p_gridSize, USHORT p_maxActiveOctrees);
    VOID SetDimension(FLOAT p_minX, FLOAT p_minY, FLOAT p_minZ, FLOAT p_maxX, FLOAT p_maxY, FLOAT p_maxZ);
    VOID SetDensity(FLOAT p_worldX, FLOAT p_worldY, FLOAT p_worldZ, FLOAT p_density);
    VOID SaveAllTiles(VOID) CONST;
    VOID SaveTileToDisk(INT x, INT y) CONST;
    VOID LoadTileFromDisk(INT x, INT y);

    VOID Test(VOID);
    VOID GenerateTestData(VOID);
    VOID PrintOctFileContents(string p_filename) CONST;
};

