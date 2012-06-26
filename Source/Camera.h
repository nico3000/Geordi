#pragma once
#include "ConstantBuffer.h"

class Camera
{
public:
    enum Matrix
    {
        MATRIX_VIEW = 1,
        MATRIX_PROJECTION = 2,
        MATRIX_BOTH = MATRIX_VIEW | MATRIX_PROJECTION,
    };

private:
    struct CameraStruct
    {
        XMFLOAT4X4 view;
        XMFLOAT4X4 projection;
        XMFLOAT4X4 projectionView;
        XMFLOAT3 positionWC;
        float viewDistance;
    };

    CameraStruct m_struct;
    ConstantBuffer m_buffer;
    float m_fov;
    float m_aspect;
    float m_minZ;
    float m_anglePhi;
    float m_angleTheta;
    XMFLOAT3 m_dirSide;
    XMFLOAT3 m_dirUp;
    XMFLOAT3 m_dirView;

    void BuildMatrices(Matrix p_matrix);

public:

    Camera(void);
    ~Camera(void);

    bool Init(float p_fov, float p_aspect, float p_minZ, float p_maxZ);
    bool Update(Matrix p_toRebuild = MATRIX_BOTH);
    void Bind(void);
    void Rotate(float p_dPhi, float p_dTheta);
    void Move(float p_dForward, float p_dRight, float p_dUp);
    void SetPosition(float p_x, float p_y, float p_z);
};

