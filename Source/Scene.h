#pragma once
#include "XMMatrixStack.h"
#include "RootNode.h"
#include "Camera.h"

class Scene
{
    friend class BaseSceneNode;

private:
    typedef std::map<std::string,std::shared_ptr<Camera>> CameraMap;

    const XMMatrixStack m_matrixStack;
    std::shared_ptr<RootNode> m_pRoot;
    CameraMap m_cameras;
    std::shared_ptr<Camera> m_pCurrentCamera;

public:
    Scene(void);
    ~Scene(void);

    HRESULT OnUpdate(unsigned long m_deltaMillis);
    HRESULT OnRestore(void);
    HRESULT OnLostDevice(void);
    void Render(void);
    bool AddChild(ActorID p_actorID, std::shared_ptr<ISceneNode> p_pChild);
    bool RemoveChild(ActorID p_actorID);
    XMMatrixStack& GetMatrixStack(void);

};

