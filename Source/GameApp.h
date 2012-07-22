#pragma once
#include "GameLogic.h"
#include "IGameView.h"
#include "Actor.h"
#include "Config.h"

class GameApp
{
private:
    bool m_continue;
    GameLogic* m_pLogic;
    Config* m_pConfig;

public:
    GameApp(void);
    ~GameApp(void);

    bool InitConfig(void);
    bool Init(void);
    void OnNextFrame(void);

    bool IsContinued(void) const { return m_continue; }
    void SetContinue(bool p_continue) { m_continue = p_continue; }
    GameLogic* GetGameLogic(void) { return m_pLogic; }
    Config* GetConfig(void) const { return m_pConfig; }

};

namespace LostIsland
{
    LRESULT CALLBACK WndProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam);
}


