#pragma once
#include "Octree.h"
#include "Grid3D.h"

class TerrainData
{
private:
    typedef std::list<Octree*> OctreeList;

    OctreeList m_loadedTiles;
    std::string m_octreeFolder;
    Octree m_fileTileInfo;
    unsigned short m_octreeSize;
    unsigned short m_maxActiveOctrees;

    void PushTileToBack(int p_x, int p_y, int p_z);
    bool SaveOctree(Octree* pTree) const;
    bool LoadOctree(int p_x, int p_y, int p_z);
    int GetRawValue(int p_x, int p_y, int p_z);
    void SetRawValue(int p_x, int p_y, int p_z, int p_value, bool p_autoOptimizeStructure = true);
    int Implode(float p_density, int p_material) const;
    void Explode(int p_value, float& p_density, int& p_material) const;

public:
    TerrainData(std::string p_octreeFolder);
    ~TerrainData(void);

    bool Init(unsigned short p_octreeSize, unsigned short p_maxActiveOctrees);
    void SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure = true);
    float GetDensity(int p_x, int p_y, int p_z);
    void SetMaterial(int p_x, int p_y, int p_z, int p_material, bool p_autoOptimizeStructure = true);
    int GetMaterial(int p_x, int p_y, int p_z);
    void SaveAllOctrees(void) const;
    bool FillGrid(Grid3D& p_weightGrid, Grid3D& p_materialGrid, int p_startX, int p_startY, int p_startZ, int p_offset = 1);
    void OptimizeAllOctrees(void) const;

    //float GetDensityLinear(float p_x, float p_y, float p_z);
    

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;

};
