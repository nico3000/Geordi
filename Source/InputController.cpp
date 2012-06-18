#include "StdAfx.h"
#include "InputController.h"


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

    //GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    LPBYTE lpb = new BYTE[dwSize];
    if(lpb != 0) 
    {
        if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
        {
            //LI_WARNING("GetRawInputData does not return correct size.");
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


void InputController::Mouse(const RAWMOUSE& data)
{
    //LI_INFO("got mouse");
}


void InputController::Keyboard(const RAWKEYBOARD& data)
{
    //LI_INFO("got keyboard");
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