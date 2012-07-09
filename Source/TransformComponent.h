#pragma once
#include "actor.h"
#include "Pose.h"
class TransformComponent :
    public ActorComponent
{
private:
    Pose m_pose;

public:
    static ComponentID sm_componentID;

    TransformComponent(void);
    ~TransformComponent(void) { }

    bool VInit(tinyxml2::XMLElement* p_pData);
    //void VPostInit(void) {}
    //void VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis) {}

    ComponentID VGetComponentID(void) const { return sm_componentID; }
    Pose& GetPose(void) { return m_pose; }
};

