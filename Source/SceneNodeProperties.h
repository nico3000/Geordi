#pragma once
class SceneNodeProperties
{
    friend class BaseSceneNode;

protected:
    ActorID m_actorID;
    std::string m_name;
    XMMATRIX m_model;
    XMMATRIX m_modelInv;
    float m_radius;

public:
    SceneNodeProperties(void) {  }

    const ActorID& GetActorID(void) const { return m_actorID; }
    const std::string& GetName(void) const { return m_name; }
    const XMMATRIX& GetModel(void) const { return m_model; }
    const XMMATRIX& GetModelInv(void) const { return m_modelInv; }
    const float& GetRadius(void) const { return m_radius; }

    void Transform(XMFLOAT4X4* p_pModel, XMFLOAT4X4* p_pModelInv) const;

};

