#pragma once
#include "ISceneNode.h"
#include "TerrainData.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "Grid3D.h"


#define NUM_BLOCKS 1024


class TerrainBlock 
{
private:
    int m_x, m_y, m_z;
    Geometry m_geometry;

public:
    TerrainBlock(void) {}
    ~TerrainBlock(void) {}

    void Build(int p_x, int p_y, int p_z, Grid3D& p_grid, float p_scale);
    
    void Draw(void) { if(m_geometry.IsReady()) m_geometry.Draw(); }
    int GetX(void) const { return m_x; }
    int GetY(void) const { return m_y; }
    int GetZ(void) const { return m_z; }
    Geometry& GetGeometry(void) { return m_geometry; }
    bool HasGeometry(void) const { return m_geometry.IsReady(); }

};


class TerrainNode :
    public ISceneNode
{
private:
    std::shared_ptr<TerrainData> m_pTerrain;
    TerrainBlock m_blocks[NUM_BLOCKS];
    Octree m_blockData;
    int m_smallradius;
    int m_chunksize;
    ShaderProgram m_program;
    std::list<std::shared_ptr<Geometry>> m_empty;
    Grid3D m_tempGrid;

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