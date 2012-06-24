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
    const XMFLOAT3& GetTranslation(void) const { return m_translation; }
    const XMFLOAT3& GetScaling(void) const { return m_scaling; }

    void SetRotation(const XMFLOAT3& p_rotation) { m_rotation = p_rotation; 
    this->UpdateMatrices(); }

    void SetScaling(const XMFLOAT3& p_scaling) { m_scaling = p_scaling;
    this->UpdateMatrices(); }

    void SetTranslation(const XMFLOAT3& p_translation) { m_translation = p_translation;
    this->UpdateMatrices(); }

    void Scale(const XMFLOAT3& p_dScale) { m_scaling.x *= p_dScale.x;
    m_scaling.y *= p_dScale.y;
    m_scaling.z *= p_dScale.z;
    this->UpdateMatrices(); }

    void Translate(const XMFLOAT3& p_dTranslate) { m_translation.x += p_dTranslate.x;
    m_translation.y += p_dTranslate.y;
    m_translation.z += p_dTranslate.z;
    this->UpdateMatrices(); }

    void Rotate(const XMFLOAT3& p_dRotate) { m_rotation.x += p_dRotate.x;
    m_rotation.y += p_dRotate.y;
    m_rotation.z += p_dRotate.z;
    this->UpdateMatrices(); }

    static ComponentID GetComponentID(void) { return 0x6e128fc9; }
};
