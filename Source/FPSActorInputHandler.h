#pragma once
#include "InputController.h"
#include "IGameView.h"

class FPSActorInputHandler :
    public IPointerHandler, public IKeyboardHandler, public IGameView
{
private:
    typedef std::map<int, bool> Keys;

    GameViewID m_viewID;
    ActorID m_actorID;
    WeakActorPtr m_pActor;
    Keys m_keys;

public:
    FPSActorInputHandler(ActorID p_cameraActorID);
    ~FPSActorInputHandler(void);

    // keyboard listeners
    bool VOnKeyUp(int keycode);
    bool VOnKeyDown(int keycode);

    // pointer listeners
    bool VOnPointerMoved(int p_x, int p_y, int p_dx, int p_dy);
    bool VOnButtonDown(unsigned int p_button);
    bool VOnButtonUp(unsigned int p_button);

    // gameview functions
    HRESULT VOnRestore(void) { return S_OK; }
    void VOnRender(unsigned long p_deltaMillis) {  }
    HRESULT VOnLostDevice(void) { return S_OK; }
    GameViewType VGetViewType(void) const { return IGameView::GAME_VIEW_HUMAN; }
    GameViewID VGetViewID(void) const { return m_viewID; }

    void VOnUpdate(unsigned long p_deltaMillis);
    void VOnAttach(GameViewID p_gameViewID, ActorID p_actorID);

};

