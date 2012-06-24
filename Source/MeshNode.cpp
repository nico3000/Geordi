#include "StdAfx.h"
#include "MeshNode.h"


HRESULT MeshNode::VRender(void)
{
    m_pGeometry->Draw();
    return S_OK;
}
