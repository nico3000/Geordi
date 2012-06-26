#include "StdAfx.h"
#include "GameLogic.h"
#include "TestEventData.h"
#include "ActorEvents.h"

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

    EventListenerDelegate onActorMove = fastdelegate::MakeDelegate(this, &GameLogic::ActorMoveDelegate);
    EventManager::Get()->VAddListener(onActorMove, ActorMoveEvent::sm_eventType);

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
    //LI_INFO("next frame");

    m_pProcessManager->UpdateProcesses(p_deltaMillis);
    EventManager::Get()->VUpdate(20);
    for(auto iter=m_actors.begin(); iter != m_actors.end(); ++iter)
    {
        iter->second->Update(p_deltaMillis);
    }

    for(auto iter=m_gameViews.begin(); iter != m_gameViews.end(); ++iter)
    {
        (*iter)->VOnUpdate(p_deltaMillis);
    }
}


void GameLogic::VRender(unsigned long p_deltaMillis)
{
    for(auto iter=m_gameViews.begin(); iter != m_gameViews.end(); ++iter)
    {
        (*iter)->VOnRender(p_deltaMillis);
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


void GameLogic::AttachView(std::shared_ptr<IGameView> p_gameView, ActorID p_actorID /* = INVALID_ACTOR_ID */)
{
    GameViewID viewID = (GameViewID)m_gameViews.size();
    m_gameViews.push_back(p_gameView);
    p_gameView->VOnAttach(viewID, p_actorID);
    p_gameView->VOnRestore();
}


void GameLogic::RemoveView(std::shared_ptr<IGameView> p_gameView)
{
    m_gameViews.remove(p_gameView);
}


void GameLogic::ActorMoveDelegate(IEventDataPtr p_pEventData)
{
    std::shared_ptr<ActorMoveEvent> pEventData = std::static_pointer_cast<ActorMoveEvent>(p_pEventData);
    if(pEventData)
    {
        StrongActorPtr pActor = this->VGetActor(pEventData->GetActorID()).lock();
        if(pActor)
        {
            pActor->GetPose().TranslateLocal(pEventData->GetDeltaTranslation());
            pActor->GetPose().RotateLocal(pEventData->GetDeltaRotation().x, pEventData->GetDeltaRotation().y, pEventData->GetDeltaRotation().z);
            pActor->GetPose().Scale(pEventData->GetDeltaScaling());

            //LI_INFO("actor moved");
            std::shared_ptr<ActorMovedEvent> pActorMoved(new ActorMovedEvent(pActor->GetID()));
            EventManager::Get()->VQueueEvent(pActorMoved);
        }
    }
}