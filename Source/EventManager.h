#pragma once

class IEventData;
typedef unsigned int EventType;
typedef std::shared_ptr<IEventData> IEventDataPtr;

class IEventData
{
public:
    typedef unsigned long TimeStamp;

    virtual const EventType& VGetEventType(void) const = 0;
    virtual TimeStamp VGetTimeStamp(void) const = 0;
    virtual void VSerialize(std::ostream& p_out) const = 0;
    virtual IEventDataPtr VCopy(void) const = 0;
    virtual const char* VGetName(void) const = 0;
};

class BaseEventData :
    public IEventData
{
private:
    const TimeStamp m_timeStamp;

public:
    explicit BaseEventData(const TimeStamp p_timeStamp = 0) : m_timeStamp(p_timeStamp) {}
    virtual ~BaseEventData(void) {}

    TimeStamp VGetTimeStamp(void) const { return m_timeStamp; }

    virtual const EventType& VGetEventType(void) const = 0;
    virtual void VSerialize(std::ostream& out) const {}
};


typedef fastdelegate::FastDelegate1<IEventDataPtr> EventListenerDelegate;
class IEventManager;
typedef std::shared_ptr<IEventManager> EventManagerPtr;

class IEventManager
{
public:
    enum EvtMgrConstants { EVT_MGR_INFINITE = 0xffffffff };

    explicit IEventManager(const char* p_name);
    virtual ~IEventManager(void);

    virtual bool VAddListener(const EventListenerDelegate& p_eventDelegate, const EventType& p_type) = 0;
    virtual bool VRemoveListener(const EventListenerDelegate& p_eventDelegate, const EventType& p_type) = 0;
    virtual bool VTriggerEvent(IEventDataPtr p_pEvent) const = 0;
    virtual bool VQueueEvent(IEventDataPtr p_pEvent) = 0;
    virtual bool VAbortEvent(const EventType& p_type, bool p_allOfType = false) = 0;
    virtual bool VUpdate(unsigned long p_maxMillis = INFINITE) = 0;

    static EventManagerPtr Get(void);

};


class EventManager :
    public IEventManager
{
private:
    typedef std::list<EventListenerDelegate> EventListenerList;
    typedef std::map<EventType,EventListenerList> EventListenerMap;
    typedef std::list<IEventDataPtr> EventQueue;

    EventListenerMap m_eventListeners;
    EventQueue m_eventQueue[2];
    int m_activeQueue;

public:
    EventManager(const char* p_name);
    ~EventManager(void);

    virtual bool VAddListener(const EventListenerDelegate& p_eventDelegate, const EventType& p_type);
    virtual bool VRemoveListener(const EventListenerDelegate& p_eventDelegate, const EventType& p_type);
    virtual bool VTriggerEvent(IEventDataPtr p_pEvent) const;
    virtual bool VQueueEvent(IEventDataPtr p_pEvent);
    virtual bool VAbortEvent(const EventType& p_type, bool p_allOfType = false);
    virtual bool VUpdate(unsigned long p_maxMillis = EVT_MGR_INFINITE);

};