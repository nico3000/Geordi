#pragma once
#include "IGameLogic.h"
#include "Actor.h"

class GameApp
{
private:
    bool m_continue;
    IGameLogic* m_pLogic;
    ActorFactory* m_pActorFactory;

public:
    GameApp(void);
    ~GameApp(void);

    bool Init(void);
    void OnNextFrame(void);

    bool IsContinued(void) const { return m_continue; }
    void SetContinue(bool p_continue) { m_continue = p_continue; }
    ActorFactory* GetActorFactory(void) { return m_pActorFactory; }
    IGameLogic* GetGameLogic(void) { return m_pLogic; }

};

namespace LostIsland
{
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}

