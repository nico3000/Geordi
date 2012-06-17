#include "StdAfx.h"
#include "GameInitializer.h"


GameTimer* LostIsland::g_pTimer = 0;
GraphicsLayer* LostIsland::g_pGraphics = 0;
GameApp* LostIsland::g_pApp = 0;
InputController* LostIsland::g_pInput = 0;


bool GameInitializer::Init(HINSTANCE hInstance)
{
#if defined _DEBUG | defined PROFILE
    Logger::Init("logging_debug.xml");
#else
    Logger::Init("logging_release.xml");
#endif

    LostIsland::g_pTimer = new GameTimer;
    if(LostIsland::g_pTimer == 0 || !LostIsland::g_pTimer->Init())
    {
        LI_ERROR("GameTimer initialization error");
        return false;
    }
    LostIsland::g_pGraphics = new GraphicsLayer;
    if(LostIsland::g_pGraphics == 0 || !LostIsland::g_pGraphics->Init(hInstance))
    {
        LI_ERROR("GraphicsLayer initialization error");
        return false;
    }
    LostIsland::g_pInput = new InputController;
    if(LostIsland::g_pInput == 0 || !LostIsland::g_pInput->Init())
    {
        LI_ERROR("InputController initialization error");
        return false;
    }
    LostIsland::g_pApp = new GameApp;
    if(LostIsland::g_pApp == 0 || !LostIsland::g_pApp->Init())
    {
        LI_ERROR("GameApp initialization error");
        return false;
    }
    return true;
}


void GameInitializer::Destroy(void)
{
    SAFE_DELETE(LostIsland::g_pApp);
    SAFE_DELETE(LostIsland::g_pInput);
    SAFE_DELETE(LostIsland::g_pGraphics);
    SAFE_DELETE(LostIsland::g_pTimer);
    Logger::Destroy();
}

