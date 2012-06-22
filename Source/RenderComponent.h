#pragma once
#include "Actor.h"
#include "Geometry.h"
#include "ConstantBuffer.h"
#include "EventManager.h"

class RenderComponent :
    public ActorComponent
{
public:
    enum GeometryType
    {
        GEOMETRY_CUBE,
    };

private:
    typedef std::list<StrongGeometryPtr> GeometryList;

    GeometryList m_geometries;
    ConstantBuffer m_model;

public:

    RenderComponent(void) {}
    ~RenderComponent(void) {}

    virtual bool VInit(tinyxml2::XMLElement* p_pData);
    virtual void VUpdate(unsigned long deltaMillis);
    StrongGeometryPtr GetGeometry(GeometryType p_type) const;
    void Draw(void) const;
    void OnActorMoved(IEventDataPtr pEvent);

    ComponentID VGetComponentID(void) const { return 0xd08283ae; }

};


