#pragma once
class XMMatrixStack
{
private:
    typedef std::vector<XMFLOAT4X4> MatrixVector;

    MatrixVector m_matrices;

public:
    XMMatrixStack(void);
    ~XMMatrixStack(void) {  }

    void PushMatrix(const XMFLOAT4X4& p_matrix);
    void PopMatrix(void) { if(m_matrices.size() > 1) m_matrices.pop_back(); else LI_ERROR("Can not pop matrix"); }
    const XMFLOAT4X4& Top(void) const { return m_matrices.back(); }

};

