// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRTDBG_MAP_ALLOC               // debug memory leaks

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <crtdbg.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <d3d11.h>
#include <D3DX11.h>
#include <xnamath.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#include <intsafe.h>
#include <memory>
#include <vector>
#include <list>
#include <hash_map>
#include <queue>
#include <string>
using std::wstring;
using std::string;

#include "DebugConsole.h"
#include "GameTimer.h"

namespace LostIsland {
    extern GameTimer g_timer;
    extern BOOL g_continue;
};

// defines
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SAFE_RELEASE(_resource){ if((_resource) != NULL) { (_resource)->Release(); (_resource) = NULL; } }
#define SAFE_DELETE(_ptr) { if((_ptr) != NULL) { delete (_ptr); (_ptr) = NULL; } }
#define RETURN_IF_FAILED(_hr, _errorMsg) { if(FAILED(_hr)) { ERROR(_errorMsg); return hr; } } // TODO (nico3000): output message
#define ERROR(_msg) { DebugConsole::PrintError(_msg, __FILE__, __LINE__); }
#define CLAMP(_val, _min, _max) max(_min, min(_val, _max))
#define LERP(_val, _min, _max) (((_val) - (_min)) / ((_max) - (_min)))
#define MIX(_val1, _val2, _t) ((1.0f - _t) * (_val1) + (_t) * (_val2))
