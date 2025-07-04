#include "WoodMonsterState.h"

// ---------- IdleState ----------
namespace WoodMonsterState
{
    // 初期化
    void IdleState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        transitionTimer_ = attackTransitionTime_;
    }

    // 更新
    void IdleState::Update(const float& elapsedTime)
    {
        transitionTimer_ -= elapsedTime;

        if (transitionTimer_ < 0.0f)
        {
            owner_->ChangeState(WoodMonster::STATE::Attack);
            return;
        }
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

// ---------- AttackState ----------
namespace WoodMonsterState
{
    // 初期化
    void AttackState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void AttackState::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
            return;
        }

        // 攻撃の後隙を作る
        // 攻撃終了後のアニメーション速度をおそくする
        // アニメーションブレンドの速度を遅くする

    }

    // 終了化
    void AttackState::Finalize()
    {
    }

    // ImGui
    void AttackState::DrawDebug()
    {
    }

    // アニメーション再生
    void AttackState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Attack1_4, false);
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
        owner_->PlayAnimationBlend(WoodMonster::Animation::Target_1, false, 1.0f, 0.0f, 0.1f);
    }
}