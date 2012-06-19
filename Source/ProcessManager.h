#pragma once

class Process;
typedef std::shared_ptr<Process> StrongProcessPtr;
typedef std::weak_ptr<Process> WeakProcessPtr;

class Process
{
    friend class ProcessManager;

public:
    enum State
    {
        STATE_UNINITIALIZED,
        STATE_REMOVED,
        STATE_RUNNING,
        STATE_PAUSED,
        STATE_SUCCEEDED,
        STATE_FAILED,
        STATE_ABORTED,
    };

private:
    State m_state;
    StrongProcessPtr m_pChild;

    void SetState(State p_state) { m_state = p_state; }

protected:
    virtual void VOnInit(void) { m_state = STATE_RUNNING; }
    virtual void VOnUpdate(unsigned long p_deltaMillis) = 0;
    virtual void VOnSuccess(void) {}
    virtual void VOnFail(void) {}
    virtual void VOnAbort(void) {}

public:
    Process(void);
    virtual ~Process(void);

    void Succeed(void);
    void Fail(void);
    void Pause(void);
    void Resume(void);

    State GetState(void) const { return m_state; }
    bool IsAlive(void) const { return m_state == STATE_RUNNING || m_state == STATE_PAUSED; }
    bool IsDead(void) const { return m_state == STATE_ABORTED || m_state == STATE_FAILED || m_state == STATE_SUCCEEDED; }
    bool IsRemoved(void) const { return m_state == STATE_REMOVED; }
    bool IsPaused(void) const { return m_state == STATE_PAUSED; }

    void AttachChild(StrongProcessPtr p_pChild) { m_pChild = p_pChild; }
    StrongProcessPtr RemoveChild(void);
    StrongProcessPtr PeekChild(void) { return m_pChild; }

};

class ProcessManager
{
private:
    typedef std::list<StrongProcessPtr> ProcessList;

    ProcessList m_processes;

    void ClearAllProcesses(void) { m_processes.clear(); }

public:

    ProcessManager(void);
    ~ProcessManager(void);

    unsigned int UpdateProcesses(unsigned long p_deltaMillis);
    WeakProcessPtr AttachProcess(StrongProcessPtr p_pProcess);
    void AbortAllProcesses(bool p_immediate);

    unsigned int GetProcessCount(void) const { return (unsigned int)m_processes.size(); }
};

