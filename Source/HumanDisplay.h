#pragma once
#include "IGameView.h"
#include "Scene.h"

class HumanDisplay :
    public IGameView
{
private:
    Scene m_scene;

public:
    static const GameViewID sm_ID;

    HumanDisplay(void);
    ~HumanDisplay(void);

    void VOnAttach(GameViewID p_gameViewID, ActorID p_actorID);
    void VOnRender(unsigned long p_deltaMillis);

    HRESULT VOnRestore(void) { return m_scene.OnRestore(); }
    HRESULT VOnLostDevice(void) { return m_scene.OnLostDevice(); }
    void VOnUpdate(unsigned long p_deltaMillis) { m_scene.OnUpdate(p_deltaMillis); }

    GameViewType VGetType(void) const { return IGameView::GAME_VIEW_HUMAN; }
    GameViewID VGetID(void) const { return sm_ID; }
    Scene& GetScene(void) { return m_scene; }

};

