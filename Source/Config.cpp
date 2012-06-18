#include "StdAfx.h"
#include "Config.h"


Config::Config(void)
{
}


Config::~Config(void)
{
}


bool Config::Init(void)
{
    return m_doc.LoadFile("config.xml") == tinyxml2::XML_NO_ERROR;
}


tinyxml2::XMLElement* Config::GetSubsection(const char* p_section, const char* p_subsection)
{
    tinyxml2::XMLElement* pSection = m_doc.FirstChildElement(p_section);
    if(pSection)
    {
        tinyxml2::XMLElement* pSubsection = pSection->FirstChildElement(p_subsection);
        if(pSubsection)
        {
            return pSubsection;
        }
    }
    return 0;
}


bool Config::GetBoolAttribute(const char* p_section, const char* p_subsection, const char* p_attribute)
{
    tinyxml2::XMLElement* pElem = this->GetSubsection(p_section, p_subsection);
    return pElem ? pElem->BoolAttribute(p_attribute) : false;
}


int Config::GetIntAttribute(const char* p_section, const char* p_subsection, const char* p_attribute)
{
    tinyxml2::XMLElement* pElem = this->GetSubsection(p_section, p_subsection);
    return pElem ? pElem->IntAttribute(p_attribute) : 0;
}


float Config::GetFloatAttribute(const char* p_section, const char* p_subsection, const char* p_attribute)
{
    tinyxml2::XMLElement* pElem = this->GetSubsection(p_section, p_subsection);
    return pElem ? pElem->FloatAttribute(p_attribute) : 0.0f;
}


std::string Config::GetStringAttribute(const char* p_section, const char* p_subsection, const char* p_attribute)
{
    tinyxml2::XMLElement* pElem = this->GetSubsection(p_section, p_subsection);
    return std::string(pElem ? pElem->Attribute(p_attribute) : "");
}