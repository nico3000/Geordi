#pragma once
#include "Actor.h"
class RotationComponent :
    public ActorComponent
{
private:
    XMFLOAT3 m_speed;

public:
    RotationComponent(void);
    ~RotationComponent(void);

    virtual bool VInit(tinyxml2::XMLElement* p_pData);
    virtual void VUpdate(unsigned long deltaMillis);

    ComponentID VGetComponentID(void) const { return GetComponentID(); }

    static ComponentID GetComponentID(void) { return 0x1d00c3f0; }

};


