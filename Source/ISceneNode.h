#pragma once

class Scene;

class ISceneNode
{
public:
    enum NodeType
    {
        UNKNOWN = 0,
        STATIC,
        DYNAMIC,
    };

    virtual ~ISceneNode(void) { }

    virtual HRESULT VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis) = 0;
    virtual HRESULT VOnRestore(void) = 0;
    virtual HRESULT VOnLostDevice(void) = 0;

    virtual HRESULT VPreRender(Scene* p_pScene) = 0;
    virtual HRESULT VRender(Scene* p_pScene) = 0;
    virtual HRESULT VRenderChildren(Scene* p_pScene) = 0;
    virtual HRESULT VPostRender(Scene* p_pScene) = 0;

    virtual bool VAddChild(std::shared_ptr<ISceneNode> p_pChild) = 0;
    virtual bool VRemoveChild(ActorID p_actorID) = 0;
    virtual bool VIsVisible(Scene* p_pScene) const = 0;
    virtual NodeType VGetNodeType(void) const = 0;
    
};

