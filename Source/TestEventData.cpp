#include "StdAfx.h"
#include "TestEventData.h"


const EventType TestEventData::sm_eventType = 0;


TestEventData::TestEventData(void)
{
}


TestEventData::~TestEventData(void)
{
}


IEventDataPtr TestEventData::VCopy(void) const
{
    IEventDataPtr pCopy(new TestEventData);
    return pCopy;
}
