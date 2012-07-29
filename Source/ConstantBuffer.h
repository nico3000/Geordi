#pragma once
#include "GraphicsLayer.h"
class ConstantBuffer
{
private:
    ID3D11Buffer* m_pBuffer;
    void* m_pData;
    bool m_ownData;
    unsigned int m_byteWidth;

    bool Build(void);

public:
    ConstantBuffer(void);
    ~ConstantBuffer(void);

    bool CopyDataAndBuild(const void* p_pData, unsigned int p_byteWidth);
    bool BuildFromSharedData(void* p_pData, unsigned int p_byteWidth);
    void Bind(unsigned int p_slot, ShaderTarget p_target);
    bool Update(void);
    bool UpdateAndBind(unsigned int p_slot, ShaderTarget p_target) { if(!this->Update()) return false; this->Bind(p_slot, p_target); return true; }

    void* GetData(void) { return m_pData; }
    bool IsBuilt(void) const { return m_pData != 0; }
};
