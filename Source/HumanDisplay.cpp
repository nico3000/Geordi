#include "StdAfx.h"
#include "HumanDisplay.h"
#include "ActorEvents.h"
#include "CameraComponent.h"


HumanDisplay::HumanDisplay(void)
{
}


HumanDisplay::~HumanDisplay(void)
{
}


void HumanDisplay::VOnRender(unsigned long p_deltaMillis)
{
    m_scene.Render();
}


void HumanDisplay::VOnAttach(GameViewID p_viewID, ActorID p_actorID)
{
    m_actorID = p_actorID;
    m_viewID = p_viewID;

    EventListenerDelegate onActorCreated = fastdelegate::MakeDelegate(this, &HumanDisplay::ActorCreatedDelegate);
    EventManager::Get()->VAddListener(onActorCreated, ActorCreatedEvent::sm_eventType);

    EventListenerDelegate onRenderComponentCreated = fastdelegate::MakeDelegate(&m_scene, &Scene::RenderComponentCreatedDelegate);
    EventManager::Get()->VAddListener(onRenderComponentCreated, RenderComponentCreatedEvent::sm_eventType);

    EventListenerDelegate onParticleComponentCreated = fastdelegate::MakeDelegate(&m_scene, &Scene::ParticleComponentCreatedDelegate);
    EventManager::Get()->VAddListener(onParticleComponentCreated, ParticleComponentCreatedEvent::sm_eventType);

    EventListenerDelegate onTerrainComponentCreated = fastdelegate::MakeDelegate(&m_scene, &Scene::TerrainComponentCreatedDelegate);
    EventManager::Get()->VAddListener(onTerrainComponentCreated, TerrainComponentCreatedEvent::sm_eventType);

    EventListenerDelegate onClipmapTerrainComponentCreated = fastdelegate::MakeDelegate(&m_scene, &Scene::ClipmapTerrainComponentCreatedDelegate);
    EventManager::Get()->VAddListener(onClipmapTerrainComponentCreated, ClipmapTerrainComponentCreatedEvent::sm_eventType);
}


void HumanDisplay::ActorCreatedDelegate(IEventDataPtr p_pEventData)
{
    std::shared_ptr<ActorCreatedEvent> pEvent = std::static_pointer_cast<ActorCreatedEvent>(p_pEventData);
    StrongActorPtr pActor = LostIsland::g_pApp->GetGameLogic()->VGetActor(pEvent->GetActorID()).lock();
    if(pActor)
    {
        std::shared_ptr<CameraComponent> pComp = pActor->GetComponent<CameraComponent>(CameraComponent::sm_componentID).lock();
        if(pComp)
        {
            std::shared_ptr<Camera> pCamera(new Camera);
            pCamera->Init(pActor);
            m_scene.AddCamera("test", pCamera);
        }
    }
}