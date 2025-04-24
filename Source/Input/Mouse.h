#pragma once
#include <Windows.h>

using MouseButton = unsigned int;

class Mouse
{
public:
    Mouse(HWND hwnd);
    ~Mouse() {}

    void Update();

	const MouseButton GetButton()		const { return buttonState_[0]; }	// ボタン入力の状態
	const MouseButton GetButtonDown()	const { return buttonDown_; }		// ボタン押下状態
	const MouseButton GetButtonUp()		const { return buttonUp_; }			// ボタン押上状態

	void SetWheel(const int& wheel) { wheel_[0] += wheel; }
	const int GetWheel() const { return wheel_[1]; }

	const int GetPositionX()	const { return positionX_[0]; }
	const int GetPositionY()	const { return positionY_[0]; }
	const int GetOldPositionX() const { return positionX_[1]; }
	const int GetOldPositionY() const { return positionY_[1]; }

	void SetScreenWidth(const int& width) { screenWidth_ = width; }
	void SetScreenHeight(const int& height) { screenHeight_ = height; }
	const int GetScreenWidth()	const { return screenWidth_; }
	const int GetScreenHeight() const { return screenHeight_; }

private:
	MouseButton		buttonState_[2] = {};
	MouseButton		buttonDown_		= 0;
	MouseButton		buttonUp_		= 0;
	int				positionX_[2]	= {};
	int				positionY_[2]	= {};
	int				wheel_[2]		= {};
	int				screenWidth_	= 0;
	int				screenHeight_	= 0;
	HWND			hwnd_			= nullptr;
};

