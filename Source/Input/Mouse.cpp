#include "Mouse.h"

static const int KeyMap[] =
{
	VK_LBUTTON,		// 左ボタン
	VK_MBUTTON,		// 中ボタン
	VK_RBUTTON,		// 右ボタン
};

// ----- コンストラクタ -----
Mouse::Mouse(HWND hwnd)
	: hwnd_(hwnd)
{
}

// ----- 更新 -----
void Mouse::Update()
{
    MouseButton newButtonState = 0;

	for (int i = 0; i < ARRAYSIZE(KeyMap); ++i)
	{
		if (GetAsyncKeyState(KeyMap[i]) & 0x8000)
		{
			newButtonState |= (1 << i);
		}
	}

	// ホイール
	wheel_[1] = wheel_[0];
	wheel_[0] = 0;

	// ボタン
	buttonState_[1] = buttonState_[0];
	buttonState_[0] = newButtonState;
	buttonDown_ = ~buttonState_[1] & newButtonState; // 押した瞬間
	buttonUp_	= ~newButtonState & buttonState_[1]; // 離した瞬間

	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(hwnd_, &cursor);

	// 画面のサイズを取得
	RECT rc;
	GetClientRect(hwnd_, &rc);
	UINT screenW = screenWidth_;
	UINT screenH = screenHeight_;
	UINT viewportW = rc.right - rc.left;
	UINT viewportH = rc.bottom - rc.top;

	// 画面補正
	positionX_[1] = positionX_[0];
	positionY_[1] = positionY_[0];
	positionX_[0] = (LONG)(cursor.x / static_cast<float>(viewportW) * static_cast<float>(screenW));
	positionY_[0] = (LONG)(cursor.y / static_cast<float>(viewportH) * static_cast<float>(screenH));
}
