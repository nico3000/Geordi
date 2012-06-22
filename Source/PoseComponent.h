#pragma once
#include "Actor.h"

class PoseComponent :
    public ActorComponent
{
private:
    XMFLOAT3 m_translation;
    XMFLOAT3 m_rotation;
    XMFLOAT3 m_scaling;
    XMFLOAT4X4 m_pModelMatrix[2];

    void UpdateMatrices(void);

public:
    PoseComponent(void);
    ~PoseComponent(void);

    virtual bool VInit(tinyxml2::XMLElement* p_pData);
    virtual void VPostInit(void);
    void SetRotation(float p_pitch, float p_yaw, float p_roll);
    
    const XMFLOAT4X4* GetModelMatrices(void) const { return m_pModelMatrix; }
    ComponentID VGetComponentID(void) const { return GetComponentID(); }
    const XMFLOAT3& GetRotation(void) const { return m_rotation; }

    static ComponentID GetComponentID(void) { return 0x6e128fc9; }
};