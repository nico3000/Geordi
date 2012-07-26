#pragma once
#include "basescenenode.h"
#include "ClipmapTerrain.h"

class ClipmapTerrainNode :
    public BaseSceneNode
{
private:
    ClipmapTerrain m_terrain;

public:
    ClipmapTerrainNode(ActorID p_actorID);
    ~ClipmapTerrainNode(void);

    HRESULT VOnRestore(void);
    HRESULT VRender(Scene* p_pScene);
};

