#pragma once
#include "actor.h"
class RigidbodyComponent :
    public ActorComponent
{
private:
    bool m_isAsleep;
    physx::PxRigidBody* m_pBody;

public:
    static ComponentID sm_componentID;

    RigidbodyComponent(void);
    ~RigidbodyComponent(void);

    bool VInit(tinyxml2::XMLElement* p_pData);
    void VPostInit(void);
    void VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis);

    ComponentID VGetComponentID(void) const { return sm_componentID; }

};

