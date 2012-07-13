#include "StdAfx.h"
#include "Pose.h"


Pose::Pose(void) :
m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scaling(1.0f),
    m_sideDir(1.0f, 0.0f, 0.0f), m_upDir(0.0f, 1.0f, 0.0f), m_viewDir(0.0f, 0.0f, 1.0f),
    m_fixed(false)
{
    this->RotationChanged();
    this->UpdateMatrices();
}


Pose::~Pose(void)
{
}


void Pose::Copy(const Pose& p_toCopy)
{
    this->SetPosition(p_toCopy.m_position);
    this->SetScaling(p_toCopy.m_scaling);
    this->SetPitchYawRoll(p_toCopy.m_rotation.x, p_toCopy.m_rotation.y, p_toCopy.m_rotation.z);
    this->m_fixed = false;
    this->UpdateMatrices();
    this->m_fixed = p_toCopy.m_fixed;
}


void Pose::RotationChanged(void)
{
    XMStoreFloat4x4(&m_rotMatrix, XMMatrixRotationZ(m_rotation.z) * XMMatrixRotationX(m_rotation.x) * XMMatrixRotationY(m_rotation.y));
    m_sideDir.x = m_rotMatrix._11; m_sideDir.y = m_rotMatrix._12; m_sideDir.z = m_rotMatrix._13;
    m_upDir  .x = m_rotMatrix._21; m_upDir  .y = m_rotMatrix._22; m_upDir  .z = m_rotMatrix._23;
    m_viewDir.x = m_rotMatrix._31; m_viewDir.y = m_rotMatrix._32; m_viewDir.z = m_rotMatrix._33;
}


void Pose::SetPosition(const XMFLOAT3& p_position)
{
    m_position = p_position;
}


void Pose::SetScaling(float p_scaling)
{
    m_scaling = p_scaling;
}


void Pose::SetPitchYawRoll(float p_pitch, float p_yaw, float p_roll)
{
    m_rotation.x = p_pitch;
    m_rotation.y = p_yaw;
    m_rotation.z = p_roll;
    this->RotationChanged();
}


void Pose::TranslateLocal(const XMFLOAT3& p_translation)
{
    m_position.x += p_translation.x * m_sideDir.x + p_translation.y * m_upDir.x + p_translation.z * m_viewDir.x;
    m_position.y += p_translation.x * m_sideDir.y + p_translation.y * m_upDir.y + p_translation.z * m_viewDir.y;
    m_position.z += p_translation.x * m_sideDir.z + p_translation.y * m_upDir.z + p_translation.z * m_viewDir.z;
}


void Pose::TranslateWorld(const XMFLOAT3& p_translation)
{
    m_position.x += p_translation.x;
    m_position.y += p_translation.y;
    m_position.z += p_translation.z;
}


void Pose::RotateLocal(float p_dPitch, float p_dYaw, float p_dRoll)
{
    m_rotation.x += p_dPitch;
    m_rotation.y += p_dYaw;
    m_rotation.z += p_dRoll;
    this->RotationChanged();
}


void Pose::RotateWorld(float p_dPitch, float p_dYaw, float p_dRoll)
{
    // TODO!!!
    LI_ERROR("RotateWorld() not supported yet! Why the heck do you need it???");
}


void Pose::Scale(float p_dScale)
{
    m_scaling *= p_dScale;
}


const Pose::ModelMatrixData& Pose::GetModelMatrixBuffer(bool p_update /* = false */)
{
    if(p_update)
    {
        this->UpdateMatrices();
    }
    return m_buffer;
}


void Pose::UpdateMatrices(void)
{
    if(!m_fixed)
    {
        XMStoreFloat4x4(&m_buffer.model,
            XMMatrixScaling(m_scaling, m_scaling, m_scaling) *
            XMLoadFloat4x4(&m_rotMatrix) *
            XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
        XMStoreFloat4x4(&m_buffer.modelInv,
            XMMatrixTranslation(-m_position.x, -m_position.y, -m_position.z) *
            XMMatrixTranspose(XMLoadFloat4x4(&m_rotMatrix)) *
            XMMatrixScaling(1.0f / m_scaling, 1.0f / m_scaling, 1.0f / m_scaling));
    }
}


void Pose::SetMatrices(const XMFLOAT4X4& p_model, const XMFLOAT4X4* p_pModelInv /* = 0 */)
{
    m_buffer.model = p_model;
    if(!p_pModelInv)
    {
        XMVECTOR det;
        XMStoreFloat4x4(&m_buffer.modelInv, XMMatrixInverse(&det, XMLoadFloat4x4(&m_buffer.model)));
    }
    else
    {
        m_buffer.modelInv = *p_pModelInv;
    }
    this->Fix();
}