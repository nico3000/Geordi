#pragma once
#include "eventmanager.h"

class TestEventData :
    public BaseEventData
{
public:
    static const EventType sm_eventType;

    TestEventData(void);
    ~TestEventData(void);

    virtual IEventDataPtr VCopy(void) const;

    virtual const EventType& VGetEventType(void) const { return sm_eventType; }
    virtual const char* VGetName(void) const { return "TestEvent"; }
};


