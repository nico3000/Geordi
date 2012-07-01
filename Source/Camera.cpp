#include "StdAfx.h"
#include "Camera.h"


#define CBUFFER_CAMERA_SLOT 0


Camera::Camera(void)
{
}


Camera::~Camera(void)
{
}


bool Camera::Init(WeakActorPtr p_pActor)
{
    m_pActor = p_pActor;
    StrongActorPtr pActor = p_pActor.lock();
    if(pActor)
    {
        m_pCameraComponent = pActor->GetComponent<CameraComponent>(CameraComponent::sm_componentID);
        if(!m_pCameraComponent.lock())
        {
            LI_ERROR("Camera initialized with non-camera actor");
            return false;
        }
        return m_buffer.BuildFromSharedData(&m_struct, sizeof(CameraStruct));
    }
    else
    {
        return false;
    }
}


bool Camera::Update(Matrix p_toRebuild /* = MATRIX_BOTH */)
{
    this->BuildMatrices(p_toRebuild);
    return m_buffer.Update();
}


void Camera::Bind(void)
{
    m_buffer.Bind(CBUFFER_CAMERA_SLOT, ConstantBuffer::TARGET_ALL);
}


void Camera::BuildMatrices(Matrix p_matrix)
{
    if(p_matrix & MATRIX_PROJECTION)
    {
        std::shared_ptr<CameraComponent> pComp = m_pCameraComponent.lock();
        if(pComp)
        {
            float aspect = pComp->GetAspectRatio() == 0.0f ? (float)LostIsland::g_pGraphics->GetWidth() / (float)LostIsland::g_pGraphics->GetHeight() : pComp->GetAspectRatio();
            XMStoreFloat4x4(&m_struct.projection, XMMatrixPerspectiveFovLH(pComp->GetFoV(), aspect, pComp->GetMinZ(), pComp->GetMaxZ()));
            m_struct.viewDistance = pComp->GetMaxZ();
        }
    }
    if(p_matrix & MATRIX_VIEW)
    {
        StrongActorPtr pActor = m_pActor.lock();
        if(pActor)
        {
            XMStoreFloat4x4(&m_struct.view, pActor->GetPose().GetModelMatrixBuffer(true).modelInv);
            m_struct.positionWC = pActor->GetPose().GetPosition();
        }
    }
    if(p_matrix & (MATRIX_BOTH))
    {
        XMStoreFloat4x4(&m_struct.projectionView, XMMatrixMultiply(XMLoadFloat4x4(&m_struct.view), XMLoadFloat4x4(&m_struct.projection)));
    }
}

