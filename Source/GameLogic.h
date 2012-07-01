#pragma once
#include "IGameLogic.h"
#include "EventManager.h"
#include "ProcessManager.h"
#include "ParticleSystem.h"

class GameLogic :
    public IGameLogic
{
    friend class GameApp;
    typedef std::map<ActorID, StrongActorPtr> ActorMap;
    typedef std::list<std::shared_ptr<IGameView>> GameViewList;

private:
    ActorMap m_actors;
    ActorFactory* m_pActorFactory;
    ProcessManager* m_pProcessManager;
    GameViewList m_gameViews;
    ParticleSystem* m_pParticleSystem;

public:
    GameLogic(void);
    ~GameLogic(void);

    bool VInit(void);
    void VDestroy(void);
    void VUpdate(unsigned long p_deltaMillis);
    void VRender(unsigned long p_deltaMillis);
    void VRestore(void);
    void VDeleteActor(ActorID p_id);
    StrongActorPtr VCreateActor(const char* p_actorResource);
    WeakActorPtr VGetActor(ActorID p_id);

    void AttachProcess(StrongProcessPtr p_pProcess) { if(m_pProcessManager) m_pProcessManager->AttachProcess(p_pProcess); }
    void AttachView(std::shared_ptr<IGameView> p_gameView, ActorID p_actorID = INVALID_ACTOR_ID);
    void RemoveView(std::shared_ptr<IGameView> p_gameView);

    void ActorMoveDelegate(IEventDataPtr p_pEventData);

};

