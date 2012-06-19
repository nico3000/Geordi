#include "StdAfx.h"
#include "GameInitializer.h"

// testing includes
#include "DelayProcess.h"
#include "DebugOutProcess.h"

Config* LostIsland::g_pConfig = 0;
GameTimer* LostIsland::g_pTimer = 0;
GraphicsLayer* LostIsland::g_pGraphics = 0;
GameApp* LostIsland::g_pApp = 0;
InputController* LostIsland::g_pInput = 0;
ProcessManager* LostIsland::g_pProcessManager = 0;


bool GameInitializer::Init(HINSTANCE hInstance)
{
#if defined _DEBUG | defined PROFILE
    Logger::Init("logging_debug.xml");
#else
    Logger::Init("logging_release.xml");
#endif

    LostIsland::g_pConfig = new Config;
    if(!LostIsland::g_pConfig->Init())
    {
        LI_ERROR("Config initialization error");
        return false;
    }
    LostIsland::g_pProcessManager = new ProcessManager;
    if(!LostIsland::g_pProcessManager)
    {
        LI_ERROR("ProcessManager initialization error");
        return false;
    }
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

    // static testing stuff goes here and only here.
    StrongProcessPtr pCountdown(new DelayProcess(1000));
    LostIsland::g_pProcessManager->AttachProcess(pCountdown);
    for(int i=5; i > 0; --i)
    {
        std::ostringstream str;
        str << i;
        
        StrongProcessPtr pTextProcess(new DebugOutProcess(str.str()));
        pCountdown->AttachChild(pTextProcess);
        StrongProcessPtr pNewCountdown(new DelayProcess(1000));
        pTextProcess->AttachChild(pNewCountdown);
        pCountdown = pNewCountdown;
    }
    StrongProcessPtr pTextProcess(new DebugOutProcess("Boooom!"));
    pCountdown->AttachChild(pTextProcess);

    return true;
}


void GameInitializer::Destroy(void)
{
    LostIsland::g_pProcessManager->AbortAllProcesses(true);
    SAFE_DELETE(LostIsland::g_pApp);
    SAFE_DELETE(LostIsland::g_pInput);
    SAFE_DELETE(LostIsland::g_pGraphics);
    SAFE_DELETE(LostIsland::g_pTimer);
    SAFE_DELETE(LostIsland::g_pProcessManager);
    SAFE_DELETE(LostIsland::g_pConfig);
    Logger::Destroy();
}

