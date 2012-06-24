#include "StdAfx.h"
#include "InputController.h"


static std::map<int,bool> g_keyPresses;
static long g_lastMouseX = 0;
static long g_lastMouseY = 0;
static long g_deltaMouseX = 0;
static long g_deltaMouseY = 0;


InputController::InputController(void)
{
}


InputController::~InputController(void)
{
}


bool InputController::Init(void)
{
    RAWINPUTDEVICE rid[2];

    // adds HID mouse
    rid[0].usUsagePage = 0x01; 
    rid[0].usUsage = 0x02; 
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = LostIsland::g_pGraphics->GetWindowHandle();

    // adds HID keyboard
    rid[1].usUsagePage = 0x01; 
    rid[1].usUsage = 0x06; 
    rid[1].dwFlags = 0;
    rid[1].hwndTarget = LostIsland::g_pGraphics->GetWindowHandle();

    if(!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        LI_ERROR("RegisterRawInputDevices() failed");
        return false;
    }

    SetCursor(0);

    return true;
}


void InputController::RawInput(LPARAM lParam, WPARAM wParam)
{
    UINT dwSize = 1024;

    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    LPBYTE lpb = new BYTE[dwSize];
    if(lpb != 0) 
    {
        if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
        {
            LI_WARNING("GetRawInputData does not return correct size.");
        }

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if(raw->header.dwType == RIM_TYPEKEYBOARD) 
        {
            this->Keyboard(raw->data.keyboard);
        }
        else if (raw->header.dwType == RIM_TYPEMOUSE) 
        {
            this->Mouse(raw->data.mouse);
        } 

        delete[] lpb;
    }
}


void InputController::UnregisterKeyboardHandler(StrongKeyboardHandlerPtr pKeyboardHandler)
{
    for(auto iter=m_keyboardHandlers.begin(); iter != m_keyboardHandlers.end(); ++iter)
    {
        if((*iter) == pKeyboardHandler)
        {
            m_keyboardHandlers.erase(iter);
            return;
        }
    }
}


void InputController::UnregisterPointerHandler(StrongPointerHandlerPtr pPointerHandler)
{
    for(auto iter=m_pointerHandlers.begin(); iter != m_pointerHandlers.end(); ++iter)
    {
        if((*iter) == pPointerHandler)
        {
            m_pointerHandlers.erase(iter);
            return;
        }
    }
}


void InputController::OnOpdate(void)
{
    if(g_deltaMouseX || g_deltaMouseY)
    {
        g_lastMouseX += g_deltaMouseX;
        g_lastMouseY += g_deltaMouseY;
        for(auto iter=m_pointerHandlers.begin(); iter != m_pointerHandlers.end(); ++iter)
        {
            if((*iter)->VOnPointerMoved(g_lastMouseX, g_lastMouseY, g_deltaMouseX, g_deltaMouseY))
            {
                break;
            }
        }
        g_deltaMouseX = g_deltaMouseY = 0;
    }
}


void InputController::Mouse(const RAWMOUSE& data)
{
    g_deltaMouseX += data.lLastX;
    g_deltaMouseY += data.lLastY;
}


void InputController::Keyboard(const RAWKEYBOARD& data)
{
    int keyCode = data.VKey;
    bool keyUp = data.Flags & RI_KEY_BREAK;
    if(keyUp)
    {
        g_keyPresses[keyCode] = false;
        for(auto iter=m_keyboardHandlers.begin(); iter != m_keyboardHandlers.end(); ++iter)
        {
            if((*iter)->VOnKeyUp(keyCode)) {
                return;
            }
        }
    }
    else
    {
        if(!g_keyPresses[keyCode])
        {
            g_keyPresses[keyCode] = true;
            for(auto iter=m_keyboardHandlers.begin(); iter != m_keyboardHandlers.end(); ++iter)
            {
                if((*iter)->VOnKeyDown(keyCode)) {
                    return;
                }
            }
        }
    }

    if(data.VKey == VK_ESCAPE)
    {
        PostQuitMessage(0);
    }
    if(data.VKey == VK_F1 && (data.Flags & RI_KEY_BREAK) == 0)
    {
        if(!LostIsland::g_pGraphics->ToggleFullscreen())
        {
            LI_ERROR("something went horribly wrong");
        }
    }
}