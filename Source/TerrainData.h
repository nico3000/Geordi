#pragma once
#include "Octree.h"
#include "Grid3D.h"
#define METRES_PER_UNIT 0.5f

class TerrainData
{
private:
	typedef std::list<Octree*> OctreeList;
    std::string m_octreeFolder;
    Octree m_fileTileInfo;
	Octree** m_pLoadedTiles;
    LONGLONG* m_pLastUsed;
    unsigned short m_octreeSize;
    unsigned short m_maxActiveOctrees;

    int GetTileForPosition(int p_x, int p_y, int p_z) const;
	bool SaveOctree(int p_index) const;
	bool LoadOctree(int p_index, int p_x, int p_y, int p_z) const;

public:
    TerrainData(std::string p_octreeFolder);
    ~TerrainData(void);

    bool Init(unsigned short p_octreeSize, unsigned short p_maxActiveOctrees);
    void SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure = true) const;
    float GetDensity(int p_x, int p_y, int p_z) const;
    void SaveAllOctrees(void) const;
    bool FillGrid(Grid3D& p_grid, int p_startX, int p_startY, int p_startZ, int p_offset = 1) const;
    void OptimizeAllOctrees(void) const;

    //float GetDensityLinear(float p_x, float p_y, float p_z);
    

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;

};
