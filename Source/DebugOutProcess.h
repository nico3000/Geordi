#pragma once
#include "ProcessManager.h"

class DebugOutProcess :
    public Process
{
private:
    std::string m_text;

protected:
    virtual void VOnUpdate(unsigned long p_deltaMillis);

public:
    DebugOutProcess(std::string p_text);
    ~DebugOutProcess(void);


};
