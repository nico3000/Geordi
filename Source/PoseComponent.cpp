#include "StdAfx.h"
#include "PoseComponent.h"
#include "ActorEvents.h"


PoseComponent::PoseComponent(void) :
m_translation(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scaling(1.0f, 1.0f, 1.0f)
{
    XMStoreFloat4x4(&m_pModelMatrix[0], XMMatrixIdentity());
    XMStoreFloat4x4(&m_pModelMatrix[1], XMMatrixIdentity());
}


PoseComponent::~PoseComponent(void)
{
}


bool PoseComponent::VInit(tinyxml2::XMLElement* p_pData)
{
    tinyxml2::XMLElement* pInitialTransformData = p_pData->FirstChildElement("InitialTransform");
    if(pInitialTransformData)
    {
        tinyxml2::XMLElement* pTranslationData = pInitialTransformData->FirstChildElement("Translation");
        tinyxml2::XMLElement* pRotationData = pInitialTransformData->FirstChildElement("Rotation");
        tinyxml2::XMLElement* pScaleData = pInitialTransformData->FirstChildElement("Scale");
        if(pTranslationData)
        {
            m_translation.x = pTranslationData->FloatAttribute("x");
            m_translation.y = pTranslationData->FloatAttribute("y");
            m_translation.z = pTranslationData->FloatAttribute("z");
        }
        if(pRotationData)
        {
            m_rotation.x = pRotationData->FloatAttribute("pitch");
            m_rotation.y = pRotationData->FloatAttribute("yaw");
            m_rotation.z = pRotationData->FloatAttribute("roll");
        }
        if(pScaleData)
        {
            m_scaling.x = pScaleData->FloatAttribute("x");
            m_scaling.y = pScaleData->FloatAttribute("y");
            m_scaling.z = pScaleData->FloatAttribute("z");
        }
    }
    return true;
}


void PoseComponent::VPostInit(void)
{
    this->UpdateMatrices();
}


void PoseComponent::UpdateMatrices(void)
{
    XMVECTOR det;
    XMMATRIX model = XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z) * XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z) * XMMatrixScaling(m_scaling.x, m_scaling.y, m_scaling.z);
    XMStoreFloat4x4(&m_pModelMatrix[0], model);
    XMStoreFloat4x4(&m_pModelMatrix[1], XMMatrixTranspose(XMMatrixInverse(&det, model)));

    IEventDataPtr pEvent(new ActorMovedEvent(m_pOwner->GetID()));
    EventManager::Get()->VQueueEvent(pEvent);
}


void PoseComponent::SetRotation(float p_pitch, float p_yaw, float p_roll)
{
    m_rotation.x = p_pitch;
    m_rotation.y = p_yaw;
    m_rotation.z = p_roll;
    this->UpdateMatrices();
}