#pragma once

class IPointerHandler
{
public:
    IPointerHandler(void) {}
    virtual ~IPointerHandler(void) {}

    virtual bool VOnPointerMoved(int p_x, int p_y, int p_dx, int p_dy) = 0;
    virtual bool VOnButtonDown(unsigned int p_button) = 0;
    virtual bool VOnButtonUp(unsigned int p_button) = 0;

};

