#include "StdAfx.h"
#include "GameApp.h"
#include "GameLogic.h"

GameApp::GameApp(void):
m_pActorFactory(0), m_pLogic(0), m_continue(true)
{
}


GameApp::~GameApp(void)
{
    m_pLogic->VDestroy();
    SAFE_DELETE(m_pLogic);
    SAFE_DELETE(m_pActorFactory);
}


bool GameApp::Init(void)
{
    m_pActorFactory = new ActorFactory;
    if(!m_pActorFactory)
    {
        LI_ERROR("ActorFactory initialization failed");
        return false;
    }
    m_pLogic = new GameLogic;
    if(!m_pLogic || !m_pLogic->VInit())
    {
        LI_ERROR("GameLogic initialization failed");
        return false;
    }
    return true;
}


void GameApp::OnNextFrame(void)
{
    LostIsland::g_pTimer->Next();
    unsigned long deltaMillis = LostIsland::g_pTimer->GetGameDeltaMillis();
    LostIsland::g_pProcessManager->UpdateProcesses(deltaMillis);

    LostIsland::g_pGraphics->Clear();

    // TODO: Dynamic testing stuff goes here and only here.

    // TODO: Main entry point
    m_pLogic->VUpdate(deltaMillis);

    LostIsland::g_pGraphics->Present();
}


LRESULT CALLBACK LostIsland::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //int wmId, wmEvent;
    //PAINTSTRUCT ps;
    //HDC hdc;

    static bool lostFocusInFullscreen = false;
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_INPUT:
        g_pInput->RawInput(lParam, wParam);
        break;
    case WM_SIZE:
        if(!g_pGraphics->OnWindowResized(LOWORD(lParam), HIWORD(lParam)))
        {
            // something went horribly wrong :O
            PostQuitMessage(0);
        }
        break;
    case WM_ACTIVATE:   // something happened with focus
        if(wParam == WA_INACTIVE)   // lost focus
        {
            g_pTimer->Pause();
            lostFocusInFullscreen = g_pGraphics->IsFullscreen();
            if(!g_pGraphics->SetFullscreen(false))
            {
                // something went horribly wrong :O
                PostQuitMessage(0);
            }
        }
        else                        // got focus
        {
            g_pTimer->Resume();
            if(lostFocusInFullscreen)
            {
                if(!g_pGraphics->SetFullscreen(true))
                {
                    // something went horribly wrong :O
                    PostQuitMessage(0);
                }
            }
            lostFocusInFullscreen = false;
        }
        break;
    case WM_SETFOCUS:

        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
