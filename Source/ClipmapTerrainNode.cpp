#include "StdAfx.h"
#include "ClipmapTerrainNode.h"


ClipmapTerrainNode::ClipmapTerrainNode(ActorID p_actorID):
BaseSceneNode(p_actorID)
{
}


ClipmapTerrainNode::~ClipmapTerrainNode(void)
{
}


HRESULT ClipmapTerrainNode::VOnRestore(void)
{
    if(!m_terrain.Init(15, 3))
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