#include "StdAfx.h"
#include "PhysicsLayer.h"

#define LI_LOGGER_TAG "PhysicsLayer"

PhysicsLayer::ErrorCallBack PhysicsLayer::sm_errorCallback;
static physx::PxDefaultAllocator g_defaultAllocatorCallback;


void PhysicsLayer::ErrorCallBack::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
    static Logger::ErrorMessenger messenger;
    switch(code)
    {
    case physx::PxErrorCode::eABORT:
    case physx::PxErrorCode::eEXCEPTION_ON_STARTUP:
    case physx::PxErrorCode::eINTERNAL_ERROR:
    case physx::PxErrorCode::eINVALID_OPERATION:
    case physx::PxErrorCode::eINVALID_PARAMETER:
    case physx::PxErrorCode::eOUT_OF_MEMORY:
        messenger.Show(message, true, "unknown", file, line);
#ifdef _DEBUG
        do { void* p = 0; } while (0);
#endif
        break;
    case physx::PxErrorCode::eDEBUG_WARNING:
    case physx::PxErrorCode::ePERF_WARNING: messenger.Show(message, false, "unknown", file, line); break;
    case physx::PxErrorCode::eDEBUG_INFO:
    case physx::PxErrorCode::eMASK_ALL: Logger::Log(LI_LOGGER_TAG, message, false, "unknown", file, line); break;
    case physx::PxErrorCode::eNO_ERROR: return;
    }
}


PhysicsLayer::PhysicsLayer(void):
m_pFoundation(0), m_pPhysics(0), m_pScene(0), m_pCpuDispatcher(0), m_pPVDConnection(0)
{
}


PhysicsLayer::~PhysicsLayer(void)
{
    SAFE_rELEASE(m_pScene);
    SAFE_rELEASE(m_pCpuDispatcher);
    SAFE_rELEASE(m_pPVDConnection);
    SAFE_rELEASE(m_pPhysics);
    SAFE_rELEASE(m_pFoundation);
}


bool PhysicsLayer::Init(void)
{
    if(!m_pFoundation)
    {
        m_stepSize = 1.0f / 60.0f;

        m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocatorCallback, sm_errorCallback);
        if(!m_pFoundation)
        {
            LI_ERROR("PxCreateFoundation() failed");
            return false;
        }
        
        m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale(), true);
        if(!m_pPhysics)
        {
            LI_ERROR("PxCreatePhysics() failed");
            return false;
        }

        physx::PxVisualDebuggerConnectionFlags flags = physx::PxVisualDebuggerExt::getAllConnectionFlags();
        m_pPVDConnection = physx::PxVisualDebuggerExt::createConnection(m_pPhysics->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, flags);

        m_pCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(4);
        physx::PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
        sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        sceneDesc.cpuDispatcher = m_pCpuDispatcher;
        sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
        if(!sceneDesc.isValid())
        {
            LI_ERROR("PxSceneDesc is not valid");
            return false;
        }
        m_pScene = m_pPhysics->createScene(sceneDesc);
        //SAFE_rELEASE(m_pCpuDispatcher); // TODO: reference count?
        if(!m_pScene)
        {
            return false;
        }


        physx::PxMaterial* pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.1f);
        physx::PxRigidStatic* pPlane = PxCreatePlane(*m_pPhysics, physx::PxPlane(physx::PxVec3(0,1,0), +5.0f), *pMaterial);        
        m_pScene->addActor(*pPlane);
        

        LI_LOG_WITH_TAG("PhysX successfully initialized");
        return true;
    }
    return false;
}


void PhysicsLayer::Update(unsigned long p_deltaMillis, unsigned long p_gameMillis)
{
    static physx::PxReal elapsed = 0.0f;
    elapsed += 1e-3f * (float)p_deltaMillis;
    if(elapsed > m_stepSize)
    {
        m_pScene->simulate(m_stepSize);
        elapsed -= m_stepSize;
    }
}


bool PhysicsLayer::FetchResults(void)
{
    return m_pScene->fetchResults(true);
}