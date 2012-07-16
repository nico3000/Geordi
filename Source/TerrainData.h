#pragma once
#include "Octree.h"
#define METRES_PER_UNIT 0.5f

class TerrainData
{
private:
    std::string m_octreeFolder;
    Octree* m_pData;
    LONGLONG* m_pLastUsed;
    USHORT m_pGridSize[3];
    USHORT m_octreeSize;
    USHORT m_activeOctrees;
    USHORT m_maxActiveOctrees;
    float m_minX;
    float m_minY;
    float m_minZ;
    float m_maxX;
    float m_maxY;
    float m_maxZ;

    char Density2Value(float p_density) const;
    float Value2Density(SHORT p_value) const;
    void Grid2World(INT p_gridX, INT p_gridY, INT p_gridZ, float& p_worldX, float& p_worldY, float& p_worldZ) const;
    void World2Grid(float p_worldX, float p_worldY, float p_worldZ, INT& p_gridX, INT& p_gridY, INT& p_gridZ) const;
    void UseTile(INT tileX, INT tileY, INT tileZ);
    void UnuseTile(INT tileX, INT tileY, INT tileZ);
    bool IsTileActive(INT tileX, INT tileY, INT tileZ) const;

public:
    TerrainData(std::string p_octreeFolder);
    ~TerrainData(void);

    bool Init(USHORT p_octreeSize, USHORT p_gridSizeX, USHORT p_gridSizeY, USHORT p_gridSizeZ, USHORT p_maxActiveOctrees);
    void SetDimension(float p_minX, float p_minY, float p_minZ, float p_maxX, float p_maxY, float p_maxZ);
    void SetDensity(INT x, INT y, INT z, float p_density);
    float GetDensity(INT x, INT y, INT z);
    void SaveAllTiles(void) const;
    void SaveTileToDisk(INT tileX, INT tileY, INT tileZ) const;
    void LoadTileFromDisk(INT tileX, INT tileY, INT tileZ);

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;
};
