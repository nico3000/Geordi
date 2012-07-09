#pragma once
#include "BaseSceneNode.h"
#include "ShaderProgram.h"

class ParticleNode :
    public BaseSceneNode
{
private:
    std::weak_ptr<ParticleCloud> m_pCloud;

    // TODO!!!
    ShaderProgram m_program;
    ID3D11ShaderResourceView* m_pTexture;
    ID3D11BlendState* m_pBlendState;
    ID3D11DepthStencilState* m_pDepthStencilState;

public:
    ParticleNode(ActorID p_actorID, std::weak_ptr<ParticleCloud> p_pCloud);
    ~ParticleNode(void);

    HRESULT VOnRestore(void);
    HRESULT VPreRender(Scene* p_pScene);
    HRESULT VRender(Scene* p_pScene);
    NodeType VGetNodeType(void) const { return TRANSPARENT_NODE; }

};

