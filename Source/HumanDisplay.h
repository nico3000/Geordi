#pragma once
#include "IGameView.h"
#include "Scene.h"

class HumanDisplay :
    public IGameView
{
private:
    Scene m_scene;

public:
    GameViewID m_viewID;
    ActorID m_actorID;

    HumanDisplay(void);
    ~HumanDisplay(void);

    void VOnAttach(GameViewID p_viewID, ActorID p_actorID);
    void VOnRender(unsigned long p_deltaMillis);

    HRESULT VOnRestore(void) { return m_scene.OnRestore(); }
    HRESULT VOnLostDevice(void) { return m_scene.OnLostDevice(); }
    void VOnUpdate(unsigned long p_deltaMillis) { m_scene.OnUpdate(p_deltaMillis); }

    GameViewType VGetViewType(void) const { return IGameView::GAME_VIEW_HUMAN; }
    GameViewID VGetViewID(void) const { return m_viewID; }
    Scene& GetScene(void) { return m_scene; }

    void ActorCreatedDelegate(IEventDataPtr p_pEventData);

};

