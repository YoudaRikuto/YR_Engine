#include <Windows.h>
#include <math.h>
#include <Xinput.h>
#include "GamePad.h"

#pragma comment (lib,"Xinput.lib")

// 更新
void GamePad::Update()
{
    axisLx_ = axisLy_ = 0.0f;
    axisRx_ = axisRy_ = 0.0f;
    triggerL_ = triggerR_ = 0.0f;

    GamePadButton newButtonState = 0;

    XINPUT_STATE xinputState;
    if (XInputGetState(slot_, &xinputState) == ERROR_SUCCESS)
    {
        XINPUT_GAMEPAD& pad = xinputState.Gamepad;

		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)					newButtonState |= BTN_UP;
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)				newButtonState |= BTN_RIGHT;
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)				newButtonState |= BTN_DOWN;
		if (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)				newButtonState |= BTN_LEFT;
		if (pad.wButtons & XINPUT_GAMEPAD_A)						newButtonState |= BTN_A;
		if (pad.wButtons & XINPUT_GAMEPAD_B)						newButtonState |= BTN_B;
		if (pad.wButtons & XINPUT_GAMEPAD_X)						newButtonState |= BTN_X;
		if (pad.wButtons & XINPUT_GAMEPAD_Y)						newButtonState |= BTN_Y;
		if (pad.wButtons & XINPUT_GAMEPAD_START)					newButtonState |= BTN_START;
		if (pad.wButtons & XINPUT_GAMEPAD_BACK)						newButtonState |= BTN_BACK;
		if (pad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)				newButtonState |= BTN_LEFT_THUMB;
		if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)				newButtonState |= BTN_RIGHT_THUMB;
		if (pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)			newButtonState |= BTN_LEFT_SHOULDER;
		if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)			newButtonState |= BTN_RIGHT_SHOULDER;
		if (pad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)	newButtonState |= BTN_LEFT_TRIGGER;
		if (pad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)	newButtonState |= BTN_RIGHT_TRIGGER;

		if ((pad.sThumbLX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
			(pad.sThumbLY <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
		{
			pad.sThumbLX = 0;
			pad.sThumbLY = 0;
		}

		if ((pad.sThumbRX <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
			(pad.sThumbRY <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
		{
			pad.sThumbRX = 0;
			pad.sThumbRY = 0;
		}

		triggerL_	= static_cast<float>(pad.bLeftTrigger) / 255.0f;
		triggerR_	= static_cast<float>(pad.bRightTrigger) / 255.0f;
		axisLx_		= static_cast<float>(pad.sThumbLX) / static_cast<float>(0x8000);
		axisLy_		= static_cast<float>(pad.sThumbLY) / static_cast<float>(0x8000);
		axisRx_		= static_cast<float>(pad.sThumbRX) / static_cast<float>(0x8000);
		axisRy_		= static_cast<float>(pad.sThumbRY) / static_cast<float>(0x8000);
    }    

	float lx = 0.0f;
	float ly = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	if (GetAsyncKeyState('D') & 0x8000) lx = 1.0f;
	if (GetAsyncKeyState('A') & 0x8000) lx = -1.0f;
	if (GetAsyncKeyState('W') & 0x8000) ly = 1.0f;
	if (GetAsyncKeyState('S') & 0x8000) ly = -1.0f;
	if (GetAsyncKeyState('L') & 0x8000) rx = 1.0f;
	if (GetAsyncKeyState('J') & 0x8000) rx = -1.0f;
	if (GetAsyncKeyState('I') & 0x8000) ry = 1.0f;
	if (GetAsyncKeyState('K') & 0x8000) ry = -1.0f;
	if (GetAsyncKeyState(' ') & 0x8000) newButtonState |= BTN_A;
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) newButtonState |= BTN_B;
	if (GetAsyncKeyState('E') & 0x8000) newButtonState |= BTN_X;
	if (GetAsyncKeyState('Q') & 0x8000) newButtonState |= BTN_RIGHT_THUMB;
	if (GetAsyncKeyState('F') & 0x8000) newButtonState |= BTN_LEFT_SHOULDER;
	if (GetAsyncKeyState('Q') & 0x8000) newButtonState |= BTN_Y;
	if (GetAsyncKeyState(VK_UP) & 0x8000)	newButtonState |= BTN_UP;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)	newButtonState |= BTN_RIGHT;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)	newButtonState |= BTN_DOWN;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)	newButtonState |= BTN_LEFT;
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)newButtonState |= BTN_START;
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)newButtonState |= BTN_RIGHT_SHOULDER;
	
	if (lx >= 1.0f || lx <= -1.0f || ly >= 1.0f || ly <= -1.0)
	{
		float power = ::sqrtf(lx * lx + ly * ly);
		axisLx_ = lx / power;
		axisLy_ = ly / power;
	}
	if (rx >= 1.0f || rx <= -1.0f || ry >= 1.0f || ry <= -1.0)
	{
		float power = ::sqrtf(rx * rx + ry * ry);
		axisRx_ = rx / power;
		axisRy_ = ry / power;
	}

	buttonState_[1] = buttonState_[0];						// スイッチ履歴
	buttonState_[0] = newButtonState;
	buttonDown_		= ~buttonState_[1] & newButtonState;	// 押した瞬間
	buttonUp_		= ~newButtonState & buttonState_[1];	// 離した瞬間
}
