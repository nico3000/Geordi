#pragma once

class IndexBuffer
{
private:
    ID3D11Buffer* m_pBuffer;
    D3D11_PRIMITIVE_TOPOLOGY m_topology;
    unsigned int m_indexCount;

public:
    IndexBuffer(void);
    ~IndexBuffer(void);

    bool Build(const unsigned int* p_data, unsigned int p_indexCount, D3D11_PRIMITIVE_TOPOLOGY p_topology);
    void Bind(void) const;

    unsigned int GetIndexCount(void) const { return m_indexCount; }
};

