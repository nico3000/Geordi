#pragma once
class InputController
{
private:
    void Keyboard(const RAWKEYBOARD& data);
    void Mouse(const RAWMOUSE& data);

public:
    InputController(void);
    ~InputController(void);

    bool Init(VOID);
    void RawInput(LPARAM lParam, WPARAM wParam);
};

