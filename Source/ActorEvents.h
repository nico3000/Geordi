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

    const char* VGetName(void) const { return "ActorCreatedEvent"; }
    const EventType& VGetEventType(void) const { return sm_eventType; }
    IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorCreatedEvent(m_id)); }
    ActorID GetActorID(void) { return m_id; }

};


class ActorDestroyedEvent :
    public BaseEventData
{
private:
    const ActorID m_id;

public:
    static const EventType sm_eventType;

    ActorDestroyedEvent(ActorID p_id) : m_id(p_id) {}
    ~ActorDestroyedEvent(void) {}

    const char* VGetName(void) const { return "ActorDestroyedEvent"; }
    const EventType& VGetEventType(void) const { return sm_eventType; }
    IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorDestroyedEvent(m_id)); }

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

    const char* VGetName(void) const { return "ActorMovedEvent"; }
    const EventType& VGetEventType(void) const { return sm_eventType; }
    IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorDestroyedEvent(m_id)); }
    const ActorID& GetActorID(void) const { return m_id; }

};


class ActorMoveEvent :
    public BaseEventData
{
private:
    const ActorID m_id;
    const XMFLOAT3 m_dTranslation;
    const XMFLOAT3 m_dRotation;
    const float m_dScaling;

public:
    static const EventType sm_eventType;

    ActorMoveEvent(ActorID p_id, const XMFLOAT3& p_dTranslation, const XMFLOAT3& p_dRotation, float p_dScaling) :
    m_id(p_id), m_dTranslation(p_dTranslation), m_dRotation(p_dRotation), m_dScaling(p_dScaling)
    {  }

    ~ActorMoveEvent(void) {}

    const char* VGetName(void) const { return "ActorMoveEvent"; }
    const EventType& VGetEventType(void) const { return sm_eventType; }
    IEventDataPtr VCopy(void) const { return IEventDataPtr(new ActorMoveEvent(m_id, m_dTranslation, m_dRotation, m_dScaling)); }
    const ActorID& GetActorID(void) const { return m_id; }
    const XMFLOAT3 GetDeltaTranslation(void) const { return m_dTranslation; }
    const XMFLOAT3 GetDeltaRotation(void) const { return m_dRotation; }
    float GetDeltaScaling(void) const { return m_dScaling; }

};


class RenderComponentCreatedEvent :
    public BaseEventData
{
private:
    const ActorID m_id;

public:
    static const EventType sm_eventType;

    RenderComponentCreatedEvent(ActorID p_id) :
    m_id(p_id)
    {  }

    ~RenderComponentCreatedEvent(void) {}

    virtual const char* VGetName(void) const { return "RenderComponentCreatedEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new RenderComponentCreatedEvent(m_id)); }
    const ActorID& GetActorID(void) const { return m_id; }

};


class ParticleComponentCreatedEvent :
    public BaseEventData
{
private:
    const ActorID m_id;

public:
    static const EventType sm_eventType;

    ParticleComponentCreatedEvent(ActorID p_id) :
    m_id(p_id)
    {  }

    ~ParticleComponentCreatedEvent(void) {}

    virtual const char* VGetName(void) const { return "ParticleComponentCreatedEvent"; }
    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual IEventDataPtr VCopy(void) const { return IEventDataPtr(new ParticleComponentCreatedEvent(m_id)); }
    const ActorID& GetActorID(void) const { return m_id; }

};