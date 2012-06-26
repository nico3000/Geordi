#pragma once
#include "ISceneNode.h"
#include "SceneNodeProperties.h"

class BaseSceneNode :
    public ISceneNode
{
private:
    typedef std::list<std::shared_ptr<ISceneNode>> SceneNodeList;
    
    SceneNodeList m_children;
    bool m_isVisible;

protected:
    SceneNodeProperties m_properties;

public:
    BaseSceneNode(ActorID p_actorID, const XMFLOAT4X4& p_model, const XMFLOAT4X4* p_pModelInv = 0);
    ~BaseSceneNode(void) { }

    virtual HRESULT VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis) { return S_OK; }
    virtual HRESULT VPreRender(Scene* p_pScene) { return S_OK; }
    virtual HRESULT VRender(Scene* p_pScene) { return S_OK; }
    virtual HRESULT VPostRender(Scene* p_pScene) { return S_OK; }

    virtual HRESULT VOnRestore(void);
    virtual HRESULT VOnLostDevice(void);

    HRESULT VRenderChildren(Scene* p_pScene);
    bool VAddChild(std::shared_ptr<ISceneNode> p_pChild);
    bool VRemoveChild(ActorID p_actorID);

    bool VIsVisible(Scene* p_pScene) const { return m_isVisible; }

};

