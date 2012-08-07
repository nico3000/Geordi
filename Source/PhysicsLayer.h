#pragma once

#define SAFE_rELEASE(_resource) do { if(_resource) { _resource->release(); _resource = 0; } } while(0)

class PhysicsLayer
{
private:
    class ErrorCallBack : public physx::PxErrorCallback
    {
    public:
        void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
    };

    static ErrorCallBack sm_errorCallback;

    physx::PxFoundation* m_pFoundation;
    physx::PxPhysics* m_pPhysics;
    physx::PxScene* m_pScene;
    physx::PxDefaultCpuDispatcher* m_pCpuDispatcher;
    physx::PxCooking* m_pCooking;
    PVD::PvdConnection* m_pPVDConnection;
    float m_stepSize;

public:
    PhysicsLayer(void);
    ~PhysicsLayer(void);

    bool Init(void);
    void Update(unsigned long p_deltaMillis, unsigned long p_gameMillis);
    bool FetchResults(void);

    physx::PxPhysics* GetPhysics(void) const { return m_pPhysics; }
    physx::PxScene* GetScene(void) const { return m_pScene; }
    physx::PxCooking* GetCooking(void) const { return m_pCooking; }

};

