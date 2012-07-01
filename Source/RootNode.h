#pragma once
#include "ISceneNode.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#define DS_NUM_TARGETS 3

class RootNode :
    public ISceneNode
{
private:
    typedef std::vector<std::shared_ptr<ISceneNode>> NodeList;

    NodeList m_staticNodes;
    NodeList m_dynamicNodes;
    ID3D11Texture2D* m_ppTex[DS_NUM_TARGETS];
    ID3D11ShaderResourceView* m_ppSRV[DS_NUM_TARGETS];
    ID3D11RenderTargetView* m_ppRTV[DS_NUM_TARGETS];
    ID3D11Texture2D* m_pDSVTex;
    ID3D11DepthStencilView* m_pDSV;
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

