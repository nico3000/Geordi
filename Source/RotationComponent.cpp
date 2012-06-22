#include "StdAfx.h"
#include "RotationComponent.h"
#include "PoseComponent.h"


RotationComponent::RotationComponent(void) :
m_speed(0.0f, 0.0f, 0.0f)
{
}


RotationComponent::~RotationComponent(void)
{
}


bool RotationComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pPropertiesData = p_pData->FirstChildElement("Properties");
    if(pPropertiesData)
    {
        m_speed.x = pPropertiesData->FloatAttribute("pitchspeed");
        m_speed.y = pPropertiesData->FloatAttribute("yawspeed");
        m_speed.z = pPropertiesData->FloatAttribute("rollspeed");
        return true;
    }
    else
    {
        LI_ERROR("RotationComponent without Properties");
        return false;
    }
}


void RotationComponent::VUpdate(unsigned long deltaMillis)
{
    std::shared_ptr<PoseComponent> pPose = m_pOwner->GetComponent<PoseComponent>(PoseComponent::GetComponentID()).lock();
    if(pPose)
    {
        XMFLOAT3 currentRotation = pPose->GetRotation();
        currentRotation.x += m_speed.x * 1e-3f * XM_2PI * (float)deltaMillis;
        currentRotation.y += m_speed.y * 1e-3f * XM_2PI * (float)deltaMillis;
        currentRotation.z += m_speed.z * 1e-3f * XM_2PI * (float)deltaMillis;
        pPose->SetRotation(currentRotation.x, currentRotation.y, currentRotation.z);
    }
}