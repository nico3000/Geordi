#pragma once
#include "GameTimer.h"
#include "GameApp.h"
#include "GraphicsLayer.h"
#include "InputController.h"

namespace LostIsland
{
    extern GameTimer* g_pTimer;
    extern GameApp* g_pApp;    
    extern GraphicsLayer* g_pGraphics;
    extern InputController* g_pInput;
}

namespace GameInitializer
{
    bool Init(HINSTANCE hInstance);
    void Destroy(void);
}
