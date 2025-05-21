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
        owner_->PlayAnimation(WoodMonster::Animation::Idle, true);
    }
}


namespace WoodMonsterState
{
}