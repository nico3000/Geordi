#include "StdAfx.h"
#include "ShaderProgram.h"


ShaderProgram::ShaderProgram(void):
m_pVS(0), m_pGS(0), m_pPS(0), m_pLayout(0), m_pVSBlob(0)
{
}


ShaderProgram::~ShaderProgram(void)
{
    SAFE_RELEASE(m_pLayout);
    SAFE_RELEASE(m_pVSBlob);
    SAFE_RELEASE(m_pVS);
    SAFE_RELEASE(m_pGS);
    SAFE_RELEASE(m_pPS);
}


bool ShaderProgram::Load(LPCSTR p_file, LPCSTR p_functionVS, LPCSTR p_functionGS, LPCSTR p_functionPS, const D3D10_SHADER_MACRO* p_pDefines /* = 0 */, GraphicsLayer::ShaderVersion p_version /* = GraphicsLayer::SHADER_VERSION_MAX */)
{
    SAFE_RELEASE(m_pLayout);
    SAFE_RELEASE(m_pVSBlob);
    SAFE_RELEASE(m_pVS);
    SAFE_RELEASE(m_pGS);
    SAFE_RELEASE(m_pPS);
    if(p_functionVS != 0)
    {
        m_pVS = LostIsland::g_pGraphics->CompileVertexShader(p_file, p_functionVS, m_pVSBlob, p_pDefines, p_version);
        if(m_pVS == 0)
        {
            return false;
        }
        else
        {
            m_pVSBlob->AddRef();
            m_pVS->AddRef();
        }
    }
    if(p_functionGS != 0)
    {
        m_pGS = LostIsland::g_pGraphics->CompileGeometryShader(p_file, p_functionGS, p_pDefines, p_version);
        if(m_pGS == 0)
        {
            return false;
        }
        else
        {
            m_pGS->AddRef();
        }
    }
    if(p_functionPS != 0)
    {
        m_pPS = LostIsland::g_pGraphics->CompilePixelShader(p_file, p_functionPS, p_pDefines, p_version);
        if(m_pPS == 0)
        {
            return false;
        }
        else
        {
            m_pPS->AddRef();
        }
    }
    return true;    
}


bool ShaderProgram::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* p_pInputElementDescs, UINT p_numElements)
{
    SAFE_RELEASE(m_pLayout);
    HRESULT hr = S_OK;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateInputLayout(p_pInputElementDescs, p_numElements, m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(), &m_pLayout);
    if(SUCCEEDED(hr))
    {
        //SAFE_RELEASE(m_pVSBlob);
    }
    else
    {
        LI_ERROR("CreateInputLayout() failed");
    }
    return SUCCEEDED(hr);
}


void ShaderProgram::Bind(void)
{
    LostIsland::g_pGraphics->GetContext()->VSSetShader(m_pVS, 0, 0);
    LostIsland::g_pGraphics->GetContext()->GSSetShader(m_pGS, 0, 0);
    LostIsland::g_pGraphics->GetContext()->PSSetShader(m_pPS, 0, 0);
    LostIsland::g_pGraphics->GetContext()->IASetInputLayout(m_pLayout);
}