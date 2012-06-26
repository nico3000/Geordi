#include "StdAfx.h"
#include "HumanDisplay.h"


const GameViewID HumanDisplay::sm_ID = 0x1c0dbf5a;


HumanDisplay::HumanDisplay(void)
{
}


HumanDisplay::~HumanDisplay(void)
{
}


void HumanDisplay::VOnRender(unsigned long p_deltaMillis)
{
    m_scene.Render();
}


void HumanDisplay::VOnAttach(GameViewID p_gameViewID, ActorID p_actorID)
{

}
