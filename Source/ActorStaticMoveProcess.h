#pragma once
#include "ProcessManager.h"
#include "PoseComponent.h"

class ActorStaticMoveProcess :
    public Process
{
private:
    ActorID m_actorID;
    XMFLOAT3 m_direction;
    float m_speed;

    void UpdateTransform(void);

protected:
    void VOnInit(void);
    void VOnUpdate(unsigned long p_deltaMillis);

public:
    ActorStaticMoveProcess(ActorID p_pActorID, float p_speed);
    ~ActorStaticMoveProcess(void);

    void SetSpeed(float p_speed) { m_speed = p_speed; }
    void SetDirection(const XMFLOAT3& p_direction) { m_direction = p_direction; } 

};

