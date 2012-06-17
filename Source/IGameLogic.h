#pragma once
class IGameLogic
{
public:
    //IGameLogic(void);
    //~IGameLogic(void);

    virtual bool VInit(void) = 0;
    virtual void VDestroy(void) = 0;
    virtual void VOnNextFrame(void) = 0;

};

