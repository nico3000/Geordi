#pragma once
#include "XMMatrixStack.h"
#include "RootNode.h"
#include "Camera.h"
#include "ConstantBuffer.h"

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
    ConstantBuffer m_modelBuffer;

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

    void PushModelMatrices(const Pose::ModelMatrixData& p_modelMatrixData, bool p_updateBuffer = true);
    void PopModelMatrices(bool p_updateBuffer = false);
    bool UpdateModelMatrixBuffer(void) { return m_modelBuffer.Update(); }

    void RenderComponentCreatedDelegate(IEventDataPtr p_pEvent);
    void ParticleComponentCreatedDelegate(IEventDataPtr p_pEvent);
    void TerrainComponentCreatedDelegate(IEventDataPtr p_pEvent);

};

