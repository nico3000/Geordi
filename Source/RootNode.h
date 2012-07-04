#pragma once
#include "ISceneNode.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "RenderTarget.h"
#include "IPostEffect.h"
#define DS_NUM_TARGETS 3

class RootNode :
    public ISceneNode
{
private:
    typedef std::vector<std::shared_ptr<ISceneNode>> NodeList;
    typedef std::vector<std::shared_ptr<IPostEffect>> PoseEffectList;

    NodeList m_staticNodes;
    NodeList m_dynamicNodes;
    PoseEffectList m_effects;
    std::shared_ptr<RenderTarget> m_pBase;
    std::shared_ptr<RenderTarget> m_pEnlightened;
    Geometry m_screenQuad;
    ShaderProgram m_dsTest;

public:
    RootNode(void);
    ~RootNode(void);

    HRESULT VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis);
    HRESULT VOnRestore(void);
    HRESULT VOnLostDevice(void);

    HRESULT VPreRender(Scene* p_pScene);
    HRESULT VRender(Scene* p_pScene);
    HRESULT VRenderChildren(Scene* p_pScene);
    HRESULT VPostRender(Scene* p_pScene);

    bool VAddChild(std::shared_ptr<ISceneNode> p_pChild);
    bool VRemoveChild(ActorID p_actorID);
    NodeType VGetNodeType(void) const { return UNKNOWN; }
    bool VIsVisible(Scene* p_pScene) const { return true; }
};

