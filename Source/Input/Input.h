#pragma once
#include "GamePad.h"
#include "Mouse.h"

class Input
{
public:
    Input(HWND hwnd);
    ~Input() {}

    static Input& Instance() { return *instance_; }

    void Update();

    GamePad& GetGamePad() { return gamePad_; }
    Mouse& GetMouse() { return mouse_; }

private:
    static Input* instance_;
    GamePad gamePad_;
    Mouse   mouse_;
};

