#include "Input.h"

Input* Input::instance_ = nullptr;

// ----- コンストラクタ -----
Input::Input(HWND hwnd)
    : mouse_(hwnd)
{
    instance_ = this;
}

// ----- 更新 -----
void Input::Update()
{
    gamePad_.Update();
    mouse_.Update();
}
