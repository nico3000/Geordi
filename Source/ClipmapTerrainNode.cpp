#include "StdAfx.h"
#include "ClipmapTerrainNode.h"
#include "Scene.h"


ClipmapTerrainNode::ClipmapTerrainNode(ActorID p_actorID):
BaseSceneNode(p_actorID)
{
}


ClipmapTerrainNode::~ClipmapTerrainNode(void)
{
}


HRESULT ClipmapTerrainNode::VOnRestore(void)
{
    if(!m_terrain.Init(63, 4))
    {
        return S_FALSE;
    }
    return BaseSceneNode::VOnRestore();
}


HRESULT ClipmapTerrainNode::VRender(Scene* p_pScene)
{
    m_terrain.Draw();
    return S_OK;
}


HRESULT ClipmapTerrainNode::VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis)
{
    m_terrain.Update(p_pScene->GetCurrentCamera()->GetPosition());
    return BaseSceneNode::VOnUpdate(p_pScene, p_deltaMillis);;
}