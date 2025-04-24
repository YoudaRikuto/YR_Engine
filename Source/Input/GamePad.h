#pragma once
#include <concrt.h>
#include <winrt/windows.gaming.input.h>

using GamePadButton = unsigned int;

class GamePad
{
public:
	static const GamePadButton BTN_UP				= (1 << 0);
	static const GamePadButton BTN_RIGHT			= (1 << 1);
	static const GamePadButton BTN_DOWN				= (1 << 2);
	static const GamePadButton BTN_LEFT				= (1 << 3);
	static const GamePadButton BTN_A				= (1 << 4);
	static const GamePadButton BTN_B				= (1 << 5);
	static const GamePadButton BTN_X				= (1 << 6);
	static const GamePadButton BTN_Y				= (1 << 7);
	static const GamePadButton BTN_START			= (1 << 8);
	static const GamePadButton BTN_BACK				= (1 << 9);
	static const GamePadButton BTN_LEFT_THUMB		= (1 << 10);
	static const GamePadButton BTN_RIGHT_THUMB		= (1 << 11);
	static const GamePadButton BTN_LEFT_SHOULDER	= (1 << 12);
	static const GamePadButton BTN_RIGHT_SHOULDER	= (1 << 13);
	static const GamePadButton BTN_LEFT_TRIGGER		= (1 << 14);
	static const GamePadButton BTN_RIGHT_TRIGGER	= (1 << 15);

public:
	GamePad() {}
	~GamePad() {}

	void Update();

	void SetSlot(const int& slot) { slot_ = slot; }

	const GamePadButton GetButton()		const { return buttonState_[0]; }	// ボタン入力の状態
	const GamePadButton GetButtonDown() const { return buttonDown_; }		// ボタン押下状態
	const GamePadButton GetButtonUp()	const { return buttonUp_; }			// ボタン押上状態

	const float GetAxisLx()   const { return axisLx_; }		// 左スティック x軸 入力状態
	const float GetAxisLy()   const { return axisLy_; }		// 左スティック y軸 入力状態
	const float GetAxisRx()   const { return axisRx_; }		// 右スティック x軸 入力状態
	const float GetAxisRy()   const { return axisRy_; }		// 右スティック y軸 入力状態
	const float GetTriggerL() const { return triggerL_; }	// 左トリガー 入力状態
	const float GetTriggerR() const { return triggerR_; }	// 右トリガー 入力状態

private:
	GamePadButton		buttonState_[2] = {};
	GamePadButton		buttonDown_		= 0;
	GamePadButton		buttonUp_		= 0;
	float				axisLx_			= 0.0f;
	float				axisLy_			= 0.0f;
	float				axisRx_			= 0.0f;
	float				axisRy_			= 0.0f;
	float				triggerL_		= 0.0f;
	float				triggerR_		= 0.0f;
	int					slot_			= 0;
};

