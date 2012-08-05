#include "StdAfx.h"
#include "GameLogic.h"
#include "TestEventData.h"
#include "ActorEvents.h"
#include "TransformComponent.h"

GameLogic::GameLogic(void) :
m_pActorFactory(0), m_pParticleSystem(0), m_pProcessManager(0), m_pTimeBuffer(new ConstantBuffer)
{
}


GameLogic::~GameLogic(void)
{
    m_pProcessManager->AbortAllProcesses(true);
    this->VDestroy();
    SAFE_DELETE(m_pActorFactory);
    SAFE_DELETE(m_pProcessManager);
    SAFE_DELETE(m_pParticleSystem);
    SAFE_DELETE(m_pTimeBuffer);
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
    m_pParticleSystem = new ParticleSystem;
    if(!m_pParticleSystem || !m_pParticleSystem->Init())
    {
        LI_ERROR("ParticleSystem initialization failed");
        return false;
    }
    TimeStruct time = { 0.0f, 0.0f, XMFLOAT2(0.0f, 0.0f) };
    if(!m_pTimeBuffer->CopyDataAndBuild(&time, sizeof(TimeStruct)))
    {
        LI_ERROR("TimeBuffer initialization failed");
        return false;
    }
    m_pTimeBuffer->UpdateAndBind(2, TARGET_ALL);

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


void GameLogic::VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis)
{
    //LI_INFO("next frame");
    TimeStruct* pTime = (TimeStruct*)m_pTimeBuffer->GetData();
    pTime->time = 1e-3f * (float)p_gameMillis;
    pTime->dTime = 1e-3f * (float)p_deltaMillis;
    m_pTimeBuffer->UpdateAndBind(2, TARGET_ALL);

    m_pProcessManager->UpdateProcesses(p_deltaMillis);
    EventManager::Get()->VUpdate(20);
    m_pParticleSystem->Simulate(p_deltaMillis);

    for(auto iter=m_actors.begin(); iter != m_actors.end(); ++iter)
    {
        iter->second->Update(p_deltaMillis, p_gameMillis);
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


void GameLogic::VRestore(void) 
{
    for(auto iter=m_gameViews.begin(); iter != m_gameViews.end(); ++iter)
    {
        (*iter)->VOnRestore();
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


StrongActorPtr GameLogic::VCreateActor(const char* p_actorResource, tinyxml2::XMLElement* p_pOverrideData)
{
    StrongActorPtr pActor = m_pActorFactory->CreateActor(p_actorResource, p_pOverrideData);
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
        std::shared_ptr<TransformComponent> pTransform = pActor ? pActor->GetComponent<TransformComponent>(TransformComponent::sm_componentID).lock() : 0;
        if(pTransform)
        {
            pTransform->GetPose().TranslateLocal(pEventData->GetDeltaTranslation());
            pTransform->GetPose().RotateLocal(pEventData->GetDeltaRotation().x, pEventData->GetDeltaRotation().y, pEventData->GetDeltaRotation().z);
            pTransform->GetPose().Scale(pEventData->GetDeltaScaling());

            //LI_INFO("actor moved");
            std::shared_ptr<ActorMovedEvent> pActorMoved(new ActorMovedEvent(pActor->GetID()));
            EventManager::Get()->VQueueEvent(pActorMoved);
        }
    }
}


bool GameLogic::VLoadGame(const char* p_levelResource)
{
    std::string levelName(p_levelResource);
    tinyxml2::XMLDocument doc;
    if(doc.LoadFile(p_levelResource) != tinyxml2::XML_NO_ERROR)
    {
        LI_ERROR("Loading level file failed: " + levelName);
        return false;
    }
    tinyxml2::XMLElement* pLevelData = doc.FirstChildElement("Level");
    if(!pLevelData)
    {
        LI_ERROR("No Level element in " + levelName);
        return false;
    }
    tinyxml2::XMLElement* pActorList = pLevelData->FirstChildElement("ActorList");
    tinyxml2::XMLElement* pActorData = pActorList ? pActorList->FirstChildElement("Actor") : 0;
    while(pActorData)
    {
        std::string actorType(pActorData->Attribute("type"));
        std::string actorResource("./Actors/" + actorType + ".xml");
        StrongActorPtr pActor = this->VCreateActor(actorResource.c_str(), pActorData->FirstChildElement());

        pActorData = pActorData->NextSiblingElement("Actor");
    }

    return true;
}