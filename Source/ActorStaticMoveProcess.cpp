#include "StdAfx.h"
#include "ActorStaticMoveProcess.h"
#include "ActorEvents.h"

ActorStaticMoveProcess::ActorStaticMoveProcess(ActorID p_pActorID, float p_speed) :
m_actorID(p_pActorID), m_direction(0.0f, 0.0f, 0.0f), m_speed(p_speed)
{
}


ActorStaticMoveProcess::~ActorStaticMoveProcess(void)
{
}


void ActorStaticMoveProcess::VOnInit(void)
{
    Process::VOnInit();
    if(LostIsland::g_pApp->GetGameLogic()->VGetActor(m_actorID).lock() == 0)
    {
        LI_ERROR("unknown actor id given to ActorStaticMoveProcess");
        this->Fail();
    }
    else
    {

    }
}


void ActorStaticMoveProcess::VOnUpdate(unsigned long p_deltaMillis)
{
    float distance = m_speed * 1e-3f * (float)p_deltaMillis;
    if(distance != 0 && (abs(m_direction.x) + abs(m_direction.y) + abs(m_direction.z)) != 0)
    {
        static XMFLOAT3 translation(0.0f, 0.0f, 0.0f);
        translation.x = distance * m_direction.x;
        translation.y = distance * m_direction.y;
        translation.z = distance * m_direction.z;
        IEventDataPtr pEvent(new ActorTranslateEvent(m_actorID, translation, false));
        EventManager::Get()->VQueueEvent(pEvent);
    }
}