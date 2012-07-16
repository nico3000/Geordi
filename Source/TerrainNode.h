#pragma once
#include "ISceneNode.h"
#include "TerrainData.h"
#include "Geometry.h"
#include "ShaderProgram.h"

class TerrainNode :
    public ISceneNode
{
private:
    const std::shared_ptr<Geometry> m_pGeometry;
    const std::shared_ptr<TerrainData> m_pTerrain;
    ShaderProgram m_program;

public:
    TerrainNode(std::string p_terrainFolder) : m_pTerrain(new TerrainData(p_terrainFolder)), m_pGeometry(new Geometry) {}
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

