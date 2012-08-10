#include "StdAfx.h"
#include "TerrainComponent.h"
#include "ActorEvents.h"
#include "TerrainNode.h"


const ComponentID TerrainComponent::sm_componentID = 0x9b217029;


TerrainComponent::TerrainComponent(void) : m_pSceneNode(0), m_pTerrain(0)
{
}


TerrainComponent::~TerrainComponent(void)
{
}


bool TerrainComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pFolderData = p_pData->FirstChildElement("Folder");
    if(!pFolderData || !pFolderData->Attribute("value"))
    {
        LI_ERROR("invalid or no Folder element in TerrainComponent");
        return false;
    }
    std::string folderStr(pFolderData->Attribute("value"));
    m_pTerrain.reset(new TerrainData);
    
    tinyxml2::XMLElement* pData = p_pData->FirstChildElement("Data");
    unsigned int levels, octreesize;
    if(!pData ||
        pData->QueryUnsignedAttribute("levels", &levels) != tinyxml2::XML_NO_ERROR ||
        pData->QueryUnsignedAttribute("octreesize", &octreesize) != tinyxml2::XML_NO_ERROR)
    {
        LI_ERROR("invalid or no Data element in TerrainComponent");
        return false;
    }
    tinyxml2::XMLElement* pVisualData = p_pData->FirstChildElement("Visual");
    unsigned int  chunksize, smallradius;
    if(!pVisualData ||
        pVisualData->QueryUnsignedAttribute("chunksize", &chunksize) != tinyxml2::XML_NO_ERROR ||
        pVisualData->QueryUnsignedAttribute("smallradius", &smallradius) != tinyxml2::XML_NO_ERROR)        
    {
        LI_ERROR("invalid or no Visual element in TerrainComponent");
        return false;
    }

    if(!m_pTerrain->Init(folderStr, levels, octreesize, chunksize))
    {
        LI_ERROR("terrain initialization failed");
        return false;
    }
    m_pTerrain->GenerateTestData();
    m_pSceneNode.reset(new TerrainNode(m_pTerrain));    
    
    return true;
}


void TerrainComponent::VPostInit(void)
{
    IEventDataPtr pEvent(new TerrainComponentCreatedEvent(m_pOwner->GetID()));
    EventManager::Get()->VQueueEvent(pEvent);
}
