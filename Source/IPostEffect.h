#pragma once
class IPostEffect
{
public:
    virtual ~IPostEffect(void) { }

    virtual HRESULT VOnRestore(void) = 0;
    virtual HRESULT VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis) = 0;
    virtual void VPreRender(void) = 0;
    virtual void VRender(void) = 0;
    virtual void VPostRender(void) = 0;
};

