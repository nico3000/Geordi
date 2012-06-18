#include "StdAfx.h"
#include "GameApp.h"


GameApp::GameApp(void):
m_pLogic(NULL), m_continue(true)
{
}


GameApp::~GameApp(void)
{
    SAFE_DELETE(m_pLogic);
}


bool GameApp::Init(void)
{
    return true;
}


void GameApp::OnNextFrame(void)
{
    LostIsland::g_pGraphics->Clear();
    // TODO: Dynamic testing stuff goes here and only here.

    // TODO: Main entry point
    //m_pLogic->VOnNextFrame();

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
