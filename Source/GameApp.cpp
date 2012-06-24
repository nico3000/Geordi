#include "StdAfx.h"
#include "GameApp.h"
#include "GameLogic.h"
#include "HumanDisplay.h"

GameApp::GameApp(void):
m_pLogic(0), m_pConfig(0), m_continue(true)
{
}


GameApp::~GameApp(void)
{
    m_pLogic->VDestroy();
    SAFE_DELETE(m_pLogic);
    SAFE_DELETE(m_pConfig);
}


bool GameApp::Init(void)
{

    m_pConfig = new Config;
    if(!m_pConfig->Init())
    {
        LI_ERROR("Config initialization error");
        return false;
    }
    m_pLogic = new GameLogic;
    if(!m_pLogic || !m_pLogic->VInit())
    {
        LI_ERROR("GameLogic initialization failed");
        return false;
    }
    StrongGameViewPtr pHumanDisp(new HumanDisplay);
    m_gameViews.push_back(pHumanDisp);
    return true;
}


void GameApp::OnNextFrame(void)
{
    LostIsland::g_pTimer->Next();
    unsigned long deltaMillis = LostIsland::g_pTimer->GetGameDeltaMillis();

    LostIsland::g_pInput->OnOpdate();
    m_pLogic->VUpdate(deltaMillis);
    
    for(auto iter=m_gameViews.begin(); iter != m_gameViews.end(); ++iter)
    {
        (*iter)->VOnUpdate(deltaMillis);
    }

    LostIsland::g_pGraphics->Clear();
    for(auto iter=m_gameViews.begin(); iter != m_gameViews.end(); ++iter)
    {
        (*iter)->VOnRender(deltaMillis);
    }
    LostIsland::g_pGraphics->Present();    
}


LRESULT CALLBACK LostIsland::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
