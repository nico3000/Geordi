#include "StdAfx.h"
#include "GameLogic.h"
#include "TestEventData.h"
#include "ActorEvents.h"


GameLogic::GameLogic(void)
{
}


GameLogic::~GameLogic(void)
{
    this->VDestroy();
}


bool GameLogic::VInit(void)
{
    EventListenerDelegate del = fastdelegate::MakeDelegate(this, &GameLogic::Test);
    EventManager::Get()->VAddListener(del, TestEventData::sm_eventType);
    return true;
}


void GameLogic::VDestroy(void)
{
    for(auto iter=m_actors.begin(); iter != m_actors.end(); ++iter)
    {
        iter->second->Destroy();
    }
    m_actors.clear();
}


void GameLogic::VUpdate(unsigned long p_deltaMillis)
{
    EventManager::Get()->VUpdate(20);
    for(auto iter=m_actors.begin(); iter != m_actors.end(); ++iter)
    {
        iter->second->Update(p_deltaMillis);
    }
}


WeakActorPtr GameLogic::VGetActor(ActorID p_id)
{
    ActorMap::iterator findIt = m_actors.find(p_id);
    if(findIt == m_actors.end())
    {
        return WeakActorPtr();
    }
    else
    {
        return WeakActorPtr(findIt->second);
    }
}


void GameLogic::VAddActor(StrongActorPtr p_pActor)
{
    m_actors[p_pActor->GetID()] = p_pActor;
}


void GameLogic::VDeleteActor(ActorID p_id)
{
    auto findIt = m_actors.find(p_id);
    if(findIt != m_actors.end())
    {
        IEventDataPtr pEvent(new ActorDestroyedEvent(p_id));
        EventManager::Get()->VQueueEvent(pEvent);
        m_actors.erase(findIt);
    }
    
}


WeakActorPtr GameLogic::VCreateActor(const char* p_actorResource)
{
    StrongActorPtr pActor = LostIsland::g_pApp->GetActorFactory()->CreateActor(p_actorResource);
    if(pActor)
    {
        IEventDataPtr pEvent(new ActorCreatedEvent(pActor->GetID()));
        EventManager::Get()->VQueueEvent(pEvent);

        m_actors[pActor->GetID()] = pActor;
        return WeakActorPtr(pActor);
    }
    else
    {
        return WeakActorPtr();
    }
}