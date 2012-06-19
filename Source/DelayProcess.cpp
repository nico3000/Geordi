#include "StdAfx.h"
#include "DelayProcess.h"


DelayProcess::DelayProcess(unsigned long p_millisToDelay)
{
    m_millisToDelay = p_millisToDelay;
}


DelayProcess::~DelayProcess(void)
{
}


void DelayProcess::VOnUpdate(unsigned long p_deltaMillis)
{
    if(m_millisToDelay <= p_deltaMillis)
    {
        m_millisToDelay = 0;
        this->Succeed();
    }
    else
    {
        m_millisToDelay -= p_deltaMillis;
    }
}