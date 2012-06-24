#pragma once
#include "IKeyboardHandler.h"
#include "IPointerHandler.h"

typedef std::shared_ptr<IKeyboardHandler> StrongKeyboardHandlerPtr;
typedef std::shared_ptr<IPointerHandler> StrongPointerHandlerPtr;

class InputController
{
private:
    typedef std::list<StrongKeyboardHandlerPtr> KeyboardHandlerList;
    typedef std::list<StrongPointerHandlerPtr> PointerHandlerList;

    KeyboardHandlerList m_keyboardHandlers;
    PointerHandlerList m_pointerHandlers;

    void Keyboard(const RAWKEYBOARD& data);
    void Mouse(const RAWMOUSE& data);

public:
    InputController(void);
    ~InputController(void);

    bool Init(VOID);
    void OnOpdate(void);
    void RawInput(LPARAM lParam, WPARAM wParam);
    void UnregisterKeyboardHandler(StrongKeyboardHandlerPtr pKeyboardHandler);
    void UnregisterPointerHandler(StrongPointerHandlerPtr pPointerHandler);

    void RegisterKeyboardHandler(StrongKeyboardHandlerPtr pKeyboardHandler) {this->UnregisterKeyboardHandler(pKeyboardHandler); m_keyboardHandlers.push_back(pKeyboardHandler); }
    void RegisterPointerHandler(StrongPointerHandlerPtr pPointerHandler) { this->UnregisterPointerHandler(pPointerHandler); m_pointerHandlers.push_back(pPointerHandler); }

};
