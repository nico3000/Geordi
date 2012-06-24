#pragma once
class ISceneNode
{
public:
    virtual ~ISceneNode(void) { }

    virtual HRESULT VOnUpdate(unsigned long p_deltaMillis) = 0;
    virtual HRESULT VOnRestore(void) = 0;
    virtual HRESULT VOnLostDevice(void) = 0;

    virtual HRESULT VPreRender(void) = 0;
    virtual bool VIsVisible(void) const = 0;
    virtual HRESULT VRender(void) = 0;
    virtual HRESULT VRenderChildren(void) = 0;
    virtual HRESULT VPostRender(void) = 0;

    virtual bool VAddChild(std::shared_ptr<ISceneNode> p_pChild) = 0;
    virtual bool VRemoveChild(ActorID p_actorID) = 0;
    
};

