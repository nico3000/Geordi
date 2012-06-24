#pragma once
#include "IGameView.h"
#include "ISceneNode.h"

class HumanDisplay :
    public IGameView
{
public:
    static const GameViewID sm_ID;

    HumanDisplay(void);
    ~HumanDisplay(void);

    HRESULT VOnRestore(void);
    void VOnRender(unsigned long p_deltaMillis);
    HRESULT VOnLostDevice(void);
    GameViewType VGetType(void) const { return IGameView::GAME_VIEW_HUMAN; }
    GameViewID VGetID(void) const { return sm_ID; }
    void VOnUpdate(unsigned long p_deltaMillis);
    void VOnAttach(GameViewID p_gameViewID, ActorID p_actorID);
};

