#include "PlayerState.h"
#include "Input/Input.h"

// ---------- IdleState ----------
namespace PlayerState
{
    // ----- 初期化 -----
    void IdleState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // ----- 更新 -----
    void IdleState::Update(const float& elapsedTime)
    {
        // 移動入力判定
        const float aLx = Input::Instance().GetGamePad().GetAxisLx();
        const float aLy = Input::Instance().GetGamePad().GetAxisLy();
        if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
        {
            owner_->ChangeState(Player::STATE::Run);
            return;
        }
    }

    // ----- 終了化 -----
    void IdleState::Finalize()
    {
    }

    // ----- ImGui -----
    void IdleState::DrawDebug()
    {
    }

    // ----- アニメーション再生 -----
    void IdleState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Idle, true);
        owner_->SetTransitionTime(0.1f);
    }
}

// ---------- RunState ----------
namespace PlayerState
{
    // ----- 初期化 -----
    void RunState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // ----- 更新 -----
    void RunState::Update(const float& elapsedTime)
    {
        // 移動入力判定
        const float aLx = Input::Instance().GetGamePad().GetAxisLx();
        const float aLy = Input::Instance().GetGamePad().GetAxisLy();
        if (aLx == 0.0f && aLy == 0.0f)
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }

        owner_->Turn(elapsedTime);
    }

    // ----- 終了化 -----
    void RunState::Finalize()
    {
        owner_->SetVelocity({});
        owner_->SetMoveDirection({});
    }

    // ----- ImGui -----
    void RunState::DrawDebug()
    {
    }

    // ----- アニメーション再生 -----
    void RunState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Run, true);
        owner_->SetTransitionTime(0.1f);
    }
}

// ---------- RollState ----------
namespace PlayerState
{
    // ----- 初期化 -----
    void RollState::Initialize()
    {
    }

    // ----- 更新 -----
    void RollState::Update(const float& elapsedTime)
    {
    }

    // ----- 終了化 -----
    void RollState::Finalize()
    {
    }

    // ----- ImGui -----
    void RollState::DrawDebug()
    {
    }
}

// ---------- JumpState ----------
namespace PlayerState
{
    // ----- 初期化 -----
    void JumpState::Initialize()
    {
    }

    // ----- 更新 -----
    void JumpState::Update(const float& elapsedTime)
    {
    }

    // ----- 終了化 -----
    void JumpState::Finalize()
    {
    }

    // ----- ImGui -----
    void JumpState::DrawDebug()
    {
    }
}

// ---------- DoubleJumpState ----------
namespace PlayerState
{
    // ----- 初期化 -----
    void DoubleJumpState::Initialize()
    {
    }

    // ----- 更新 -----
    void DoubleJumpState::Update(const float& elapsedTime)
    {
    }

    // ----- 終了化 -----
    void DoubleJumpState::Finalize()
    {
    }

    // ----- ImGui -----
    void DoubleJumpState::DrawDebug()
    {
    }
}