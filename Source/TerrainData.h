#pragma once
#include "Octree.h"
#include "Grid3D.h"

class LevelData;

class TerrainData
{
    friend class LevelData;

private:
    typedef std::vector<LevelData> LevelList;

    LevelList m_levels;

    void SetRawValue(int p_x, int p_y, int p_z, int p_rawValue, bool p_autoOptimizeStructure);

    static int Implode(float p_density, int p_material);
    static void Explode(int p_value, float& p_density, int& p_material);

public:
    TerrainData(void);
    ~TerrainData(void);

    bool Init(std::string p_terrainFolder, unsigned char p_levels, unsigned short p_octreeSize);
    void SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure = true);
    float GetDensity(int p_x, int p_y, int p_z);
    void SetMaterial(int p_x, int p_y, int p_z, int p_material, bool p_autoOptimizeStructure = true);
    int GetMaterial(int p_x, int p_y, int p_z);
    bool FillGrid(Grid3D& p_weightGrid, Grid3D& p_materialGrid, int p_startX, int p_startY, int p_startZ, int p_level);
    void SaveAllData(void) const;

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;

};


class LevelData
{
    friend class TerrainData;

private:
    typedef std::list<Octree*> OctreeList;

    std::string m_terrainFolder;
    int m_level;
    OctreeList m_loadedTiles;
    //Octree m_fileTileInfo;
    unsigned short m_octreeSize;

    bool Init(std::string p_terrainFolder, int p_level, unsigned short p_octreeSize);
    void PushTileToBack(int p_x, int p_y, int p_z);
    bool SaveOctree(Octree* pTree) const;
    bool LoadOctree(int p_x, int p_y, int p_z);
    int GetRawValue(int p_x, int p_y, int p_z);
    void SetRawValue(int p_x, int p_y, int p_z, int p_value, bool p_autoOptimizeStructure = true);

public:
    LevelData(void);
    ~LevelData(void);

    void SaveAllOctrees(void) const;
    void OptimizeAllOctrees(void) const;
    bool FillGrid(Grid3D& p_weightGrid, Grid3D& p_materialGrid, int p_startX, int p_startY, int p_startZ);

};