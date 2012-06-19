#include "StdAfx.h"
#include "DebugOutProcess.h"


DebugOutProcess::DebugOutProcess(std::string p_text)
{
    m_text = p_text;
}


DebugOutProcess::~DebugOutProcess(void)
{
}


void DebugOutProcess::VOnUpdate(unsigned long p_deltaMillis)
{
    OutputDebugStringA((m_text + "\n").c_str());
    this->Succeed();
}