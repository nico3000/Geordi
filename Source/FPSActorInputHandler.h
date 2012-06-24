#pragma once
#include "InputController.h"
#include "ActorStaticMoveProcess.h"

class FPSActorInputHandler :
    public IPointerHandler, public IKeyboardHandler
{
private:
    ActorID m_actorID;
    std::shared_ptr<ActorStaticMoveProcess> m_pProcess;
    std::weak_ptr<PoseComponent> m_pPose;
    XMFLOAT3 m_moveDir;
    XMFLOAT3 m_worldMoveDir;
    XMFLOAT3 m_sideDir;
    XMFLOAT3 m_upDir;
    XMFLOAT3 m_viewDir;
    XMFLOAT3 m_pos;
    float m_phi;
    float m_theta;

    void UpdateWorldMoveDir(void);

public:
    FPSActorInputHandler(ActorID p_actorID);
    ~FPSActorInputHandler(void);

    // keyboard listeners
    bool VOnKeyUp(int keycode);
    bool VOnKeyDown(int keycode);

    // pointer listeners
    bool VOnPointerMoved(int p_x, int p_y, int p_dx, int p_dy);
    bool VOnButtonDown(unsigned int p_button);
    bool VOnButtonUp(unsigned int p_button);
};

