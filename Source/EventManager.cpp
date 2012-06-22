#include "stdafx.h"
#include "EventManager.h"

#define LI_LOGGER_TAG "EventSystem"


EventManagerPtr IEventManager::Get(void)
{
    static EventManagerPtr pGlobalManager(new EventManager("global manager"));
    return pGlobalManager;
}


IEventManager::IEventManager(const char* p_name)
{
    LI_LOG_WITH_TAG("EventManager created: " + std::string(p_name));
}


IEventManager::~IEventManager(void)
{

}


EventManager::EventManager(const char* p_name) :
IEventManager(p_name), m_activeQueue(0)
{

}


EventManager::~EventManager(void)
{

}


bool EventManager::VAddListener(const EventListenerDelegate& p_eventDelegate, const EventType& p_type)
{
    EventListenerList& listenerList = m_eventListeners[p_type];
    for(auto iter=listenerList.begin(); iter != listenerList.end(); ++iter)
    {
        if(*iter == p_eventDelegate)
        {
            LI_LOG_WITH_TAG("duplicate registering of delegate");
            return false;
        }
    }
    listenerList.push_back(p_eventDelegate);
    return true;
}


bool EventManager::VRemoveListener(const EventListenerDelegate& p_eventDelegate, const EventType& p_type)
{
    EventListenerMap::iterator listIter = m_eventListeners.find(p_type);
    EventListenerList& listenerList = listIter->second;
    for(auto iter=listenerList.begin(); iter != listenerList.end(); ++iter)
    {
        if(*iter == p_eventDelegate)
        {
            listenerList.erase(iter);
            if(listenerList.empty())
            {
                m_eventListeners.erase(listIter);
            }
            return true;
        }
    }
    return false;
}


bool EventManager::VTriggerEvent(IEventDataPtr p_pEvent) const
{
    bool processed = false;
    auto findIt = m_eventListeners.find(p_pEvent->VGetEventType());
    if(findIt != m_eventListeners.end())
    {
        const EventListenerList& listeners = findIt->second;
        for(auto iter=listeners.begin(); iter != listeners.end(); ++iter)
        {
            (*iter)(p_pEvent);
            processed = true;
        }
    }
    return processed;
}


bool EventManager::VQueueEvent(IEventDataPtr p_pEvent)
{
    auto findIt = m_eventListeners.find(p_pEvent->VGetEventType());
    if(findIt != m_eventListeners.end())
    {
        m_eventQueue[m_activeQueue].push_back(p_pEvent);
        return true;
    }
    else
    {
        //LI_WARNING("new event ignored, because no one is interested: " + std::string(p_pEvent->VGetName()));
        return false;
    }
}


bool EventManager::VAbortEvent(const EventType& p_type, bool p_allOfType /* = false */)
{
    bool success = false;
    auto findIt = m_eventListeners.find(p_type);
    if(findIt != m_eventListeners.end())
    {
        EventQueue& activeQueue = m_eventQueue[m_activeQueue];
        auto iter = activeQueue.begin();
        while(iter != activeQueue.end())
        {
            if((*iter)->VGetEventType() == p_type)
            {
                iter = activeQueue.erase(iter);
                if(!p_allOfType)
                {
                    break;
                }
            }
            else
            {
                ++iter;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}


bool EventManager::VUpdate(unsigned long p_maxMillis /* = EVT_MGR_INFINITE */)
{
    unsigned long start = GetTickCount();

    int queueToProcess = m_activeQueue;
    m_activeQueue = (m_activeQueue + 1) % 2;
    m_eventQueue[m_activeQueue].clear();

    while(!m_eventQueue[queueToProcess].empty())
    {
        IEventDataPtr pEvent = m_eventQueue[queueToProcess].front();
        m_eventQueue[queueToProcess].pop_front();
        auto findIt = m_eventListeners.find(pEvent->VGetEventType());
        if(findIt != m_eventListeners.end())
        {
            EventListenerList& listeners = findIt->second;
            for(auto iter=listeners.begin(); iter != listeners.end(); ++iter)
            {
                (*iter)(pEvent);
            }
        }
        if(GetTickCount() - start > p_maxMillis)
        {
            break;
        }
    }
    if(!m_eventQueue[queueToProcess].empty())
    {
        LI_LOG_WITH_TAG("Remaining events enqueued");
        while(!m_eventQueue[queueToProcess].empty())
        {
            IEventDataPtr pEvent = m_eventQueue[queueToProcess].back();
            m_eventQueue[queueToProcess].pop_back();
            m_eventQueue[queueToProcess].push_front(pEvent);
        }
        return false;
    }
    else
    {
        return true;
    }
}