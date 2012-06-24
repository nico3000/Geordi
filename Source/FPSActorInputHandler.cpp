#include "StdAfx.h"
#include "FPSActorInputHandler.h"
#include "ActorEvents.h"


FPSActorInputHandler::FPSActorInputHandler(ActorID p_actorID) :
m_actorID(p_actorID), m_moveDir(0.0f, 0.0f, 0.0f), m_pProcess(0), m_phi(0), m_theta(0), m_pos(0.0f, 0.0f, 0.0f),
    m_sideDir(1.0f, 0.0f, 0.0f), m_upDir(0.0f, 1.0f, 0.0f), m_viewDir(0.0f, 0.0f, 1.0f)
{
    m_pProcess.reset(new ActorStaticMoveProcess(p_actorID, 1.0f));
    StrongActorPtr pActor = LostIsland::g_pApp->GetGameLogic()->VGetActor(p_actorID).lock();
    if(pActor)
    {
        m_pPose = pActor->GetComponent<PoseComponent>(PoseComponent::GetComponentID());
    }
    LostIsland::g_pApp->GetGameLogic()->AttachProcess(m_pProcess);
}


FPSActorInputHandler::~FPSActorInputHandler(void)
{
}


bool FPSActorInputHandler::VOnKeyDown(int keycode)
{
    switch(keycode)
    {
    case 'W': m_moveDir.z += 1.0f; break;
    case 'S': m_moveDir.z -= 1.0f; break;
    case 'A': m_moveDir.x -= 1.0f; break;
    case 'D': m_moveDir.x += 1.0f; break;
    case ' ': m_moveDir.y += 1.0f; break;
    case 'C': m_moveDir.y -= 1.0f; break;
    default: return false;
    }
    this->UpdateWorldMoveDir();
    return true;
}


bool FPSActorInputHandler::VOnKeyUp(int keycode)
{
    switch(keycode)
    {
    case 'W': m_moveDir.z -= 1.0f; break;
    case 'S': m_moveDir.z += 1.0f; break;
    case 'A': m_moveDir.x += 1.0f; break;
    case 'D': m_moveDir.x -= 1.0f; break;
    case ' ': m_moveDir.y -= 1.0f; break;
    case 'C': m_moveDir.y += 1.0f; break;
    default: return false;
    }
    this->UpdateWorldMoveDir();
    return true;
}


void FPSActorInputHandler::UpdateWorldMoveDir(void)
{
    m_worldMoveDir.x = m_moveDir.z * m_viewDir.x + m_moveDir.y * m_upDir.x + m_moveDir.x * m_sideDir.x;
    m_worldMoveDir.y = m_moveDir.z * m_viewDir.y + m_moveDir.y * m_upDir.y + m_moveDir.x * m_sideDir.y;
    m_worldMoveDir.z = m_moveDir.z * m_viewDir.z + m_moveDir.y * m_upDir.z + m_moveDir.x * m_sideDir.z;
    m_pProcess->SetDirection(m_worldMoveDir);
}


bool FPSActorInputHandler::VOnButtonDown(unsigned int button) { return false; }


bool FPSActorInputHandler::VOnButtonUp(unsigned int button) { return false; }


bool FPSActorInputHandler::VOnPointerMoved(int p_x, int p_y, int p_dx, int p_dy)
{
    std::shared_ptr<PoseComponent> pPose = m_pPose.lock();
    if(pPose)
    {
        static const float speed = 1e-2f;
        m_phi += speed * p_dx;
        m_theta = CLAMP(m_theta + speed * p_dy, -XM_PIDIV2, +XM_PIDIV2);

        float sinPhi = sin(m_phi);
        float cosPhi = cos(m_phi);
        float sinTheta = sin(m_theta);
        float cosTheta = cos(m_theta);

        m_sideDir.x = cosPhi;  m_upDir.x = sinPhi * sinTheta; m_viewDir.x = sinPhi * cosTheta;
        m_sideDir.y = 0.0f;    m_upDir.y = cosTheta;          m_viewDir.y = -sinTheta;
        m_sideDir.z = -sinPhi; m_upDir.z = cosPhi * sinTheta; m_viewDir.z = cosPhi * cosTheta;

        this->UpdateWorldMoveDir();

        static XMFLOAT3 rotation(0.0f, 0.0f, 0.0f);
        rotation.x = m_theta;
        rotation.y = m_phi;

        IEventDataPtr pEvent(new ActorRotateEvent(m_actorID, rotation, true));
        EventManager::Get()->VQueueEvent(pEvent);
        
        return true;
    }
    else
    {
        return false;
    }
}