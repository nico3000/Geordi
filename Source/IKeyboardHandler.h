#pragma once

class IKeyboardHandler
{
public:
    IKeyboardHandler(void) {}
    ~IKeyboardHandler(void) {}

    virtual bool VOnKeyUp(int p_keycode) = 0;
    virtual bool VOnKeyDown(int p_keycode) = 0;

};
