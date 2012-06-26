#include "StdAfx.h"
#include "RenderComponent.h"
#include "PoseComponent.h"
#include "ActorEvents.h"
#include "MeshNode.h"

bool RenderComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pGeoData = p_pData->FirstChildElement("Geometry");
    if(pGeoData)
    {
        std::string type(pGeoData->Attribute("type"));
        if(type.compare("cube") == 0)
        {
            m_properties.type = GEOMETRY_CUBE;
            m_properties.resource = 0;
        }
        else
        {
            LI_ERROR(std::string("unknown geometry found: ") + pGeoData->Attribute("type"));
            return false;
        }
    }

    return true;
}


std::shared_ptr<ISceneNode> RenderComponent::GetSceneNode(void)
{
    if(!m_pSceneNode)
    {
        static XMFLOAT4X4 identity(1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f);
        m_pSceneNode.reset(new MeshNode(m_pOwner->GetID(), identity, GetGeometry(m_properties.type)));
    }
    return m_pSceneNode;
}


StrongGeometryPtr RenderComponent::GetGeometry(GeometryType p_type)
{
    switch(p_type)
    {
    case RenderComponent::GEOMETRY_CUBE:
        {
            static std::weak_ptr<Geometry> pCube;
            StrongGeometryPtr pStrongCube = pCube.lock();
            if(!pStrongCube)
            {
                pStrongCube.reset(new Geometry);
                Geometry::IndexBufferPtr pIndices(new IndexBuffer);
                Geometry::VertexBufferPtr pVertices(new VertexBuffer);

                VertexBuffer::SimpleVertex vertices[] = {
                    { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
                };
                if(!pVertices->Build(vertices, ARRAYSIZE(vertices), sizeof(VertexBuffer::SimpleVertex)))
                {
                    LI_ERROR("cube vertex buffer error");
                    return StrongGeometryPtr(0);
                }
                else
                {
                    pStrongCube->SetVertices(pVertices);
                }

                unsigned int indices[] = {
                    0, 1, 3, 2, 7, 6, 4, 5, 0, 1, 0xFFFFFFFF,
                    1, 5, 2, 6, 0xFFFFFFFF,
                    3, 7, 0, 4,
                };
                if(!pIndices->Build(indices, ARRAYSIZE(indices), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP))
                {
                    LI_ERROR("cube index buffer error");
                    return StrongGeometryPtr(0);
                }
                else
                {
                    pStrongCube->SetIndices(pIndices);
                }
                pCube = pStrongCube;
            }
            return pStrongCube;
        }
        break;
    default:
        LI_ERROR(std::string("unkown actor geometry"));
        break;
    }
    return StrongGeometryPtr(0);
}