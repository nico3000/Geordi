#pragma once

class Config
{
private:
    tinyxml2::XMLDocument m_doc;

    tinyxml2::XMLElement* GetSubsection(const char* p_section, const char* p_subsection);

public:
    Config(void);
    ~Config(void);

    bool Init(void);

    int GetIntAttribute(const char* p_section, const char* p_subsection, const char* p_attribute);
    bool GetBoolAttribute(const char* p_section, const char* p_subsection, const char* p_attribute);
    float GetFloatAttribute(const char* p_section, const char* p_subsection, const char* p_attribute);
    std::string GetStringAttribute(const char* p_section, const char* p_subsection, const char* p_attribute);

};


