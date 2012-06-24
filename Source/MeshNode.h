#pragma once
#include "BaseSceneNode.h"
#include "Geometry.h"

class MeshNode :
    public BaseSceneNode
{
private:
    const std::shared_ptr<Geometry> m_pGeometry;

public:
    MeshNode(std::shared_ptr<Geometry> p_pGeometry) : m_pGeometry(p_pGeometry) { }
    ~MeshNode(void) { }

    HRESULT VOnRestore(void) { return S_OK; }
    HRESULT VOnLostDevice(void) { return S_OK; }

    HRESULT VRender(void);

};

