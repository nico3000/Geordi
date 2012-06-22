#include "StdAfx.h"
#include "RenderComponent.h"
#include "PoseComponent.h"
#include "ActorEvents.h"

bool RenderComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pGeoData = p_pData->FirstChildElement("Geometry");
    while(pGeoData)
    {
        std::string type(pGeoData->Attribute("type"));
        if(type.compare("cube") == 0)
        {
            StrongGeometryPtr pGeo = this->GetGeometry(GEOMETRY_CUBE);
            if(pGeo)
            {
                m_geometries.push_back(pGeo);
            }
            else
            {
                LI_ERROR("error");
            }
        }
        else
        {
            LI_WARNING(std::string("unknown geometry found: ") + pGeoData->Attribute("type"));
        }
        pGeoData = pGeoData->NextSiblingElement("Geometry");
    }

    EventListenerDelegate del = fastdelegate::MakeDelegate(this, &RenderComponent::OnActorMoved);
    EventManager::Get()->VAddListener(del, ActorMovedEvent::sm_eventType);

    return true;
}


void RenderComponent::VUpdate(unsigned long deltaMillis)
{
    m_model.Bind(1, ConstantBuffer::TARGET_ALL);
    this->Draw();
}


void RenderComponent::OnActorMoved(IEventDataPtr pEvent)
{
    if(std::static_pointer_cast<ActorMovedEvent>(pEvent)->GetActorID() == m_pOwner->GetID()) {
        std::shared_ptr<PoseComponent> pPoseComponent = m_pOwner->GetComponent<PoseComponent>(PoseComponent::GetComponentID()).lock();
        if(pPoseComponent)
        {
            if(m_model.IsBuilt())
            {
                m_model.Update();
            }
            else
            {
                m_model.Build((void*)pPoseComponent->GetModelMatrices(), 2 * sizeof(XMFLOAT4X4));
            }
            
        }
        else
        {
            LI_ERROR("Actor does have a RenderComponent but no PoseComponent");
        }
    }
}


void RenderComponent::Draw() const
{
    for(auto iter=m_geometries.begin(); iter != m_geometries.end(); ++iter)
    {
        (*iter)->Draw();
    }
}


StrongGeometryPtr RenderComponent::GetGeometry(GeometryType p_type) const
{
    switch(p_type)
    {
    case GEOMETRY_CUBE:
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