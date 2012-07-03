#pragma once
#include "ISceneNode.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "RenderTarget.h"
#define DS_NUM_TARGETS 3

class RootNode :
    public ISceneNode
{
private:
    typedef std::vector<std::shared_ptr<ISceneNode>> NodeList;

    NodeList m_staticNodes;
    NodeList m_dynamicNodes;
    RenderTarget m_base;
    RenderTarget m_enlightened;
    RenderTarget m_temp;
    ID3D11ComputeShader* m_pBlurHor;
    ID3D11ComputeShader* m_pBlurVer;
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

