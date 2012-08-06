// Geordi.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Geordi.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                    // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_hWnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
bool                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, unsigned int, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, unsigned int, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MSG msg;
    HACCEL hAccelTable;

    if(!GameInitializer::Init(hInstance))
    {
        GameInitializer::Destroy();
        return -1;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GEORDI));
    // Main message loop:
    while (LostIsland::g_pApp->IsContinued())
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if(msg.message == WM_QUIT)
            {
                LostIsland::g_pApp->SetContinue(false);
            }
        }
        else
        {
            LostIsland::g_pApp->OnNextFrame();
        }        
    }

    GameInitializer::Destroy();
    DestroyWindow(g_hWnd);

    //new INT[3];
    return 0;//(int)msg.wParam;
}
