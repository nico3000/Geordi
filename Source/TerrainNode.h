#pragma once
#include "ISceneNode.h"
#include "TerrainData.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "Grid3D.h"
#include "MarchingCubeGrid.h"


#define LOD_RADIUS 2
#define NUM_LEVELS 4
#define NUM_BLOCKS 8


class TerrainNode :
    public ISceneNode
{
    friend class TerrainBlock;

private:
    class TerrainBlock
    {
    private:
        TerrainNode* m_pTerrainNode;
        int m_x, m_y, m_z;
        int m_level;
        bool m_isRefined;
        int m_geometry;
        std::shared_ptr<TerrainBlock> m_pRefined[8];
        std::shared_ptr<Geometry> m_pGeometry;
        std::shared_ptr<Geometry> m_pBackup;
        std::shared_ptr<Geometry> m_pWireframe;

        bool BuildGeometry(void);
        void ReleaseGeometry(bool p_releaseChildren);

    public:
        TerrainBlock(int p_x, int p_y, int p_z, int p_level, TerrainNode* p_pTerrainNode);
        ~TerrainBlock(void);

        bool SetPointOfReference(int p_x, int p_y, int p_z);

    };

    typedef std::list<std::weak_ptr<Geometry>> GeometryList;

    std::shared_ptr<TerrainData> m_pTerrain;

    std::shared_ptr<TerrainBlock> m_pTest[NUM_BLOCKS * NUM_BLOCKS * NUM_BLOCKS];
    GeometryList m_blockList;
    GeometryList m_wireframeList;
    Octree m_pGeometryData[NUM_LEVELS];
    int m_chunksize;
    ShaderProgram m_program;
    Grid3D m_tempGrid;
    MarchingCubeGrid m_tempMCGrid;
    float m_scale;
    int m_maxBlocksPerFrame;
    int m_currentBlocksPerFrame;
    ID3D11ShaderResourceView* m_pDiffuseTex;
    ID3D11ShaderResourceView* m_pBumpTex;
    ID3D11ShaderResourceView* m_pNormalTex;

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