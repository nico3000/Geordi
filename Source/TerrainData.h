#pragma once
#include "Octree.h"
#include "Grid3D.h"
#include "MarchingCubeGrid.h"

class LevelData;
class TerrainBlock;
class Geometry;

class TerrainData
{
    friend class LevelData;
    friend class TerrainBlock;

public:
    struct BlockInfo
    {
        std::shared_ptr<TerrainBlock> pBlock;
        bool done;
    };
    typedef std::list<std::weak_ptr<Geometry>> GeometryList;
    typedef std::vector<BlockInfo> BlockVector;

private:
    typedef std::vector<LevelData*> LevelList;

    LevelList m_levels;
    std::string m_terrainFolder;
    GeometryList m_chunkList;
    Octree* m_pChunkData;
    unsigned short m_chunksize;
    Grid3D m_weightGrid;
    Grid3D m_materialGrid;
    MarchingCubeGrid m_tempMCGrid;
    float m_scale;
    int m_referenceX, m_referenceY, m_referenceZ;
    bool m_referenceChanged;
    BlockVector m_blockVector;

    void SetRawValue(int p_x, int p_y, int p_z, int p_rawValue, bool p_autoOptimizeStructure);

    static int Implode(float p_density, int p_material);
    static void Explode(int p_value, float& p_density, int& p_material);

public:
    TerrainData(void);
    ~TerrainData(void);

    bool Init(std::string p_terrainFolder, unsigned char p_levels, unsigned short p_octreeSize, int p_chunksize);
    void SetDensity(int p_x, int p_y, int p_z, float p_density, bool p_autoOptimizeStructure = true);
    float GetDensity(int p_x, int p_y, int p_z);
    void SetMaterial(int p_x, int p_y, int p_z, int p_material, bool p_autoOptimizeStructure = true);
    int GetMaterial(int p_x, int p_y, int p_z);
    bool FillGrid(Grid3D& p_weightGrid, Grid3D& p_materialGrid, int p_startX, int p_startY, int p_startZ, int p_level);
    void SaveAllData(void) const;
    void CreateTerrainBlock(int p_x, int p_y, int p_z);
    void SetPointOfReference(float p_worldX, float p_worldY, float p_worldZ);
    void Update(unsigned long m_maxMillis);

    unsigned int GetLevelCount(void) const { return (unsigned int)m_levels.size(); }
    GeometryList& GetGeometryList(void) { return m_chunkList; }

    void Test(void);
    void GenerateTestData(void);
    void PrintOctFileContents(std::string p_filename) const;

};


class LevelData
{
    friend class TerrainData;

private:
    struct LoadedOctree
    {
        Octree* pTree;
        bool changed;
    };

    typedef std::list<LoadedOctree> LoadedOctreeList;

    std::string m_terrainFolder;
    int m_level;
    LoadedOctreeList m_loadedTrees;
    //Octree m_fileTileInfo;
    unsigned short m_octreeSize;

    bool Init(std::string p_terrainFolder, int p_level, unsigned short p_octreeSize);
    void PushTileToBack(int p_x, int p_y, int p_z);
    void DestroyOctree(unsigned int p_index);
    bool SaveOctree(unsigned int p_index) const;
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


class TerrainBlock
{
    friend class TerrainData;

private:
    TerrainData* m_pData;
    int m_x, m_y, m_z;
    int m_level;
    bool m_isRefined;
    int m_flag;
    std::shared_ptr<TerrainBlock> m_pRefined[8];
    std::shared_ptr<Geometry> m_pGeometry;
    std::shared_ptr<Geometry> m_pBackup;
    physx::PxActor* m_pPhysicsActor;

    TerrainBlock(int p_x, int p_y, int p_z, int p_level, TerrainData* p_pData);

    bool BuildGeometry(int p_timerID, unsigned long p_maxMillis);
    void ReleaseGeometry(bool p_releaseChildren);
    bool UseGeometryFromBackup(void);

public:
    ~TerrainBlock(void);

    bool Update(int p_referenceX, int p_referenceY, int p_referenceZ, int p_timerID, unsigned long p_maxMillis);

};