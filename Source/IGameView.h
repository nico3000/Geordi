#pragma once

typedef unsigned int GameViewID;

class IGameView
{
public:
    enum GameViewType
    {
        GAME_VIEW_HUMAN,
    };

    virtual ~IGameView(void) {}

    virtual HRESULT VOnRestore(void) = 0;
    virtual void VOnRender(unsigned long p_deltaMillis) = 0;
    virtual HRESULT VOnLostDevice(void) = 0;
    virtual GameViewType VGetType(void) const = 0;
    virtual GameViewID VGetID(void) const = 0;
    virtual void VOnUpdate(unsigned long p_deltaMillis) = 0;
    virtual void VOnAttach(GameViewID p_gameViewID, ActorID p_actorID) = 0;

};

typedef std::shared_ptr<IGameView> StrongGameViewPtr;


