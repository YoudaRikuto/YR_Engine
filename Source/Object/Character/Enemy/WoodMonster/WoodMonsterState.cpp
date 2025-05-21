#include "WoodMonsterState.h"

// ---------- IdleState ----------
namespace WoodMonsterState
{
    // 初期化
    void IdleState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void IdleState::Update(const float& elapsedTime)
    {

    }

    // 終了化
    void IdleState::Finalize()
    {

    }

    // ImGui
    void IdleState::DrawDebug()
    {

    }

    // アニメーション再生
    void IdleState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Idle, true);
    }
}

// ---------- FinisherTarget0State ----------
namespace WoodMonsterState
{
    // 初期化
    void FinisherTarget0State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void FinisherTarget0State::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
            return;
        }
    }

    // 終了化
    void FinisherTarget0State::Finalize()
    {
    }

    // ImGui
    void FinisherTarget0State::DrawDebug()
    {
    }

    // アニメーション再生
    void FinisherTarget0State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Target_3, false, 1.0f, 0.0f, 0.1f);
    }
}