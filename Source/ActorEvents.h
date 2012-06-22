#pragma once
#include "eventmanager.h"
class ActorCreatedEvent :
    public BaseEventData
{
private:
    const ActorID m_id;

public:
    static const EventType sm_eventType;

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
    const ActorID m_id;

public:
    static const EventType sm_eventType;

    ActorDestroyedEvent(ActorID p_id) : m_id(p_id) {}
    virtual ~ActorDestroyedEvent(void) {}

    virtual const char* VGetName(void) const { return "ActorDestroyedEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorDestroyedEvent(m_id)); }

};


class ActorMovedEvent :
    public BaseEventData
{
private:
    const ActorID m_id;

public:
    static const EventType sm_eventType;

    ActorMovedEvent(ActorID p_id) : m_id(p_id) {}
    virtual ~ActorMovedEvent(void) {}

    virtual const char* VGetName(void) const { return "ActorMovedEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorDestroyedEvent(m_id)); }
    const ActorID& GetActorID(void) const { return m_id; }

};

