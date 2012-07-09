#include "StdAfx.h"
#include "GameInitializer.h"

// testing includes
#include "FPSActorInputHandler.h"
#include "HumanDisplay.h"
#include "RenderComponent.h"
#include "CameraComponent.h"
#include "ParticleComponent.h"
#include "MeshNode.h"

GameTimer* LostIsland::g_pTimer = 0;
GraphicsLayer* LostIsland::g_pGraphics = 0;
GameApp* LostIsland::g_pApp = 0;
InputController* LostIsland::g_pInput = 0;
PhysicsLayer* LostIsland::g_pPhysics = 0;


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
    LostIsland::g_pTimer->Pause();

    LostIsland::g_pApp = new GameApp;
    if(LostIsland::g_pApp == 0 || !LostIsland::g_pApp->InitConfig())
    {
        LI_ERROR("GameApp or Config initialization error");
        return false;
    }

    LostIsland::g_pGraphics = new GraphicsLayer;
    if(LostIsland::g_pGraphics == 0 || !LostIsland::g_pGraphics->Init(hInstance))
    {
        LI_ERROR("GraphicsLayer initialization error");
        return false;
    }

    LostIsland::g_pPhysics = new PhysicsLayer;
    if(!LostIsland::g_pPhysics || !LostIsland::g_pPhysics->Init())
    {
        LI_ERROR("PhysicsLayer initialization failed");
        return false;
    }

    if(!LostIsland::g_pApp->Init())
    {
        LI_ERROR("GameApp initialization error");
        return false;
    }

    LostIsland::g_pInput = new InputController;
    if(LostIsland::g_pInput == 0 || !LostIsland::g_pInput->Init())
    {
        LI_ERROR("InputController initialization error");
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    ////////////// static testing stuff goes here and only here. /////////////
    //////////////////////////////////////////////////////////////////////////
    //LostIsland::g_pTimer->SetFactor(0.1f);

    // human view
    std::shared_ptr<HumanDisplay> pHumanDisplay(new HumanDisplay);
    LostIsland::g_pApp->GetGameLogic()->AttachView(pHumanDisplay);

    // actor testing
    StrongActorPtr pCamera = LostIsland::g_pApp->GetGameLogic()->VCreateActor("./Actors/Camera.xml", 0);
    LostIsland::g_pApp->GetGameLogic()->VLoadGame("./Levels/NicotopiaTest.xml");

    // input testing
    std::shared_ptr<FPSActorInputHandler> pFPSHandler(new FPSActorInputHandler(pCamera->GetID()));
    LostIsland::g_pInput->RegisterKeyboardHandler(pFPSHandler);
    LostIsland::g_pInput->RegisterPointerHandler(pFPSHandler);
    LostIsland::g_pApp->GetGameLogic()->AttachView(pFPSHandler, pCamera->GetID());

    return true;
}


void GameInitializer::Destroy(void)
{
    SAFE_DELETE(LostIsland::g_pInput);
    SAFE_DELETE(LostIsland::g_pPhysics);
    SAFE_DELETE(LostIsland::g_pGraphics);
    SAFE_DELETE(LostIsland::g_pApp);
    SAFE_DELETE(LostIsland::g_pTimer);
    Logger::Destroy();
}

