#pragma once
#include "IGameLogic.h"

class GameLogic :
    public IGameLogic
{
    typedef std::map<ActorID, StrongActorPtr> ActorMap;

private:
    ActorMap m_actors;

public:
    GameLogic(void);
    ~GameLogic(void);

    bool VInit(void);
    void VDestroy(void);
    void VUpdate(unsigned long p_deltaMillis);
    void VAddActor(StrongActorPtr p_pActor);
    void VDeleteActor(ActorID p_id);
    WeakActorPtr VCreateActor(const char* p_actorResource);
    WeakActorPtr VGetActor(ActorID p_id);

};

