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
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.01f);

            ImGui::DragFloat("TransitionAttack", &transitionAttack_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void IdleState::PlayAnimation()
    {
        float transitionTime = 0.1f;
        WoodMonster::Animation animationIndex = owner_->GetAnimationIndex();

        if (animationIndex == WoodMonster::Animation::Attack1_4) transitionTime = transitionAttack_;

        owner_->PlayAnimationBlend(WoodMonster::Animation::IdleCombat, true, animationSpeed_, 0.0f, transitionAttack_);
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
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
            owner_->SetRootMotionValue(rootMotionValue_);
        }

        // TODO:旋回処理 (Playerの方に向く)


        // アニメーション再生速度更新
        UpdateAnimationSpeed();

        // 終了判定
        if (owner_->GetAnimationSeconds() >= animationEndFrame_)
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
            return;
        }
    }

    // 終了化
    void AttackState::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
        owner_->SetRootMotionValue({ 1.0f, 1.0f, 1.0f });
    }

    // ImGui
    void AttackState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);

            if (ImGui::TreeNodeEx("Transition", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::DragFloat("Idle", &transitionIdle_, 0.01f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("AttackRecovery", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::DragFloat("StartFrame", &attackRecoveryStartFrame_, 0.01f);
                ImGui::DragFloat("EndFrame", &attackRecoveryEndFrame_, 0.01f);
                ImGui::DragFloat("Speed", &attackRecoverySpeed_, 0.1f);

                ImGui::TreePop();
            }

            ImGui::DragFloat3("RootMotionValue", &rootMotionValue_.x, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackState::PlayAnimation()
    {
        float transitionTime = 0.1f;
        WoodMonster::Animation animationIndex = owner_->GetAnimationIndex();

        if (animationIndex == WoodMonster::Animation::IdleCombat) transitionTime = transitionIdle_;

        owner_->PlayAnimationBlend(WoodMonster::Animation::Attack1_4, false, 1.0f, animationStartFrame_, transitionTime);
    }

    // アニメーション再生速度更新
    void AttackState::UpdateAnimationSpeed()
    {
        const float animationSeconds = owner_->GetAnimationSeconds();
        float animationSpeed = 1.0f;

        // 攻撃の後隙を作るために、アニメーション速度を遅くする
        if (animationSeconds >= attackRecoveryStartFrame_ &&
            animationSeconds <= attackRecoveryEndFrame_)
        {
            animationSpeed = attackRecoverySpeed_;
        }

        owner_->SetAnimationSpeed(animationSpeed);
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