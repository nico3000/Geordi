#pragma once
class ConstantBuffer
{
private:
    ID3D11Buffer* m_pBuffer;
    void* m_pData;
    unsigned int m_byteWidth;

public:
    enum ShaderTarget
    {
        TARGET_VS = 1,
        TARGET_GS = 2,
        TARGET_PS = 4,
        TARGET_VS_PS = TARGET_VS | TARGET_PS,
        TARGET_ALL = TARGET_VS | TARGET_GS | TARGET_PS,
    };

    ConstantBuffer(void);
    ~ConstantBuffer(void);

    bool Build(void* p_pData, unsigned int p_byteWidth);
    void Bind(unsigned int p_slot, int p_target);
    bool Update(void);
};

