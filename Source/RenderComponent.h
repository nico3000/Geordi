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
        GEOMETRY_HEIGHTFIELD,
    };

    struct GeometryProperties
    {
        GeometryType type;
        std::string resource;
    };

private:
    GeometryProperties m_properties;
    std::shared_ptr<ISceneNode> m_pSceneNode;

public:
    static ComponentID sm_componentID;

    RenderComponent(void) : m_pSceneNode(0) {}
    ~RenderComponent(void) {}

    bool VInit(tinyxml2::XMLElement* p_pData);
    void VPostInit(void);
    std::shared_ptr<ISceneNode> GetSceneNode(void);
    const GeometryProperties& GetProperties(void) const { return m_properties; }
    ComponentID VGetComponentID(void) const { return sm_componentID; }

    static StrongGeometryPtr GetGeometry(GeometryType p_type);

};
