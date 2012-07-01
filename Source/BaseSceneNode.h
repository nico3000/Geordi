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
    WeakActorPtr m_pActor;

public:
    BaseSceneNode(ActorID p_actorID);
    ~BaseSceneNode(void) { }

    virtual HRESULT VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis);

    virtual HRESULT VPreRender(Scene* p_pScene);
    virtual HRESULT VRender(Scene* p_pScene) = 0;
    HRESULT VRenderChildren(Scene* p_pScene);
    virtual HRESULT VPostRender(Scene* p_pScene);
    
    virtual HRESULT VOnRestore(void);
    virtual HRESULT VOnLostDevice(void);
    
    bool VAddChild(std::shared_ptr<ISceneNode> p_pChild);
    bool VRemoveChild(ActorID p_actorID);
    virtual NodeType VGetNodeType(void) const { return DYNAMIC; }

    bool VIsVisible(Scene* p_pScene) const { return m_isVisible; }

};

