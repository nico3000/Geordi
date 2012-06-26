#pragma once
class XMMatrixStack
{
private:
    typedef std::vector<XMMATRIX> MatrixVector;

    MatrixVector m_matrices;

public:
    XMMatrixStack(void) { m_matrices.push_back(XMMatrixIdentity()); }
    ~XMMatrixStack(void) {  }

    void PushMatrix(const XMMATRIX& p_matrix) { m_matrices.push_back(p_matrix * this->Top()); }
    void PopMatrix(void) { if(m_matrices.size() > 1) m_matrices.pop_back(); else LI_ERROR("cannot pop matrix"); }
    const XMMATRIX& Top(void) const { return m_matrices.back(); }

};

