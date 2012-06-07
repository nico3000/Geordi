#pragma once
#include "DebugConsole.h"
#include "GameTimer.h"

namespace LostIsland
{
    extern BOOL g_continue;
    extern GameTimer* g_pTimer;

    extern IDXGISwapChain* g_pSwapChain;
    extern ID3D11Device* g_pDevice;
    extern ID3D11DeviceContext* g_pContext;

    extern D3D_FEATURE_LEVEL g_featureLevel;

    class GameInitializer
    {
    private:
        BOOL InitDirect3D(HWND hWnd);

    public:
        GameInitializer(VOID);
        ~GameInitializer(VOID);

        BOOL Init(HWND hWnd);
    };
}


