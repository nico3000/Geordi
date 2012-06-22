#include "StdAfx.h"
#include "Actor.h"
#include "RenderComponent.h"
#include "PoseComponent.h"
#include "RotationComponent.h"

//////////////////////////////////////////////////////////////////////////
//////// Component Creator Functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ActorComponent* CreateRenderComponent(void) {
    return new RenderComponent;
}

ActorComponent* CreatePoseComponent(void) {
    return new PoseComponent;
}

ActorComponent* CreateRotationComponent(void) {
    return new RotationComponent;
}

//////////////////////////////////////////////////////////////////////////

Actor::~Actor(void)
{
}


void Actor::Destroy(void)
{
    m_components.clear();
}


bool Actor::Init(tinyxml2::XMLElement* p_pData)
{
    // TODO: nothing todo yet
    return true;
}


void Actor::PostInit(void)
{
    ActorComponents::iterator iter;
    for(iter = m_components.begin(); iter != m_components.end(); ++iter)
    {
        iter->second->VPostInit();
    }
}


void Actor::Update(unsigned long p_deltaMillis)
{
    ActorComponents::iterator iter;
    for(iter = m_components.begin(); iter != m_components.end(); ++iter)
    {
        iter->second->VUpdate(p_deltaMillis);
    }
}


void Actor::AddComponent(StrongActorComponentPtr p_pComponent)
{
    m_components[p_pComponent->VGetComponentID()] = p_pComponent;
}


ActorFactory::ActorFactory(void)
{
    m_actorComponentCreators["RenderComponent"] = CreateRenderComponent;
    m_actorComponentCreators["PoseComponent"] = CreatePoseComponent;
    m_actorComponentCreators["RotationComponent"] = CreateRotationComponent;
}


ActorFactory::~ActorFactory(void)
{
    m_actorComponentCreators.clear();
}


StrongActorPtr ActorFactory::CreateActor(const char* p_actorResource)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* pActorData = 0;
    if(doc.LoadFile(p_actorResource) != tinyxml2::XML_NO_ERROR)
    {
        LI_ERROR("Invalid actor file: " + std::string(p_actorResource));
        return StrongActorPtr(0);
    }
    pActorData = doc.FirstChildElement("Actor");
    if(!pActorData)
    {
        LI_ERROR("No Actor tag in actor file: " + std::string(p_actorResource));
        return StrongActorPtr(0);
    }
    StrongActorPtr pActor(new Actor(this->GetNextActorID()));
    if(!pActor->Init(pActorData))
    {
        LI_ERROR("Actor initialization failed: " + std::string(p_actorResource));
        return StrongActorPtr(0);
    }
    tinyxml2::XMLElement* pComponentData = pActorData->FirstChildElement();
    while(pComponentData)
    {
        StrongActorComponentPtr pComponent(this->CreateComponent(pComponentData));
        if(pComponent)
        {
            pComponent->SetOwner(pActor);
            pActor->AddComponent(pComponent);
        }
        else
        {
            //return StrongActorPtr(0);
        }
        pComponentData = pComponentData->NextSiblingElement();
    }
    pActor->PostInit();
    return pActor;    
}


StrongActorComponentPtr ActorFactory::CreateComponent(tinyxml2::XMLElement* p_pData)
{
    std::string name(p_pData->Name());
    ActorComponentCreatorMap::iterator iter = m_actorComponentCreators.find(p_pData->Name());
    if(iter != m_actorComponentCreators.end())
    {
        ActorComponentCreator create = m_actorComponentCreators[name];
        StrongActorComponentPtr pComponent(create());
        if(pComponent->VInit(p_pData))
        {
            return pComponent;
        }
        else
        {
            LI_ERROR("Component initialization failed: " + name);
        }
    }
    else
    {
        LI_ERROR("No suitable component constructor found: " + name);
    }
    return StrongActorComponentPtr(0);
}