#pragma once
#include "iposteffect.h"
#include "RenderTarget.h"
#include "ShaderProgram.h"

class BloomEffectCS :
    public IPostEffect
{
private:
    std::weak_ptr<RenderTarget> m_pInput;
    std::weak_ptr<RenderTarget> m_pOutput;
    unsigned int m_inputIndex;
    RenderTarget m_temp;
    ID3D11ComputeShader* m_pBlurHor;
    ID3D11ComputeShader* m_pBlurVer;

public:
    BloomEffectCS(std::weak_ptr<RenderTarget> p_pInput, unsigned int p_inputIndex, std::weak_ptr<RenderTarget> p_pOutput);
    ~BloomEffectCS(void);

    HRESULT VOnRestore(void);
    HRESULT VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis);
    void VExecute(void);
};

class BloomEffectPS :
    public IPostEffect
{
private:
    std::weak_ptr<RenderTarget> m_pInput;
    std::weak_ptr<RenderTarget> m_pOutput;
    unsigned int m_inputIndex;
    RenderTarget m_temp;
    ShaderProgram m_blurHor;
    ShaderProgram m_blurVer;

public:
    BloomEffectPS(std::weak_ptr<RenderTarget> p_pInput, unsigned int p_inputIndex, std::weak_ptr<RenderTarget> p_pOutput);
    ~BloomEffectPS(void);

    HRESULT VOnRestore(void);
    HRESULT VOnUpdate(unsigned long m_deltaGameMillis, unsigned long m_deltaSysMillis);
    void VExecute(void);
};

