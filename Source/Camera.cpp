#include "StdAfx.h"
#include "Camera.h"


#define CBUFFER_CAMERA_SLOT 0


Camera::Camera(void):
m_dirView(0.0f, 0.0f, 1.0f), m_dirUp(0.0f, 1.0f, 0.0f), m_dirSide(1.0f, 0.0f, 0.0f), m_anglePhi(0.0f), m_angleTheta(0.0f), m_fov(XM_PIDIV2)
{
    m_struct.viewDistance = 1e+2f;
}


Camera::~Camera(void)
{
}


bool Camera::Init(void)
{
    this->BuildMatrices(MATRIX_BOTH);
    return m_buffer.Build(&m_struct, sizeof(CameraStruct));
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

    if(p_matrix & MATRIX_VIEW)
    {
        XMStoreFloat4x4(&m_struct.view, XMMatrixLookToLH(XMLoadFloat3(&m_struct.positionWC), XMLoadFloat3(&m_dirView), XMLoadFloat3(&m_dirUp)));
    }
    if(p_matrix & MATRIX_PROJECTION)
    {
        XMStoreFloat4x4(&m_struct.projection, XMMatrixPerspectiveFovLH(m_fov, (float)LostIsland::g_pGraphics->GetWidth() / (float)LostIsland::g_pGraphics->GetHeight(), 1e-2f, m_struct.viewDistance));
    }
    if(p_matrix & (MATRIX_BOTH))
    {
        XMStoreFloat4x4(&m_struct.projectionView, XMMatrixMultiply(XMLoadFloat4x4(&m_struct.view), XMLoadFloat4x4(&m_struct.projection)));
    }
}


void Camera::Rotate(float p_dPhi, float p_dTheta)
{
    m_anglePhi += p_dPhi;
    m_angleTheta = CLAMP(m_angleTheta + p_dTheta, -XM_PIDIV2, +XM_PIDIV2);

    float sinPhi = sin(m_anglePhi);
    float cosPhi = cos(m_anglePhi);
    float sinTheta = sin(m_angleTheta);
    float cosTheta = cos(m_angleTheta);
    m_dirSide.x = cosPhi;    m_dirSide.y = sinPhi * sinTheta; m_dirSide.z = cosPhi * sinTheta;
    m_dirUp  .x = 0.0f;      m_dirUp  .y = cosPhi;            m_dirUp  .z = -sinPhi;
    m_dirSide.x = -sinTheta; m_dirSide.y = sinPhi * cosTheta; m_dirSide.z = cosPhi * cosTheta;
}


void Camera::Move(float p_dForward, float p_dRight, float p_dUp)
{
    m_struct.positionWC.x += p_dForward * m_dirView.x + p_dRight * m_dirSide.x;
    m_struct.positionWC.y += p_dForward * m_dirView.y + p_dRight * m_dirSide.y + p_dUp;
    m_struct.positionWC.z += p_dForward * m_dirView.z + p_dRight * m_dirSide.z;
}


void Camera::SetPosition(float p_x, float p_y, float p_z)
{
    m_struct.positionWC.x = p_x;
    m_struct.positionWC.y = p_y;
    m_struct.positionWC.z = p_z;
}