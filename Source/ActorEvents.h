#pragma once
#include "eventmanager.h"
class ActorCreatedEvent :
    public BaseEventData
{
private:
    static const EventType sm_eventType;

    const ActorID m_id;

public:
    ActorCreatedEvent(ActorID p_id) : m_id(p_id) {}
    virtual ~ActorCreatedEvent(void) {}

    virtual const char* VGetName(void) const { return "ActorCreatedEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorCreatedEvent(m_id)); }

};


class ActorDestroyedEvent :
    public BaseEventData
{
private:
    static const EventType sm_eventType;

    const ActorID m_id;

public:
    ActorDestroyedEvent(ActorID p_id) : m_id(p_id) {}
    virtual ~ActorDestroyedEvent(void) {}

    virtual const char* VGetName(void) const { return "ActorDestroyedEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorDestroyedEvent(m_id)); }

};

