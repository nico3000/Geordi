#pragma once
#include "Config.h"
#include "GameTimer.h"
#include "GameApp.h"
#include "GraphicsLayer.h"
#include "InputController.h"
#include "ProcessManager.h"

namespace LostIsland
{
    extern Config* g_pConfig;
    extern GameTimer* g_pTimer;
    extern GameApp* g_pApp;    
    extern GraphicsLayer* g_pGraphics;
    extern InputController* g_pInput;
    extern ProcessManager* g_pProcessManager;
}

namespace GameInitializer
{
    bool Init(HINSTANCE hInstance);
    void Destroy(void);
}


