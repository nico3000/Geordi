#include "StdAfx.h"
#include "Actor.h"
#include "RenderComponent.h"
#include "CameraComponent.h"
#include "ParticleComponent.h"
#include "RigidbodyComponent.h"
#include "TransformComponent.h"
#include "TerrainComponent.h"

//////////////////////////////////////////////////////////////////////////
//////// Component Creator Functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ActorComponent* CreateTransformComponent(void)
{
    return new TransformComponent;
}


ActorComponent* CreateRenderComponent(void)
{
    return new RenderComponent;
}


ActorComponent* CreateCameraComponent(void)
{
    return new CameraComponent;
}


ActorComponent* CreateParticleComponent(void)
{
    return new ParticleComponent;
}


ActorComponent* CreateRigidbodyComponent(void)
{
    return new RigidbodyComponent;
}


ActorComponent* CreateTerrainComponent(void)
{
    return new TerrainComponent;
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
//     tinyxml2::XMLElement* pInitialPose = p_pData->FirstChildElement("InitialPose");
//     if(pInitialPose)
//     {
//         tinyxml2::XMLElement* pScaling = pInitialPose->FirstChildElement("Scaling");
//         if(pScaling)
//         {
//             float scaling = 1.0f;
//             pScaling->QueryFloatAttribute("scaling", &scaling);
//         }
//         tinyxml2::XMLElement* pPosition = pInitialPose->FirstChildElement("Position");
//         if(pPosition)
//         {
//             XMFLOAT3 position(0.0f, 0.0f, 0.0f);
//             pPosition->QueryFloatAttribute("x", &position.x);
//             pPosition->QueryFloatAttribute("y", &position.y);
//             pPosition->QueryFloatAttribute("z", &position.z);
//             m_localPose.SetPosition(position);
//         }
//         tinyxml2::XMLElement* pRotation = pInitialPose->FirstChildElement("Rotation");
//         if(pRotation)
//         {
//             XMFLOAT3 rotation(0.0f, 0.0f, 0.0f);
//             pRotation->QueryFloatAttribute("pitch", &rotation.x);
//             pRotation->QueryFloatAttribute("yaw", &rotation.y);
//             pRotation->QueryFloatAttribute("roll", &rotation.z);
//             m_localPose.SetPitchYawRoll(rotation.x, rotation.y, rotation.z);
//         }
//     }
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


void Actor::Update(unsigned long p_deltaMillis, unsigned long p_gameMillis)
{
    ActorComponents::iterator iter;
    for(iter = m_components.begin(); iter != m_components.end(); ++iter)
    {
        iter->second->VUpdate(p_deltaMillis, p_gameMillis);
    }
}


void Actor::AddComponent(StrongActorComponentPtr p_pComponent)
{
    m_components[p_pComponent->VGetComponentID()] = p_pComponent;
}


ActorFactory::ActorFactory(void)
{
    m_actorComponentCreators["RenderComponent"] = CreateRenderComponent;
    m_actorComponentIDs["RenderComponent"] = RenderComponent::sm_componentID;
    m_actorComponentCreators["CameraComponent"] = CreateCameraComponent;
    m_actorComponentIDs["CameraComponent"] = CameraComponent::sm_componentID;
    m_actorComponentCreators["ParticleComponent"] = CreateParticleComponent;
    m_actorComponentIDs["ParticleComponent"] = ParticleComponent::sm_componentID;
    m_actorComponentCreators["RigidbodyComponent"] = CreateRigidbodyComponent;
    m_actorComponentIDs["RigidbodyComponent"] = RigidbodyComponent::sm_componentID;
    m_actorComponentCreators["TransformComponent"] = CreateTransformComponent;
    m_actorComponentIDs["TransformComponent"] = TransformComponent::sm_componentID;
    m_actorComponentCreators["TerrainComponent"] = CreateTerrainComponent;
    m_actorComponentIDs["TerrainComponent"] = TerrainComponent::sm_componentID;
}


ActorFactory::~ActorFactory(void)
{
    m_actorComponentCreators.clear();
}


StrongActorPtr ActorFactory::CreateActor(const char* p_actorResource, tinyxml2::XMLElement* p_pOverrideData)
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
    pComponentData = p_pOverrideData;
    while(pComponentData) 
    {
        std::string name(pComponentData->Name());
        auto findit = m_actorComponentIDs.find(pComponentData->Name());
        if(findit != m_actorComponentIDs.end())
        {
            ComponentID id = findit->second;
            StrongActorComponentPtr pComponent(pActor->GetComponent<ActorComponent>(id));
            if(pComponent)
            {
                pComponent->VInit(pComponentData);
            }
            else
            {
                pComponent = this->CreateComponent(pComponentData);
                if(pComponent)
                {
                    pComponent->SetOwner(pActor);
                    pActor->AddComponent(pComponent);
                }
            }
        }
        else
        {
            LI_ERROR("Unknown component: " + name);
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
        LI_ERROR("Unknown component: " + name);
    }
    return StrongActorComponentPtr(0);
}