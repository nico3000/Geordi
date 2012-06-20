#pragma once
#include "Actor.h"

class IGameLogic
{
public:
    virtual ~IGameLogic(void) {}

    virtual bool VInit(void) = 0;
    virtual void VDestroy(void) = 0;
    virtual void VUpdate(unsigned long p_deltaMillis) = 0;
    virtual void VAddActor(StrongActorPtr p_pActor) = 0;
    virtual void VDeleteActor(ActorID p_id) = 0;
    virtual WeakActorPtr VCreateActor(const char* p_actorResource) = 0;
    virtual WeakActorPtr VGetActor(ActorID p_id) = 0;
};

