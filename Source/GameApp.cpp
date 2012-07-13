#include "StdAfx.h"
#include "GameApp.h"
#include "GameLogic.h"

GameApp::GameApp(void):
m_pLogic(0), m_pConfig(0), m_continue(true)
{
}


GameApp::~GameApp(void)
{
    if(m_pLogic)
    {
        m_pLogic->VDestroy();
    }
    SAFE_DELETE(m_pLogic);
    SAFE_DELETE(m_pConfig);
}


bool GameApp::InitConfig(void)
{
    m_pConfig = new Config;
    if(!m_pConfig->Init())
    {
        LI_ERROR("Config initialization error");
        return false;
    }
    return true;
}


bool GameApp::Init(void)
{
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
    static unsigned long gameMillis = 0;

    LostIsland::g_pTimer->Next();
    unsigned long deltaMillis = LostIsland::g_pTimer->GetGameDeltaMillis();
    gameMillis += deltaMillis;

    LostIsland::g_pPhysics->Update(deltaMillis, gameMillis);

    LostIsland::g_pInput->OnUpdate();
    m_pLogic->VUpdate(deltaMillis, gameMillis);    

    LostIsland::g_pGraphics->Clear();
    m_pLogic->VRender(deltaMillis);
    LostIsland::g_pGraphics->Present();

    LostIsland::g_pPhysics->FetchResults();
}


LRESULT CALLBACK LostIsland::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool lostFocusInFullscreen = false;
    static bool fullscreen = g_pGraphics->IsFullscreen();
    if(fullscreen != g_pGraphics->IsFullscreen())
    {
        ShowCursor(fullscreen);
        fullscreen ^= true;
    }
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
        else
        {
            if(g_pApp->GetGameLogic() && g_pGraphics && g_pGraphics->IsInitialized() &&  !g_pGraphics->IsOnShutdown())
            {
                g_pApp->GetGameLogic()->VRestore();
            }
            
        }
        return 1;
//     case WM_ACTIVATE:   // something happened with focus
//         if(wParam == WA_INACTIVE)   // lost focus
//         {
//             g_pTimer->Pause();
//             lostFocusInFullscreen = g_pGraphics->IsFullscreen();
//             if(!g_pGraphics->SetFullscreen(false))
//             {
//                 // something went horribly wrong :O
//                 PostQuitMessage(0);
//             }
//         }
//         else                        // got focus
//         {
//             g_pTimer->Resume();
//             if(lostFocusInFullscreen)
//             {
//                 if(!g_pGraphics->SetFullscreen(true))
//                 {
//                     // something went horribly wrong :O
//                     PostQuitMessage(0);
//                 }
//             }
//             lostFocusInFullscreen = false;
//         }
//         break;
    case WM_SETFOCUS:
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
