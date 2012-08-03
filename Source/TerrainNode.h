#pragma once
#include "ISceneNode.h"
#include "TerrainData.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "Grid3D.h"


#define NUM_LEVELS (5)
#define BLOCK_DIMENSION (4)
//#define MAX_BLOCKS ((BLOCK_DIMENSION * BLOCK_DIMENSION * BLOCK_DIMENSION / 8) * (7 * NUM_LEVELS + 1))
#define MAX_BLOCKS 128

class TerrainNode :
    public ISceneNode
{
private:
    struct TerrainBlock 
    {
        int x, y, z;
        int level;
        std::shared_ptr<Geometry> pGeometry;
    };

    typedef std::list<TerrainBlock> BlockList;

    std::shared_ptr<TerrainData> m_pTerrain;
    BlockList m_pBlockLists[NUM_LEVELS];
    Octree m_pGeometryData[NUM_LEVELS];
    int m_chunksize;
    ShaderProgram m_program;
    std::list<std::shared_ptr<Geometry>> m_empty;
    Grid3D m_tempGrid;
    float m_scale;

    bool IsValid(int p_blockX, int p_blockY, int p_blockZ, int p_camX, int p_camY, int p_camZ, int p_level) const;
    bool CreateBlock(int p_level, int p_x, int p_y, int p_z);

public:
    TerrainNode(std::shared_ptr<TerrainData> p_pTerrain, int p_chunksize, int p_smallradius);
    ~TerrainNode(void);

    HRESULT VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis);
    HRESULT VOnRestore(void);
    HRESULT VOnLostDevice(void);

    HRESULT VPreRender(Scene* p_pScene);
    HRESULT VRender(Scene* p_pScene);
    HRESULT VRenderChildren(Scene* p_pScene) { return S_OK; }
    HRESULT VPostRender(Scene* p_pScene);

    bool VAddChild(std::shared_ptr<ISceneNode> p_pChild) { return false; }
    bool VRemoveChild(ActorID p_actorID) { return false; }
    bool VIsVisible(Scene* p_pScene) const { return true; }
    NodeType VGetNodeType(void) const { return ISceneNode::STATIC_NODE; }
};