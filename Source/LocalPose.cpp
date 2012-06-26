#include "StdAfx.h"
#include "LocalPose.h"


LocalPose::LocalPose(void) :
m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scaling(1.0f),
    m_sideDir(1.0f, 0.0f, 0.0f), m_upDir(0.0f, 1.0f, 0.0f), m_viewDir(0.0f, 0.0f, 1.0f),
    m_fixed(false)
{
}


LocalPose::~LocalPose(void)
{
}


void LocalPose::Copy(const LocalPose& p_toCopy)
{
    this->SetPosition(p_toCopy.m_position);
    this->SetScaling(p_toCopy.m_scaling);
    this->SetPitchYawRoll(p_toCopy.m_rotation.x, p_toCopy.m_rotation.y, p_toCopy.m_rotation.z);
    this->m_fixed = false;
    this->UpdateMatrices();
    this->m_fixed = p_toCopy.m_fixed;
}


void LocalPose::RotationChanged(void)
{
    static XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z));
    m_sideDir.x = temp._11; m_sideDir.y = temp._12; m_sideDir.z = temp._13;
    m_upDir  .x = temp._21; m_upDir  .y = temp._22; m_upDir  .z = temp._23;
    m_viewDir.x = temp._31; m_viewDir.y = temp._32; m_viewDir.z = temp._33;
}


void LocalPose::SetPosition(const XMFLOAT3& p_position)
{
    m_position = p_position;
}


void LocalPose::SetScaling(float p_scaling)
{
    m_scaling = p_scaling;
}


void LocalPose::SetPitchYawRoll(float p_pitch, float p_yaw, float p_roll)
{
    m_rotation.x = p_pitch;
    m_rotation.y = p_yaw;
    m_rotation.z = p_roll;
    this->RotationChanged();
}


void LocalPose::TranslateLocal(const XMFLOAT3& p_translation)
{
    m_position.x += p_translation.x * m_sideDir.x + p_translation.y * m_upDir.x + p_translation.z * m_viewDir.x;
    m_position.y += p_translation.x * m_sideDir.y + p_translation.y * m_upDir.y + p_translation.z * m_viewDir.y;
    m_position.z += p_translation.x * m_sideDir.z + p_translation.y * m_upDir.z + p_translation.z * m_viewDir.z;
}


void LocalPose::TranslateWorld(const XMFLOAT3& p_translation)
{
    m_position.x += p_translation.x;
    m_position.y += p_translation.y;
    m_position.z += p_translation.z;
}


void LocalPose::RotateLocal(float p_dPitch, float p_dYaw, float p_dRoll)
{
    m_rotation.x += p_dPitch;
    m_rotation.y += p_dYaw;
    m_rotation.z += p_dRoll;
    this->RotationChanged();
}


void LocalPose::RotateWorld(float p_dPitch, float p_dYaw, float p_dRoll)
{
    // TODO!!!
    LI_ERROR("RotateWorld() not supported yet! Why the heck do you need it???");
}


void LocalPose::Scale(float p_dScale)
{
    m_scaling *= p_dScale;
}


const LocalPose::ModelMatrixData& LocalPose::GetModelMatrixBuffer(bool p_update /* = false */)
{
    if(p_update)
    {
        this->UpdateMatrices();
    }
    return m_buffer;
}


void LocalPose::UpdateMatrices(void)
{
    if(!m_fixed)
    {
        m_buffer.model = XMMatrixScaling(m_scaling, m_scaling, m_scaling) *
            XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z) *
            XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
        m_buffer.modelInv = XMMatrixTranslation(-m_position.x, -m_position.y, -m_position.z) *
            XMMatrixRotationRollPitchYaw(-m_rotation.x, -m_rotation.y, -m_rotation.z) *
            XMMatrixScaling(1.0f / m_scaling, 1.0f / m_scaling, 1.0f / m_scaling);
    }
}