#pragma once
#include "Octree.h"
#include "Grid3D.h"
#define METRES_PER_UNIT 0.5f

class TerrainData
{
private:
    std::string m_octreeFolder;
    Octree* m_pData;
    LONGLONG* m_pLastUsed;
    unsigned short m_pGridSize[3];
    unsigned short m_octreeSize;
    unsigned short m_activeOctrees;
    unsigned short m_maxActiveOctrees;

    void UseTile(int tileX, int tileY, int tileZ);
    void UnuseTile(int tileX, int tileY, int tileZ);
    bool IsTileActive(int tileX, int tileY, int tileZ) const;

public:
    TerrainData(std::string p_octreeFolder);
    ~TerrainData(void);

    bool Init(unsigned short p_octreeSize, unsigned short p_gridSizeX, unsigned short p_gridSizeY, unsigned short p_gridSizeZ, unsigned short p_maxActiveOctrees);
    void SetDensity(int x, int y, int z, float p_density, bool p_autoOptimizeStructure = true);
    float GetDensity(int x, int y, int z);
    float GetDensityLinear(float p_x, float p_y, float p_z);
    void SaveAllTiles(void) const;
    void SaveTileToDisk(int tileX, int tileY, int tileZ) const;
    void LoadTileFromDisk(int tileX, int tileY, int tileZ);
    bool FillGrid(Grid3D& p_grid, unsigned short p_startX, unsigned short p_startY, unsigned short p_startZ, unsigned short p_offset = 1);

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;

    int GetSizeX(void) const { return m_pGridSize[0] * m_octreeSize; }
    int GetSizeY(void) const { return m_pGridSize[1] * m_octreeSize; }
    int GetSizeZ(void) const { return m_pGridSize[2] * m_octreeSize; }

    static short Density2Value(float p_density);
    static float Value2Density(short p_value);

};
