#include "StdAfx.h"
#include "TransformComponent.h"


ComponentID TransformComponent::sm_componentID = 0xc6f09e1a;


TransformComponent::TransformComponent(void)
{
    
}


bool TransformComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pScaling = p_pData->FirstChildElement("Scaling");
    if(pScaling)
    {
        float scaling = 1.0f;
        pScaling->QueryFloatAttribute("scaling", &scaling);
        m_pose.SetScaling(scaling);            
    }
    tinyxml2::XMLElement* pPosition = p_pData->FirstChildElement("Position");
    if(pPosition)
    {
        XMFLOAT3 position;
        pPosition->QueryFloatAttribute("x", &position.x);
        pPosition->QueryFloatAttribute("y", &position.y);
        pPosition->QueryFloatAttribute("z", &position.z);
        m_pose.SetPosition(position);
    }
    tinyxml2::XMLElement* pRotation = p_pData->FirstChildElement("Rotation");
    if(pRotation)
    {
        XMFLOAT3 rotation(0.0f, 0.0f, 0.0f);
        pRotation->QueryFloatAttribute("pitch", &rotation.x);
        pRotation->QueryFloatAttribute("yaw", &rotation.y);
        pRotation->QueryFloatAttribute("roll", &rotation.z);
        m_pose.SetPitchYawRoll(rotation.x, rotation.y, rotation.z);
    }
    m_pose.UpdateMatrices();
    return true;
}
