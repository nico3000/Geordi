#pragma once
#include "ConstantBuffer.h"
#include "CameraComponent.h"

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

    WeakActorPtr m_pActor;
    std::weak_ptr<CameraComponent> m_pCameraComponent;
    CameraStruct m_struct;
    ConstantBuffer m_buffer;

    void BuildMatrices(Matrix p_matrix);

public:

    Camera(void);
    ~Camera(void);

    bool Init(WeakActorPtr p_pActor);
    bool Update(Matrix p_toRebuild = MATRIX_BOTH);
    void Bind(void);
};

