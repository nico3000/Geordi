#include "StdAfx.h"
#include "RootNode.h"


XMFLOAT4X4 identity(1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);


RootNode::RootNode(void) :
BaseSceneNode(INVALID_ACTOR_ID, identity)
{
}


RootNode::~RootNode(void)
{
}
