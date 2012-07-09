#include "StdAfx.h"
#include "RigidbodyComponent.h"
#include "ActorEvents.h"
#include "TransformComponent.h"


ComponentID RigidbodyComponent::sm_componentID = 0x2819eea9;


RigidbodyComponent::RigidbodyComponent(void):
m_pBody(0), m_isAsleep(false)
{
}


RigidbodyComponent::~RigidbodyComponent(void)
{
    if(m_pBody && LostIsland::g_pPhysics)
    {
        LostIsland::g_pPhysics->GetScene()->removeActor(*m_pBody);        
    }
}


void RigidbodyComponent::VUpdate(unsigned long p_deltaMillis, unsigned long p_gameMillis)
{
    std::shared_ptr<TransformComponent> pTransform = m_pOwner->GetComponent<TransformComponent>(TransformComponent::sm_componentID).lock();
    physx::PxMat44 physxMat = physx::PxMat44(m_pBody->getGlobalPose());
    pTransform->GetPose().SetMatrices(XMFLOAT4X4((float*)&physxMat), &XMFLOAT4X4((float*)&physxMat.inverseRT()));
}


bool RigidbodyComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    m_pBody = LostIsland::g_pPhysics->GetPhysics()->createRigidDynamic(physx::PxTransform(physx::PxVec3(0,0,0)));
    tinyxml2::XMLElement* pShapeData = p_pData->FirstChildElement("Shape");
    while(pShapeData != 0)
    {
        std::string type = pShapeData->Attribute("type");
        if(type.compare("box") == 0)
        {
            tinyxml2::XMLElement* pHalfextentsData = pShapeData->FirstChildElement("Halfextents");
            tinyxml2::XMLElement* pCenterData = pShapeData->FirstChildElement("Center");
            if(pHalfextentsData && pCenterData)
            {
                physx::PxVec3 halfextents(0.0f, 0.0f, 0.0f);
                physx::PxVec3 center(0.0f, 0.0f, 0.0f);
                if(pHalfextentsData->QueryFloatAttribute("x", &halfextents.x) || pHalfextentsData->QueryFloatAttribute("y", &halfextents.y) || pHalfextentsData->QueryFloatAttribute("z", &halfextents.z))
                {
                    LI_WARNING("illegal Size attributes for cubetype Shape");
                }
                if(pCenterData->QueryFloatAttribute("x", &center.x) || pCenterData->QueryFloatAttribute("y", &center.y) || pCenterData->QueryFloatAttribute("z", &center.z))
                {
                    LI_WARNING("illegal Center attributes for cubetype Shape");
                }
                physx::PxShape* pShape = m_pBody->createShape(physx::PxBoxGeometry(halfextents), *LostIsland::g_pPhysics->GetPhysics()->createMaterial(0.5f, 0.5f, 0.5f), physx::PxTransform(center));
            }
            else
            {
                LI_WARNING("no Size or Center element for cubetype Shape");
            }
        }
        else
        {
            LI_WARNING("unknown Shape type: " + type);
        }
        pShapeData = pShapeData->NextSiblingElement("Shape");
    }
    physx::PxRigidBodyExt::updateMassAndInertia(*m_pBody, 1.0f);
    return true;
}


void RigidbodyComponent::VPostInit(void)
{
    std::shared_ptr<TransformComponent> pTransform = m_pOwner->GetComponent<TransformComponent>(TransformComponent::sm_componentID).lock();
    if(!pTransform)
    {
        LI_ERROR("RigidbodyComponent without TransformComponent");
        return;
    }
    XMFLOAT4X4 pose;
    XMStoreFloat4x4(&pose, pTransform->GetPose().GetModelMatrixBuffer(true).model);
    physx::PxTransform transform(physx::PxMat44((float*)&pose));
    m_pBody->setGlobalPose(transform);
    LostIsland::g_pPhysics->GetScene()->addActor(*m_pBody);
}
