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

    int GetTileForPosition(int p_x, int p_y, int p_z);
	bool SaveOctree(int p_index) const;
	bool LoadOctree(int p_index, int p_tileX, int p_tileY, int p_tileZ);
	void GetTile(int p_x, int p_y, int p_z, int& p_tileX, int& p_tileY, int& p_tileZ) const;

public:
    TerrainData(std::string p_octreeFolder);
    ~TerrainData(void);

    bool Init(unsigned short p_octreeSize, unsigned short p_maxActiveOctrees);
    void SetDensity(int x, int y, int z, float p_density, bool p_autoOptimizeStructure = true);
    float GetDensity(int x, int y, int z);


    float GetDensityLinear(float p_x, float p_y, float p_z);
    void SaveAllTiles(void) const;
    void SaveTileToDisk(int ) const;
    void LoadTileFromDisk(int tileX, int tileY, int tileZ);
    bool FillGrid(Grid3D& p_grid, unsigned short p_startX, unsigned short p_startY, unsigned short p_startZ, unsigned short p_offset = 1);

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;

    static short Density2Value(float p_density);
    static float Value2Density(short p_value);

};
