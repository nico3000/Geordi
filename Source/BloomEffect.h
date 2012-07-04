#pragma once
#include "iposteffect.h"
#include "RenderTarget.h"
class BloomEffect :
    public IPostEffect
{
private:
    std::weak_ptr<RenderTarget> m_pInput;
    unsigned int m_inputIndex;
    RenderTarget m_temp;
    ID3D11ComputeShader* m_pBlurHor;
    ID3D11ComputeShader* m_pBlurVer;

public:
    BloomEffect(std::weak_ptr<RenderTarget> p_pInput, unsigned int p_inputIndex);
    ~BloomEffect(void);

    HRESULT VOnRestore(void);
    HRESULT VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis);
    void VExecute(void);
};

