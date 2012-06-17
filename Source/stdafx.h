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

// used for status bar
#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib")

#include <tinyxml2.h>
#pragma comment(lib, "tinyxml2.lib")

#include <intsafe.h>
#include <memory>
#include <vector>
#include <list>
#include <hash_map>
#include <map>
#include <queue>
#include <string>
#include <iostream>
#include <streambuf>
#include <fstream>
#include <sstream>

//#define DXUT_AUTOLIB
//#include <DXUT.h>
//#pragma comment(lib, "DXUT.lib")

#ifndef DEBUG_NEW
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Logger.h"
#include "GameInitializer.h"

// defines
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SAFE_DELETE(_ptr) do    \
{                               \
    if((_ptr) != NULL)          \
    {                           \
        delete (_ptr);          \
        (_ptr) = NULL;          \
    }                           \
} while(0)

#define SAFE_DELETE_ARRAY(_ptr) do  \
{                                   \
    if((_ptr) != NULL)              \
    {                               \
        delete[] (_ptr);            \
        (_ptr) = NULL;              \
    }                               \
} while(0)

#define CLAMP(_val, _min, _max) max(_min, min(_val, _max))
#define LERP(_val, _min, _max) (((_val) - (_min)) / ((_max) - (_min)))
#define MIX(_val1, _val2, _t) ((1.0f - _t) * (_val1) + (_t) * (_val2))
