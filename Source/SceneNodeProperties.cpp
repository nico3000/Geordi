#include "StdAfx.h"
#include "SceneNodeProperties.h"


void SceneNodeProperties::Transform(XMFLOAT4X4* p_pModel, XMFLOAT4X4* p_pModelInv) const
{
    if(p_pModel)
    {
        XMStoreFloat4x4(p_pModel, m_model);
    }
    if(p_pModelInv)
    {
        XMStoreFloat4x4(p_pModelInv, m_modelInv);
    }
}