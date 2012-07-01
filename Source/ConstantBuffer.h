#pragma once
class ConstantBuffer
{
private:
    ID3D11Buffer* m_pBuffer;
    void* m_pData;
    bool m_ownData;
    unsigned int m_byteWidth;

    bool Build(void);

public:
    enum ShaderTarget
    {
        TARGET_VS = 1,
        TARGET_GS = 2,
        TARGET_PS = 4,
        TARGET_CS = 8,
        TARGET_VS_PS = TARGET_VS | TARGET_PS,
        TARGET_ALL = TARGET_VS | TARGET_GS | TARGET_PS | TARGET_CS,
    };

    ConstantBuffer(void);
    ~ConstantBuffer(void);

    bool CopyDataAndBuild(const void* p_pData, unsigned int p_byteWidth);
    bool BuildFromSharedData(void* p_pData, unsigned int p_byteWidth);
    void Bind(unsigned int p_slot, ShaderTarget p_target);
    bool Update(void);

    void* GetData(void) { return m_pData; }
    bool IsBuilt(void) const { return m_pData != 0; }
};
