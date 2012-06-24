#pragma once
#include "GameLogic.h"
#include "IGameView.h"
#include "Actor.h"
#include "Config.h"

class GameApp
{
private:
    typedef std::list<std::shared_ptr<IGameView>> GameViewList;

    bool m_continue;
    GameLogic* m_pLogic;
    Config* m_pConfig;
    GameViewList m_gameViews;

public:
    GameApp(void);
    ~GameApp(void);

    bool Init(void);
    void OnNextFrame(void);

    bool IsContinued(void) const { return m_continue; }
    void SetContinue(bool p_continue) { m_continue = p_continue; }
    GameLogic* GetGameLogic(void) { return m_pLogic; }
    Config* GetConfig(void) const { return m_pConfig; }

};

namespace LostIsland
{
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}


