#pragma once
#include "XMMatrixStack.h"
#include "RootNode.h"
#include "Camera.h"

class Scene
{
    friend class BaseSceneNode;

private:
    typedef std::map<std::string,std::shared_ptr<Camera>> CameraMap;

    XMMatrixStack m_modelStack;
    XMMatrixStack m_modelInvStack;
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
    void AddCamera(const std::string& p_name, std::shared_ptr<Camera> p_pCamera, bool p_activate = false);

    XMMatrixStack& GetModelStack(void) { return m_modelStack; }
    XMMatrixStack& GetModelInvStack(void) { return m_modelInvStack; }

};

