#pragma once
class IndexBuffer
{
private:
    ID3D11Buffer* m_pBuffer;
    D3D11_PRIMITIVE_TOPOLOGY m_topology;

public:
    IndexBuffer(void);
    ~IndexBuffer(void);

    bool Build(unsigned int* p_data, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology);
    void Bind(void);
};
