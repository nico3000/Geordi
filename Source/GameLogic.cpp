#include "StdAfx.h"
#include "GameLogic.h"
#include "TestEventData.h"
#include "ActorEvents.h"
#include "PoseComponent.h"

GameLogic::GameLogic(void)
{
}


GameLogic::~GameLogic(void)
{
    m_pProcessManager->AbortAllProcesses(true);
    this->VDestroy();
    SAFE_DELETE(m_pActorFactory);
    SAFE_DELETE(m_pProcessManager);
}


bool GameLogic::VInit(void)
{
    m_pProcessManager = new ProcessManager;
    if(!m_pProcessManager)
    {
        LI_ERROR("ProcessManager initialization error");
        return false;
    }
    m_pActorFactory = new ActorFactory;
    if(!m_pActorFactory)
    {
        LI_ERROR("ActorFactory initialization failed");
        return false;
    }

    EventListenerDelegate del = fastdelegate::MakeDelegate(this, &GameLogic::Test);
    EventManager::Get()->VAddListener(del, TestEventData::sm_eventType);

    EventListenerDelegate onActorTranslateDelegate = fastdelegate::MakeDelegate(this, &GameLogic::OnActorTranslate);
    EventManager::Get()->VAddListener(onActorTranslateDelegate, ActorTranslateEvent::sm_eventType);

    EventListenerDelegate onActorRotateDelegate = fastdelegate::MakeDelegate(this, &GameLogic::OnActorRotate);
    EventManager::Get()->VAddListener(onActorRotateDelegate, ActorRotateEvent::sm_eventType);

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
    m_pProcessManager->UpdateProcesses(p_deltaMillis);
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


StrongActorPtr GameLogic::VCreateActor(const char* p_actorResource)
{
    StrongActorPtr pActor = m_pActorFactory->CreateActor(p_actorResource);
    if(pActor)
    {
        IEventDataPtr pEvent(new ActorCreatedEvent(pActor->GetID()));
        EventManager::Get()->VQueueEvent(pEvent);

        m_actors[pActor->GetID()] = pActor;
    }
    return pActor;
}


void GameLogic::OnActorTranslate(IEventDataPtr pEvent)
{
    std::shared_ptr<ActorTranslateEvent> pMoveEvent = std::static_pointer_cast<ActorTranslateEvent>(pEvent);
    StrongActorPtr pActor = this->VGetActor(pMoveEvent->GetActorID()).lock();
    if(pActor)
    {
        std::shared_ptr<PoseComponent> pPose = pActor->GetComponent<PoseComponent>(PoseComponent::GetComponentID()).lock();
        if(pPose)
        {
            if(pMoveEvent->IsAbsolute())
            {
                pPose->SetTranslation(pMoveEvent->GetTranslation());
            }
            else
            {
                pPose->Translate(pMoveEvent->GetTranslation());
            }
        }
        else
        {
            LI_ERROR("tried to translate an untranslateable actor");
        }
    }
}


void GameLogic::OnActorRotate(IEventDataPtr pEvent)
{
    std::shared_ptr<ActorRotateEvent> pMoveEvent = std::static_pointer_cast<ActorRotateEvent>(pEvent);
    StrongActorPtr pActor = this->VGetActor(pMoveEvent->GetActorID()).lock();
    if(pActor)
    {
        std::shared_ptr<PoseComponent> pPose = pActor->GetComponent<PoseComponent>(PoseComponent::GetComponentID()).lock();
        if(pPose)
        {
            if(pMoveEvent->IsAbsolute())
            {
                pPose->SetRotation(pMoveEvent->GetRotation());
            }
            else
            {
                pPose->Rotate(pMoveEvent->GetRotation());
            }
        }
        else
        {
            LI_ERROR("tried to rotate an unrotateable actor");
        }
    }
}