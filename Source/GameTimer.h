#pragma once

enum TockOption {
    ERASE, RESET, KEEPRUNNING,
};

enum TickOption {
    REALTIME, GAMETIME, IMMEDIATE
};

typedef std::hash_map<INT,LONG> StopWatchMap;
typedef std::hash_map<INT,LONGLONG> ImmediateMap;
typedef std::hash_map<INT,TickOption> StopWatchTypeMap;

class GameTimer
{
private:
    double           m_sysFrequency;
    LONG             m_sysAccumulator;
    LONG             m_sysDeltaMillis;
    StopWatchMap     m_realtime;
    double           m_gameFrequency;
    LONG             m_gameAccumulator;
    LONG             m_gameDeltaMillis;
    StopWatchMap     m_gametime;
    LONGLONG         m_lastStop;
    bool             m_paused;
    ImmediateMap     m_immediateStops;
    StopWatchTypeMap m_types;

    INT GetTickTockID(void) const;
    INT Tick(INT p_id, TickOption p_option);
    
public:
    GameTimer(void);
    ~GameTimer(void);

    bool Init(void);
    void Next(void);
    LONG Tock(INT p_id, TockOption p_option);

    INT Tick(TickOption p_option) { return this->Tick(this->GetTickTockID(), p_option); }
    void Pause(void) { m_paused = true; }
    void Resume(void) { m_paused = false; }
    void SetFactor(double p_factor) { m_gameFrequency = m_sysFrequency / p_factor; }
    LONG GetGameDeltaMillis(void) const { return m_gameDeltaMillis; }
    LONG GetSysDeltaMillis(void) const { return m_sysDeltaMillis; }
};

