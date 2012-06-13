#include "StdAfx.h"
#include "GameInitializer.h"
#include "TerrainData.h"

namespace LostIsland
{
    BOOL g_continue = TRUE;
    GameTimer* g_pTimer = NULL;

    IDXGISwapChain* g_pSwapChain = NULL;
    ID3D11Device* g_pDevice = NULL;
    ID3D11DeviceContext* g_pContext = NULL;

    D3D_FEATURE_LEVEL g_featureLevel;

    GameInitializer::GameInitializer(VOID)
    {
    }


    GameInitializer::~GameInitializer(VOID)
    {
        SAFE_RELEASE(g_pContext);
        SAFE_RELEASE(g_pDevice);
        SAFE_RELEASE(g_pSwapChain);

        SAFE_DELETE(g_pTimer);
    }


    BOOL GameInitializer::Init(HWND hWnd)
    {
#ifdef _DEBUG
        Logger::Init(hWnd, "logging.xml", "debug");
#elif defined PROFILE
        Logger::Init(hWnd, "logging.xml", "profile");
#else
        Logger::Init(hWnd, "logging.xml", "release");
#endif
        
        g_pTimer = new GameTimer();
        g_pTimer->Init();

        if(!this->InitDirect3D(hWnd))
        {
            LI_TAG_ERROR("general", "Direct3D initialization messed up");
            return FALSE;
        }

        // TODO: Static testing stuff goes here and only here.
        TerrainData terrain;
        terrain.Init(32, 8, 4, 8, 8);
        terrain.Test();
        //g_continue = FALSE;
        
        return TRUE;
    }


    BOOL GameInitializer::InitDirect3D(HWND hWnd)
    {
        HRESULT hr = S_OK;

//        DXGI_SWAP_CHAIN_DESC scDesc;
//        scDesc.BufferCount = 1;
//        scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
//        scDesc.BufferDesc.Width = SCREEN_WIDTH;
//        scDesc.BufferDesc.Height = SCREEN_HEIGHT;
//        scDesc.BufferDesc.RefreshRate.Numerator = 120;
//        scDesc.BufferDesc.RefreshRate.Denominator = 1;
//        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//        scDesc.Flags = 0;
//        scDesc.OutputWindow = hWnd;
//        scDesc.SampleDesc.Count = 1;
//        scDesc.SampleDesc.Quality = 0;
//        scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//        scDesc.Windowed = TRUE;
//        UINT flags = 0;
//#if defined(_DEBUG)
//        flags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//        D3D_FEATURE_LEVEL pFeatureLevels[] = {
//            D3D_FEATURE_LEVEL_11_0,
//            D3D_FEATURE_LEVEL_10_1,
//            D3D_FEATURE_LEVEL_10_0,
//        };
//        hr = D3D11CreateDeviceAndSwapChain(NULL,
//                                           D3D_DRIVER_TYPE_HARDWARE,
//                                           NULL,
//                                           flags,
//                                           pFeatureLevels,
//                                           ARRAYSIZE(pFeatureLevels),
//                                           D3D11_SDK_VERSION,
//                                           &scDesc,
//                                           &g_pSwapChain,
//                                           &g_pDevice,
//                                           &g_featureLevel,
//                                           &g_pContext);
//        RETURN_IF_FAILED(hr, "D3D11CreateDeviceAndSwapChain() failed");
//        switch(g_featureLevel)
//        {
//        case D3D_FEATURE_LEVEL_11_0: DebugConsole::PrintInfo("Your GPU supports Direct3D 11"); break;
//        case D3D_FEATURE_LEVEL_10_1: DebugConsole::PrintWarning("Your GPU does not support Direct3D 11, falling back to 10.1"); break;
//        case D3D_FEATURE_LEVEL_10_0: DebugConsole::PrintWarning("Your GPU does not support Direct3D 11, falling back to 10.0"); break;
//        }

        return SUCCEEDED(hr);
    }
}

