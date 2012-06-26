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

    EventListenerDelegate actorCreatedDelegate = fastdelegate::MakeDelegate(this, &HumanDisplay::ActorCreatedDelegate);
    EventManager::Get()->VAddListener(actorCreatedDelegate, ActorCreatedEvent::sm_eventType);
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
            pCamera->Init(pComp->GetFoV(), pComp->GetAspectRatio(), pComp->GetMinZ(), pComp->GetMaxZ());
            pCamera->SetPosition(0.0f, 0.0f, -2.0f);
            m_scene.AddCamera("test", pCamera);
        }
    }
}