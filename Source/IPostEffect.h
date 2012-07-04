#pragma once
class IPostEffect
{
public:
    virtual ~IPostEffect(void) { }

    virtual HRESULT VOnRestore(void) = 0;
    virtual HRESULT VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis) = 0;
    virtual void VExecute(void) = 0;
};

