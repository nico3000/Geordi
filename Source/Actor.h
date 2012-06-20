#pragma once

class Actor;
class ActorComponent;
typedef ActorComponent* (*ActorComponentCreator)(void);
typedef std::map<std::string, ActorComponentCreator> ActorComponentCreatorMap;
typedef unsigned long ActorID;
typedef unsigned int ComponentID;
typedef std::shared_ptr<Actor> StrongActorPtr;
typedef std::weak_ptr<Actor> WeakActorPtr;
typedef std::shared_ptr<ActorComponent> StrongActorComponentPtr;

class ActorComponent
{
    friend class ActorFactory;
    friend class Actor;

private:
    void SetOwner(StrongActorPtr p_pOwner) { m_pOwner = p_pOwner; }

protected:
    StrongActorPtr m_pOwner;

public:
    virtual ~ActorComponent(void) {}
    virtual bool VInit(tinyxml2::XMLElement* p_pData) = 0;
    virtual void VPostInit(void) {}
    virtual void VUpdate(unsigned long deltaMillis) {}
    virtual ComponentID VGetComponentID(void) const = 0;

};


class Actor
{
    friend class ActorFactory;

    typedef std::map<ComponentID, StrongActorComponentPtr> ActorComponents;

private:
    ActorID m_id;
    ActorComponents m_components;

    void AddComponent(StrongActorComponentPtr p_pComponent);

public:
    explicit Actor(ActorID p_id) { m_id = p_id; }
    ~Actor(void);

    bool Init(tinyxml2::XMLElement* p_pData);
    void PostInit(void);
    void Destroy(void);
    void Update(unsigned long p_deltaMillis);

    ActorID GetID(void) const { return m_id; }

    template<class ComponentType>
    std::weak_ptr<ComponentType> GetComponent(ComponentID p_id)
    {
        ActorComponents::iterator iter = m_components.find(p_id);
        if(iter != m_components.end())
        {
            StrongActorComponentPtr pBase(iter->second);
            std::shared_ptr<ComponentType> pSub(std::static_pointer_cast<ComponentType>(pBase));
            std::weak_ptr<ComponentType> pWeakSub(pSub);
            return pWeakSub;
        }
        else
        {
            return std::weak_ptr<ComponentType>(0);
        }
    }
};


class ActorFactory
{
private:
    ActorID m_lastActorID;

    ActorID GetNextActorID(void) { return m_lastActorID++; }

protected:
    ActorComponentCreatorMap m_actorComponentCreators;
    virtual StrongActorComponentPtr CreateComponent(tinyxml2::XMLElement* p_pData);

public:
    ActorFactory(void);
    ~ActorFactory(void);

    StrongActorPtr CreateActor(const char* p_actorResource);

};


class CubeRenderComponent :
    public ActorComponent
{
private:
    XMFLOAT4 m_color;
    float m_size;

public:
    CubeRenderComponent(void) {}
    ~CubeRenderComponent(void) {}

    bool VInit(tinyxml2::XMLElement* p_pData);
    ComponentID VGetComponentID(void) const { return 0xd08283ae; }

};
