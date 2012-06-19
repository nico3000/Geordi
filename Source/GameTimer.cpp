#include "StdAfx.h"
#include "GameTimer.h"


GameTimer::GameTimer(void)
{
    m_sysFrequency = 0;
    m_sysAccumulator = 0;
    m_sysDeltaMillis = 0;

    m_gameFrequency = 0;
    m_gameAccumulator = 0;
    m_gameDeltaMillis = 0;

    m_lastStop = 0;
    m_paused = false;
}


GameTimer::~GameTimer(void)
{
}


bool GameTimer::Init(void)
{
    LARGE_INTEGER frequency, now;
    int success = QueryPerformanceFrequency(&frequency);
    m_sysFrequency = (double)frequency.QuadPart;
    QueryPerformanceCounter(&now);    
    m_lastStop = now.QuadPart;
    this->SetFactor(1.0f);
    return success != 0;
}


void GameTimer::Next(void)
{
    static LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    LONGLONG deltaTicks = now.QuadPart - m_lastStop;

    if(!m_paused)
    {
        LONGLONG sysDeltaTicks = deltaTicks + m_sysAccumulator;
        m_sysDeltaMillis = (unsigned long)((double)sysDeltaTicks / (1e-3 * m_sysFrequency));
        m_sysAccumulator = (unsigned long)(sysDeltaTicks - (LONGLONG)((double)m_sysDeltaMillis * (1e-3 * m_sysFrequency)));
        StopWatchMap::iterator rtend = m_realtime.end();
        for(StopWatchMap::iterator iter = m_realtime.begin(); iter != rtend; ++iter) 
        {
            iter->second += m_sysDeltaMillis;
        }

        LONGLONG gameDeltaTicks = deltaTicks + m_gameAccumulator;
        m_gameDeltaMillis = (unsigned long)((double)gameDeltaTicks / (1e-3 * m_gameFrequency));
        m_gameAccumulator = (unsigned long)(gameDeltaTicks - (LONGLONG)((double)m_gameDeltaMillis * (1e-3 * m_gameFrequency)));
        StopWatchMap::iterator gtend = m_gametime.end();
        for(StopWatchMap::iterator iter = m_gametime.begin(); iter != gtend; ++iter) 
        {
            iter->second += m_gameDeltaMillis;
        }
    }

    m_lastStop = now.QuadPart;
}


int GameTimer::Tick(INT id, TickOption p_option) 
{
    switch(m_types[id] = p_option)
    {
    case REALTIME: m_realtime[id] = 0; break;
    case GAMETIME: m_gametime[id] = 0; break;
    case IMMEDIATE:
        static LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        m_immediateStops[id] = now.QuadPart;
        break;
    }
    return id;
}


unsigned long GameTimer::Tock(INT p_id, TockOption p_option) 
{
    unsigned long time = 0;
    if(m_types.find(p_id) != m_types.end())
    {
        TickOption tick = m_types[p_id];
        StopWatchMap* pTarget = NULL;
        switch(tick)
        {
        case GAMETIME: pTarget = &m_gametime; break;
        case REALTIME: pTarget = &m_realtime; break;
        }
        if(pTarget == NULL)
        {
            static LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            time = (unsigned long)((double)(now.QuadPart - m_immediateStops[p_id]) / (1e-3 * m_sysFrequency));
        }
        else
        {
            time = (*pTarget)[p_id];
        }

        switch(p_option)
        {
        case ERASE:
            if(pTarget == NULL)
            {
                m_immediateStops.erase(p_id);
            }
            else
            {
                pTarget->erase(p_id);
            }
            break;
        case RESET: this->Tick(p_id, tick); break;
        case KEEPRUNNING: break;
        }
    }
    return time;
}


int GameTimer::GetTickTockID(void) const
{
    int id = rand();
    while(m_types.find(id) != m_types.end())
    {
        id = rand();
    }
    return id;
}
