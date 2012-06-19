#pragma once

enum TockOption {
    ERASE, RESET, KEEPRUNNING,
};

enum TickOption {
    REALTIME, GAMETIME, IMMEDIATE
};

typedef std::hash_map<int,unsigned long> StopWatchMap;
typedef std::hash_map<int,LONGLONG> ImmediateMap;
typedef std::hash_map<int,TickOption> StopWatchTypeMap;

class GameTimer
{
private:
    double           m_sysFrequency;
    unsigned long    m_sysAccumulator;
    unsigned long    m_sysDeltaMillis;
    StopWatchMap     m_realtime;
    double           m_gameFrequency;
    unsigned long    m_gameAccumulator;
    unsigned long    m_gameDeltaMillis;
    StopWatchMap     m_gametime;
    LONGLONG         m_lastStop;
    bool             m_paused;
    ImmediateMap     m_immediateStops;
    StopWatchTypeMap m_types;

    int GetTickTockID(void) const;
    int Tick(int p_id, TickOption p_option);
    
public:
    GameTimer(void);
    ~GameTimer(void);

    bool Init(void);
    void Next(void);
    unsigned long Tock(INT p_id, TockOption p_option);

    int Tick(TickOption p_option) { return this->Tick(this->GetTickTockID(), p_option); }
    void Pause(void) { m_paused = true; }
    void Resume(void) { m_paused = false; }
    void SetFactor(double p_factor) { m_gameFrequency = m_sysFrequency / p_factor; }
    unsigned long GetGameDeltaMillis(void) const { return m_gameDeltaMillis; }
    unsigned long GetSysDeltaMillis(void) const { return m_sysDeltaMillis; }
};

