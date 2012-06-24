#pragma once
#include "eventmanager.h"

struct Transform
{
    bool absolute;
    XMFLOAT3 position;
    XMFLOAT3 rotation;
};

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


class ActorTranslateEvent :
    public BaseEventData
{
private:
    const ActorID m_id;
    const XMFLOAT3 m_translation;
    const bool m_absolute;

public:
    static const EventType sm_eventType;

    ActorTranslateEvent(ActorID p_id, const XMFLOAT3& p_translation, bool p_absolute) : m_id(p_id), m_translation(p_translation), m_absolute(p_absolute) {}
    virtual ~ActorTranslateEvent(void) {}

    virtual const char* VGetName(void) const { return "ActorTranslateEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorTranslateEvent(m_id, m_translation, m_absolute)); }
    const ActorID& GetActorID(void) const { return m_id; }
    const XMFLOAT3& GetTranslation(void) const { return m_translation; }
    bool IsAbsolute(void) const { return m_absolute; }

};


class ActorRotateEvent :
    public BaseEventData
{
private:
    const ActorID m_id;
    const XMFLOAT3 m_rotation;
    const bool m_absolute;

public:
    static const EventType sm_eventType;

    ActorRotateEvent(ActorID p_id, const XMFLOAT3& p_rotation, bool p_absolute) : m_id(p_id), m_rotation(p_rotation), m_absolute(p_absolute) {}
    virtual ~ActorRotateEvent(void) {}

    virtual const char* VGetName(void) const { return "ActorRotateEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorRotateEvent(m_id, m_rotation, m_absolute)); }
    const ActorID& GetActorID(void) const { return m_id; }
    const XMFLOAT3& GetRotation(void) const { return m_rotation; }
    bool IsAbsolute(void) const { return m_absolute; }

};
