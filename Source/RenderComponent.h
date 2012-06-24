#pragma once
#include "Actor.h"
#include "Geometry.h"
#include "ConstantBuffer.h"
#include "EventManager.h"
#include "ISceneNode.h"

class RenderComponent :
    public ActorComponent
{
public:
    enum GeometryType
    {
        GEOMETRY_CUBE,
    };

    struct GeometryProperties
    {
        GeometryType type;
        const char* resource;
    };

private:
    GeometryProperties m_properties;
    std::shared_ptr<ISceneNode> m_pSceneNode;

public:
    RenderComponent(void) : m_pSceneNode(0) {}
    ~RenderComponent(void) {}

    virtual bool VInit(tinyxml2::XMLElement* p_pData);
    std::shared_ptr<ISceneNode> GetSceneNode(void);
    const GeometryProperties& GetProperties(void) const { return m_properties; }

    ComponentID VGetComponentID(void) const { return 0xd08283ae; }

};
