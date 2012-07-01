#pragma once

class ShaderProgram
{
private:
    ID3D11VertexShader* m_pVS;
    ID3D11GeometryShader* m_pGS;
    ID3D11PixelShader* m_pPS;
    ID3D11InputLayout* m_pLayout;
    ID3D10Blob* m_pVSBlob;

public:
    ShaderProgram(void);
    ~ShaderProgram(void);

    bool Load(LPCSTR p_file, LPCSTR p_functionVS, LPCSTR p_functionGS, LPCSTR p_functionPS, const D3D10_SHADER_MACRO* p_pDefines = 0, GraphicsLayer::ShaderVersion p_version = GraphicsLayer::SHADER_VERSION_MAX);
    bool CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* p_pInputElementDescs, UINT p_numElements);
    void Bind(void);
};

