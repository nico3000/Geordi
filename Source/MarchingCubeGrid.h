#pragma once
#include "Grid3D.h"
#include "Geometry.h"


class MarchingCubeGrid
{
public:
    struct TerrainVertex;

private:
    typedef std::vector<TerrainVertex> Vertices;
    typedef std::vector<unsigned int> Indices;
    struct CubeInfo
    {
        unsigned char code;
        unsigned short pEdgeIndices[12];
    };

    static unsigned char sm_pTriangles[256][16];
    const static unsigned char sm_pEdgeTransforms[4][12];
    const static unsigned char sm_pCodeTransforms[4][8];

    Vertices m_vertices;
    Indices m_indices;
    short m_cubes;
    CubeInfo* m_pCubes;

    bool IsIn(short p_x, short p_y, short p_z) const { return 0 <= p_x && p_x < m_cubes && 0 <= p_y && p_y < m_cubes && 0 <= p_z && p_z < m_cubes; }
    CubeInfo& GetCube(short p_x, short p_y, short p_z) { return m_pCubes[m_cubes * m_cubes * p_z + m_cubes * p_y + p_x]; }

    void ResetCubeCodes(short p_cubes);
    void SetBitIfExists(short p_x, short p_y, short p_z, char p_bit);
    void SetBits(short p_x, short p_y, short p_z);
    void AddEdge(char p_edge, unsigned short p_index, short p_x, short p_y, short p_z);
    void AddEdgeIfExists(char p_edge, unsigned short p_index, short p_x, short p_y, short p_z);

    static void ProcessBaseCube(unsigned char p_code, unsigned char* p_pTriangles);
    static void Transform(unsigned char& p_code, unsigned char* p_pTriangles, int p_transformIndex);
    static void CopyTriangles(unsigned char p_code, unsigned char* p_pTriangles, bool mirrored);

public:
    MarchingCubeGrid(void) : m_pCubes(0) {}
    ~MarchingCubeGrid(void) { SAFE_DELETE(m_pCubes); }
      
    std::shared_ptr<Geometry> CreateGeometry(void);
    physx::PxActor* CreatePhysicsActor(void) const;
    bool ConstructData(Grid3D& p_weightGrid, Grid3D& p_materialGrid, const XMFLOAT3& m_position, float p_scale, int p_level);
    
    static bool Init(void);


    struct TerrainVertex
    {
        XMFLOAT3 positionMC;
        XMFLOAT3 normalMC;
        float materialMix;
        unsigned char material1;
        unsigned char material2;
    };
    static D3D11_INPUT_ELEMENT_DESC sm_pTerrainVertexElementDesc[5];
    static unsigned int sm_terrainVertexNumElements;


};
