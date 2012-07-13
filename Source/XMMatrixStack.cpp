#include "StdAfx.h"
#include "XMMatrixStack.h"


XMMatrixStack::XMMatrixStack(void)
{
    XMFLOAT4X4 identity;
    XMStoreFloat4x4(&identity, XMMatrixIdentity());
    m_matrices.push_back(identity);
}


void XMMatrixStack::PushMatrix(const XMFLOAT4X4& p_matrix)
{
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, XMLoadFloat4x4(&p_matrix) * XMLoadFloat4x4(&m_matrices.back()));
    m_matrices.push_back(temp);
}