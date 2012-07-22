#include "StdAfx.h"
#include "RenderComponent.h"
#include "ActorEvents.h"
#include "MeshNode.h"


ComponentID RenderComponent::sm_componentID = 0xd08283ae;


bool RenderComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pGeoData = p_pData->FirstChildElement("Geometry");
    if(pGeoData)
    {
        std::string type(pGeoData->Attribute("type"));
        if(type.compare("cube") == 0)
        {
            m_properties.type = GEOMETRY_CUBE;
            m_properties.resource = "";
        }
        else if(type.compare("heightfield") == 0)
        {
            m_properties.type = GEOMETRY_HEIGHTFIELD;
            m_properties.resource = pGeoData->Attribute("file");
        }
        else
        {
            LI_ERROR(std::string("unknown geometry found: ") + pGeoData->Attribute("type"));
            return false;
        }
    }

    return true;
}


void RenderComponent::VPostInit(void)
{
    IEventDataPtr pEvent(new RenderComponentCreatedEvent(m_pOwner->GetID()));
    EventManager::Get()->VQueueEvent(pEvent);
}


std::shared_ptr<ISceneNode> RenderComponent::GetSceneNode(void)
{
    if(!m_pSceneNode)
    {
        m_pSceneNode.reset(new MeshNode(m_pOwner->GetID(), GetGeometry(m_properties.type)));
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
                    { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

                    { XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },

                    { XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },

                    { XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },

                    { XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },

                    { XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
                    { XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
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
                    0, 1, 2, 3, 0xFFFFFFFF,
                    4, 5, 6, 7, 0xFFFFFFFF,
                    8, 9, 10, 11, 0xFFFFFFFF,
                    12, 13, 14, 15, 0xFFFFFFFF,
                    16, 17, 18, 19, 0xFFFFFFFF,
                    20, 21, 22, 23, 0xFFFFFFFF,
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
    case GEOMETRY_HEIGHTFIELD:
        {
            unsigned int width = 1024;
            unsigned int height = 1024;
            VertexBuffer::SimpleVertex* pVertices = new VertexBuffer::SimpleVertex[(width+1) * (height+1)];
            unsigned int* pIndices = new unsigned int[height * (2 * (width + 1) + 1)];
            unsigned int currentIndex = 0;

            for(unsigned int y=0; y <= height; ++y)
            {
                for(unsigned int x=0; x <= width; ++x)
                {
                    VertexBuffer::SimpleVertex& vertex = pVertices[y * (width + 1) + x];
                    vertex.positionMC.x = 0.1f * ((float)x - 0.5f * (float)width);
                    vertex.positionMC.z = 0.1f * ((float)y - 0.5f * (float)height);
                    vertex.positionMC.y = -5.0f;

                    vertex.vertexColor = XMFLOAT4(
                        0.5f + 0.5f * sin(0.1f * x) * cos(0.3f * y),
                        0.5f + 0.5f * sin(0.2f * x) * cos(0.2f * y),
                        0.5f + 0.5f * sin(0.3f * x) * cos(0.1f * y), 1.0f);
                    if(y != height)
                    {
                        pIndices[currentIndex++] = (y + 1) * (width + 1) + x;
                        pIndices[currentIndex++] = y * (width + 1) + x;
                    }
                }
                if(y != height)
                {
                    pIndices[currentIndex++] = (unsigned int)(-1);
                }
            }

            Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
            Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
            pVertexBuffer->Build(pVertices, (width+1) * (height+1), sizeof(VertexBuffer::SimpleVertex));
            pIndexBuffer->Build(pIndices, currentIndex, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
            std::shared_ptr<Geometry> pGeo(new Geometry);
            pGeo->SetIndices(pIndexBuffer);
            pGeo->SetVertices(pVertexBuffer);
            
            SAFE_DELETE(pVertices);
            SAFE_DELETE(pIndices);
            return pGeo;
        }
        break;
    default:
        LI_ERROR(std::string("unkown actor geometry"));
        break;
    }
    return StrongGeometryPtr(0);
}