#pragma once
#include "IGameLogic.h"
#include "EventManager.h"
#include "ProcessManager.h"

class GameLogic :
    public IGameLogic
{
    typedef std::map<ActorID, StrongActorPtr> ActorMap;

private:
    ActorMap m_actors;
    ActorFactory* m_pActorFactory;
    ProcessManager* m_pProcessManager;

public:
    GameLogic(void);
    ~GameLogic(void);

    bool VInit(void);
    void VDestroy(void);
    void VUpdate(unsigned long p_deltaMillis);
    void VDeleteActor(ActorID p_id);
    StrongActorPtr VCreateActor(const char* p_actorResource);
    WeakActorPtr VGetActor(ActorID p_id);

    void Test(IEventDataPtr p_pEvent) { LI_INFO(std::string(p_pEvent->VGetName()) + " fired"); }
    void OnActorTranslate(IEventDataPtr pEvent);
    void OnActorRotate(IEventDataPtr pEvent);

    void AttachProcess(StrongProcessPtr p_pProcess) { if(m_pProcessManager) m_pProcessManager->AttachProcess(p_pProcess); }

};

