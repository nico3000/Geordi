#pragma once
#include "IGameLogic.h"

class GameApp
{
private:
    bool m_continue;
    IGameLogic* m_pLogic;

public:
    GameApp(void);
    ~GameApp(void);

    bool Init(void);
    void OnNextFrame(void);

    bool IsContinued(void) const { return m_continue; }
    void SetContinue(bool p_continue) { m_continue = p_continue; }

};

namespace LostIsland
{
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}

