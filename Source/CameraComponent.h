#pragma once
#include "actor.h"
class CameraComponent :
    public ActorComponent
{
private:
    float m_fov;
    float m_aspect;
    float m_minZ;
    float m_maxZ;

public:
    static ComponentID sm_componentID;

    CameraComponent(void) {  }
    ~CameraComponent(void) {  }

    bool VInit(tinyxml2::XMLElement* p_pData);

    virtual ComponentID VGetComponentID(void) const { return sm_componentID; };

    float GetFoV(void) const { return m_fov; }
    float GetAspectRatio(void) const { return m_aspect; }
    float GetMinZ(void) const { return m_minZ; }
    float GetMaxZ(void) const { return m_maxZ; }
};

