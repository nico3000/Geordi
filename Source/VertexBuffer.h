#pragma once

class VertexBuffer
{
private:
    ID3D11Buffer* m_pBuffer;
    unsigned int m_byteStride;

public:
    VertexBuffer(void);
    ~VertexBuffer(void);

    bool Build(void* p_data, unsigned int p_numVertices, unsigned int p_byteStride);
    void Bind(void);

    struct SimpleVertex
    {
        XMFLOAT3 positionMC;
        XMFLOAT3 normalMC;
        XMFLOAT4 vertexColor;
    };
    static D3D11_INPUT_ELEMENT_DESC sm_pSimpleVertexElementDesc[3];
    static unsigned int sm_simpleVertexNumElements;

    struct ScreenQuadVertex
    {
        XMFLOAT2 pos;
    };
    static D3D11_INPUT_ELEMENT_DESC sm_pScreenQuadVertexElementDesc[1];
    static unsigned int sm_screenQuadVertexNumElements;
};

