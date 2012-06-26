#pragma once
class SceneNodeProperties
{
    friend class BaseSceneNode;

protected:
    ActorID m_actorID;
    std::string m_name;
    float m_radius;

public:
    SceneNodeProperties(void) {  }

    const ActorID& GetActorID(void) const { return m_actorID; }
    const std::string& GetName(void) const { return m_name; }
    const float& GetRadius(void) const { return m_radius; }

};

