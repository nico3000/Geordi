#pragma once
class Pose
{
public:
    struct ModelMatrixData
    {
        XMFLOAT4X4 model;
        XMFLOAT4X4 modelInv;
    };

private:
    ModelMatrixData m_buffer;
    bool m_fixed;
    // global pose properties
    XMFLOAT3 m_position;
    float m_scaling;
    XMFLOAT3 m_rotation;
    // temp vectors for local delta translation
    XMFLOAT4X4 m_rotMatrix;
    XMFLOAT3 m_sideDir;
    XMFLOAT3 m_upDir;
    XMFLOAT3 m_viewDir;

    void RotationChanged(void);

public:
    Pose(void);
    ~Pose(void);

    void SetPosition(const XMFLOAT3& p_position);
    void SetPitchYawRoll(float p_pitch, float p_yaw, float p_roll);
    void SetScaling(float p_scaling);
    void Copy(const Pose& p_toCopy);

    void TranslateWorld(const XMFLOAT3& p_translation);
    void RotateWorld(float p_dPitch, float p_dYaw, float p_dRoll);
    void TranslateLocal(const XMFLOAT3& p_translation);
    void RotateLocal(float p_dPitch, float p_dYaw, float p_dRoll);
    void Scale(float p_dScale);

    const XMFLOAT3& GetPosition(void) const { return m_position; }
    float GetPitch(void) const { return m_rotation.x; }
    float GetYaw(void) const { return m_rotation.y; }
    float GetRoll(void) const { return m_rotation.z; }
    float GetScaling(void) const { return m_scaling; }

    void Fix(void) { m_fixed = true; }
    void Unfix(void) { m_fixed = false; }
    void UpdateMatrices(void);
    void SetMatrices(const XMFLOAT4X4& p_model, const XMFLOAT4X4* p_pModelInv = 0);

    const ModelMatrixData& GetModelMatrixBuffer(bool p_update = false);
};

