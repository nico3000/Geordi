#include "StdAfx.h"
#include "ProcessManager.h"


Process::Process(void)
{
    m_state = STATE_UNINITIALIZED;
}


Process::~Process(void)
{
}


void Process::Succeed(void)
{
    if(this->IsAlive())
    {
        m_state = STATE_SUCCEEDED;
    }
}


void Process::Fail(void)
{
    if(this->IsAlive())
    {
        m_state = STATE_FAILED;
    }
}


void Process::Pause(void)
{
    if(this->IsAlive())
    {
        m_state = STATE_PAUSED;
    }
}


void Process::Resume(void)
{
    if(this->IsPaused())
    {
        m_state = STATE_RUNNING;
    }
}


StrongProcessPtr Process::RemoveChild(void)
{
    StrongProcessPtr pTemp = m_pChild;
    m_pChild = 0;
    return pTemp;
}


ProcessManager::ProcessManager(void)
{
}


ProcessManager::~ProcessManager(void)
{
    this->ClearAllProcesses();
}


unsigned int ProcessManager::UpdateProcesses(unsigned long p_deltaMillis)
{
    unsigned short succeeded = 0;
    unsigned short failed = 0;

    ProcessList::iterator iter = m_processes.begin();
    while(iter != m_processes.end())
    {
        StrongProcessPtr pCurrentProcess = *iter;
        if(pCurrentProcess->GetState() == Process::STATE_UNINITIALIZED)
        {
            pCurrentProcess->VOnInit();
        }
        if(pCurrentProcess->GetState() == Process::STATE_RUNNING)
        {
            pCurrentProcess->VOnUpdate(p_deltaMillis);
        }
        if(pCurrentProcess->IsDead())
        {
            switch(pCurrentProcess->GetState())
            {
            case Process::STATE_ABORTED:
                pCurrentProcess->VOnAbort();
                ++failed;
                break;
            case Process::STATE_FAILED:
                pCurrentProcess->VOnFail();
                ++failed;
                break;
            case Process::STATE_SUCCEEDED:
                pCurrentProcess->VOnSuccess();
                StrongProcessPtr pChild = pCurrentProcess->RemoveChild();
                if(pChild)
                {
                    this->AttachProcess(pChild);
                }
                else
                {
                    ++succeeded;
                }
                break;
            }
            iter = m_processes.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    return ((unsigned int)succeeded << 16) | ((unsigned int)failed);
}


WeakProcessPtr ProcessManager::AttachProcess(StrongProcessPtr p_pProcess)
{
    m_processes.push_back(p_pProcess);
    return p_pProcess;
}


void ProcessManager::AbortAllProcesses(bool p_immediate)
{
    ProcessList::iterator iter = m_processes.begin();
    while(iter != m_processes.end())
    {
        StrongProcessPtr pCurrentProcess = *iter;
        pCurrentProcess->SetState(Process::STATE_ABORTED);
        if(p_immediate)
        {
            pCurrentProcess->VOnAbort();
        }
        ++iter;
    }
}
