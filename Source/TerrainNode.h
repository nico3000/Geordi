#pragma once
#include "ISceneNode.h"
#include "TerrainData.h"
#include "ShaderProgram.h"


#define NUM_BLOCKS 16


class TerrainNode :
    public ISceneNode
{
private:
    std::shared_ptr<TerrainData> m_pTerrain;
    std::shared_ptr<TerrainBlock> m_pTest[NUM_BLOCKS * NUM_BLOCKS * NUM_BLOCKS];
    ShaderProgram m_program;
    ID3D11ShaderResourceView* m_pDiffuseTex;
    ID3D11ShaderResourceView* m_pBumpTex;
    ID3D11ShaderResourceView* m_pNormalTex;

public:
    TerrainNode(std::shared_ptr<TerrainData> p_pData);
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