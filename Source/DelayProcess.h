#pragma once
#include "ProcessManager.h"
class DelayProcess :
    public Process
{
private:
    unsigned long m_millisToDelay;

protected:
    virtual void VOnUpdate(unsigned long p_deltaMillis);

public:
    DelayProcess(unsigned long p_millisToDelay);
    ~DelayProcess(void);

};

