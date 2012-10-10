#include "StdAfx.h"
#include "FPSActorInputHandler.h"
#include "ActorEvents.h"
#include "TransformComponent.h"


FPSActorInputHandler::FPSActorInputHandler(ActorID p_cameraActorID) :
m_actorID(INVALID_ACTOR_ID)
{
}


void FPSActorInputHandler::VOnAttach(GameViewID p_gameViewID, ActorID p_actorID)
{
    m_actorID = p_actorID;
    m_pActor = LostIsland::g_pApp->GetGameLogic()->VGetActor(p_actorID);
}


void FPSActorInputHandler::VOnUpdate(unsigned long p_deltaMillis)
{
    static unsigned int id = LostIsland::g_pTimer->Tick(REALTIME);
    unsigned long deltaMillisRealtime = LostIsland::g_pTimer->Tock(id, RESET);
    static const XMFLOAT3 rotation(0.0f, 0.0f, 0.0f);
    float distance = 1e-3f * (float)deltaMillisRealtime;
    XMFLOAT3 moveDir(0.0f, 0.0f, 0.0f);
    if(m_keys[VK_SHIFT])
    {
        distance *= 20.0f;
    }
    if(m_keys['W'])
    {
        moveDir.z += distance;
    }
    if(m_keys['S'])
    {
        moveDir.z -= distance;
    }
    if(m_keys['A'])
    {
        moveDir.x -= distance;
    }
    if(m_keys['D'])
    {
        moveDir.x += distance;
    }
    if(m_keys[' '])
    {
        moveDir.y += distance;
    }
    if(m_keys['C'])
    {
        moveDir.y -= distance;
    }    

    if(abs(moveDir.x) + abs(moveDir.y) + abs(moveDir.z) != 0.0f)
    {
        //LI_INFO("actor move");
        std::shared_ptr<ActorMoveEvent> pEventData(new ActorMoveEvent(m_actorID, moveDir, rotation, 1.0f));
        EventManager::Get()->VQueueEvent(pEventData);
    }
}


FPSActorInputHandler::~FPSActorInputHandler(void)
{
}


bool FPSActorInputHandler::VOnKeyDown(int keycode)
{
    m_keys[keycode] = true;
    return true;
}


bool FPSActorInputHandler::VOnKeyUp(int keycode)
{
    if(keycode == 'P')
    {
        if(LostIsland::g_pTimer->IsPaused())
        {
            LostIsland::g_pTimer->Resume();
        }
        else
        {
            LostIsland::g_pTimer->Pause();
        }
    }
    m_keys[keycode] = false;
    return true;
}


bool FPSActorInputHandler::VOnButtonDown(unsigned int button) { return false; }


bool FPSActorInputHandler::VOnButtonUp(unsigned int button) { return false; }


bool FPSActorInputHandler::VOnPointerMoved(int p_x, int p_y, int p_dx, int p_dy)
{
    StrongActorPtr pActor = m_pActor.lock();
    std::shared_ptr<TransformComponent> pTransform = pActor ? pActor->GetComponent<TransformComponent>(TransformComponent::sm_componentID).lock() : 0;
    if(pTransform)
    {
        float pitch = pTransform->GetPose().GetPitch();
        float yaw = pTransform->GetPose().GetYaw();
        float newPitch = CLAMP(pitch + 1e-2f * (float)p_dy, -XM_PIDIV2, +XM_PIDIV2);
        float newYaw = yaw + 1e-2f * (float)p_dx;
        static const XMFLOAT3 translation(0.0f, 0.0f, 0.0f);
        XMFLOAT3 rotation(newPitch - pitch, newYaw - yaw, 0.0f);
        
        std::shared_ptr<ActorMoveEvent> pEventData(new ActorMoveEvent(m_actorID, translation, rotation, 1.0f));
        EventManager::Get()->VQueueEvent(pEventData);

        return true;
    }
    else
    {
        return false;
    }
}


