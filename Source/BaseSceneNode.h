#pragma once
#include "ISceneNode.h"
class BaseSceneNode :
    public ISceneNode
{
private:
    WeakActorPtr m_pActor;
    typedef std::list<std::shared_ptr<ISceneNode>> SceneNodeList;

    SceneNodeList m_children;
    bool m_isVisible;

public:
    BaseSceneNode(WeakActorPtr p_pActor) : m_pActor(p_pActor), m_isVisible(true) { }
    ~BaseSceneNode(void) { }

    virtual HRESULT VRender(void) { return S_OK; }

    virtual HRESULT VOnUpdate(unsigned long p_deltaMillis) { return S_OK; }
    virtual HRESULT VPreRender(void) { return S_OK; }
    virtual HRESULT VPostRender(void) { return S_OK; }

    virtual HRESULT VOnRestore(void);
    virtual HRESULT VOnLostDevice(void);

    HRESULT VRenderChildren(void);
    bool VAddChild(std::shared_ptr<ISceneNode> p_pChild);
    bool VRemoveChild(ActorID p_actorID);

    bool VIsVisible(void) const { return m_isVisible; }

};

