#include "StdAfx.h"
#include "CameraComponent.h"


ComponentID CameraComponent::sm_componentID = 0x9ec682e9;


bool CameraComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pSettings = p_pData->FirstChildElement("Settings");
    if(pSettings)
    {
        m_fov = XM_PI * pSettings->FloatAttribute("fov") / 180.0f;
        if(std::string("auto").compare(pSettings->Attribute("aspect")))
        {
            m_aspect = 0.0f;
        }
        else
        {
            m_aspect = pSettings->FloatAttribute("aspect");
        }
        m_minZ = pSettings->FloatAttribute("minZ");
        m_maxZ = pSettings->FloatAttribute("maxZ");
        return true;
    }
    else
    {
        LI_ERROR("CameraComponent without Settings element");
        return false;
    }
}