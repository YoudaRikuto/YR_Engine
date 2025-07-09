#include "PlayerState.h"
#include "Input/Input.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Object/Character/Enemy/WoodMonster/WoodMonster.h"
#include <Resource/EffectManager.h>

// ---------- IdleState ----------
namespace PlayerState
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
        // ガード
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT)
        {
            owner_->ChangeState(Player::STATE::FinisherAttack0);
            return;
        }

        // 回避
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
        {
            owner_->ChangeState(Player::STATE::Roll);
            return;
        }

        // ジャンプ
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A)
        {
            owner_->ChangeState(Player::STATE::JumpStart);
            return;
        }

        // 通常攻撃
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->ChangeState(Player::STATE::Attack1_1);
            return;
        }

        // 移動入力判定
        const float aLx = Input::Instance().GetGamePad().GetAxisLx();
        const float aLy = Input::Instance().GetGamePad().GetAxisLy();
        if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
        {
            owner_->ChangeState(Player::STATE::Run);
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

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void IdleState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Idle, true);
    }

}

// ---------- RunState ----------
namespace PlayerState
{
    // 初期化 
    void RunState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新 
    void RunState::Update(const float& elapsedTime)
    {
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        // 回避
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
        {
            owner_->ChangeState(Player::STATE::Roll);
            return;
        }

        // ジャンプ
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A)
        {
            owner_->ChangeState(Player::STATE::JumpStart);
            return;
        }

        //通常攻撃
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->ChangeState(Player::STATE::Attack1_1);
            return;
        }

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

    // 終了化 
    void RunState::Finalize()
    {
        owner_->SetVelocity({});
        owner_->SetMoveDirection({});
    }

    // ImGui 
    void RunState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.1f);

            ImGui::DragFloat("Transition JumpEnd", &transitionJumpEnd_, 0.01f);
            ImGui::DragFloat("Transition AttackAirToFloorEnd", &transitionAttackAirToFloorEnd_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void RunState::PlayAnimation()
    {
        float transitionTime = 0.1f;
        const Player::Animation animationIndex = owner_->GetAnimationIndex();

        if (animationIndex == Player::Animation::JumpEnd) transitionTime = transitionJumpEnd_;
        else if (animationIndex == Player::Animation::AttackAirToFloor_End) transitionTime = transitionAttackAirToFloorEnd_;

        owner_->PlayAnimationBlend(Player::Animation::Run, true, animationSpeed_, 0.0f, transitionTime);
    }
}

// ---------- RollState ----------
namespace PlayerState
{
    // 初期化 
    void RollState::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();
    }

    // 更新 
    void RollState::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // 旋回処理
        owner_->Turn(elapsedTime);

        // ---------- 先行入力 ----------

        // 移動入力があれば硬直キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();
            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化 
    void RollState::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui 
    void RollState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNode("Transition Frame"))
            {
                ImGui::DragFloat("Run", &runTransitionFrame_, 0.01f);

                ImGui::TreePop();
            }

            ImGui::DragFloat3("RootMotionValue", &rootMotionValue_.x, 0.1f);

            ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void RollState::PlayAnimation()
    {
        // TODO:回避
        // カメラからみたスティック入力が、キャラクターの向きと反対なら、
        // 後ろ向きアニメーション

        // それ以外

        owner_->PlayAnimationBlend(Player::Animation::RollForward, false, animationSpeed_);

        owner_->SetRootMotionValue(rootMotionValue_);
    }
}

// ---------- JumpStartState ----------
namespace PlayerState
{
    // 初期化 
    void JumpStartState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        isJumped_ = false;
    }

    // 更新 
    void JumpStartState::Update(const float& elapsedTime)
    {
        // ---------- 斬り上げ ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y &&
            owner_->GetAnimationSeconds() < attackUpAirTransitionEndFrame_)
        {
            owner_->ChangeState(Player::STATE::AttackUpAir);
            return;
        }

        // ---------- 空中攻撃 ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X &&
            owner_->GetAnimationSeconds() >= attackAir1_1TransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::AttackAir1_1);
            return;
        }

        // ---------- 強攻撃 ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y &&
            owner_->GetAnimationSeconds() >= attackAirToFloorTransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::AttackAirToFloor);
            return;
        }

        // ---------- ダブルジャンプ ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A)
        {
            owner_->ChangeState(Player::STATE::DoubleJump);
            return;
        }

        if (isJumped_ == false && owner_->GetAnimationSeconds() >= jumpFrame_)
        {
            DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
            velocity.y = jumpPower_;
            owner_->SetVelocity(velocity);

            isJumped_ = true;
        }

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::JumpLoop);
            return;
        }

        // 旋回処理
        owner_->Turn(elapsedTime);
    }

    // 終了化 
    void JumpStartState::Finalize()
    {

    }

    // ImGui 
    void JumpStartState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("TransitionIdle", &transitionIdle_, 0.01f);
                ImGui::DragFloat("TransitionRun", &transitionRun_, 0.01f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("TransitionFrame", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::DragFloat("AttackAir1_1", &attackAir1_1TransitionFrame_, 0.01f);
                ImGui::DragFloat("AttackAirToFloor", &attackAirToFloorTransitionFrame_, 0.01f);

                ImGui::Text("Transition End Frame");
                ImGui::DragFloat("AttackUpAir", &attackUpAirTransitionEndFrame_, 0.01f);


                ImGui::TreePop();
            }

            ImGui::DragFloat("JumpFrame", &jumpFrame_, 0.1f);
            ImGui::DragFloat("JumpPower", &jumpPower_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void JumpStartState::PlayAnimation()
    {
        const Player::STATE oldState = owner_->GetOldState();
        float transitionTime = 0.1f;

        if (oldState == Player::STATE::Idle)    transitionTime = transitionIdle_;
        if (oldState == Player::STATE::Run)     transitionTime = transitionRun_;

        owner_->PlayAnimationBlend(Player::Animation::JumpStartForward, false, 1.0f, animationStartFrame_, transitionTime);
    }
}

// ---------- JumpLoopState ----------
namespace PlayerState
{
    // 初期化 
    void JumpLoopState::Initialize()
    {
        // ダブルジャンプが可能か判定
        isDoubleJumpEnabled_ = owner_->GetAnimationIndex() != Player::Animation::DoubleJump;

        // アニメーション再生
        PlayAnimation();
    }

    // 更新 
    void JumpLoopState::Update(const float& elapsedTime)
    {
        // ---------- 強攻撃 ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y)
        {
            owner_->ChangeState(Player::STATE::AttackAirToFloor);
            return;
        }

        // ダブルジャンプ
        if (isDoubleJumpEnabled_ &&
            Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A)
        {
            owner_->ChangeState(Player::STATE::DoubleJump);
            return;
        }

        // 地面に着いたら JumpEnd に遷移する
        if (owner_->GetTransform()->GetPositionY() <= landingTriggerPositionY_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::JumpEndToRun);
                return;
            }

            owner_->ChangeState(Player::STATE::JumpEnd);

            //owner_->ChangeState(Player::STATE::JumpEndToRun);
            return;
        }

        // 旋回処理
        owner_->Turn(elapsedTime);
    }

    // 終了化 
    void JumpLoopState::Finalize()
    {
    }

    // ImGui 
    void JumpLoopState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("TransitionAttackAir1", &transitionAttackAir1_, 0.01f);

            ImGui::DragFloat("LandingTriggerPositionY", &landingTriggerPositionY_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void JumpLoopState::PlayAnimation()
    {
        Player::Animation animationIndex = static_cast<Player::Animation>(owner_->GetAnimationIndex());
        float transitionTime = 0.1f;

        if (animationIndex == Player::Animation::AttackAir1_1) transitionTime = transitionAttackAir1_;


        owner_->PlayAnimationBlend(Player::Animation::JumpLoop, false, 1.0f, 0.0f, transitionTime);
    }
}

// ---------- JumpEndState ----------
namespace PlayerState
{
    // 初期化 
    void JumpEndState::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 移動処理をしない
        const DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
        owner_->SetVelocity({ 0.0f, velocity.y, 0.0f });
        owner_->SetMoveDirection({});

        EffectManager::Instance().GetEffect("landing")->Play(owner_->GetTransform()->GetPosition(), 1.0f);
    }

    // 更新 
    void JumpEndState::Update(const float& elapsedTime)
    {


        // アニメーション後隙キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_1);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() == Player::STATE::Attack1_1)
        {
            owner_->ChangeState(owner_->GetNextState());
            return;
        }


        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化 
    void JumpEndState::Finalize()
    {
    }

    // ImGui 
    void JumpEndState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("RunTransitionFrame", &runTransitionFrame_, 0.01f);
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);
            ImGui::DragFloat("TransitionTime", &transitionTime_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void JumpEndState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::JumpEnd, false, 1.0f, animationStartFrame_, transitionTime_);
    }
}

// ---------- JumpEndToRunState ----------
namespace PlayerState
{
    // 初期化 
    void JumpEndToRunState::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();
    }

    // 更新 
    void JumpEndToRunState::Update(const float& elapsedTime)
    {
        // 指定のフレームを過ぎたら、走りステートへ遷移する
        if (owner_->GetAnimationSeconds() >= animationEndFrame_)
        {
            owner_->ChangeState(Player::STATE::Run);
            return;
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_1);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() == Player::STATE::Attack1_1)
        {
            owner_->ChangeState(owner_->GetNextState());
            return;
        }
    }

    // 終了化 
    void JumpEndToRunState::Finalize()
    {
    }

    // ImGui 
    void JumpEndToRunState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);
            ImGui::DragFloat("AnimationEndFrame", &animationEndFrame_, 0.01f);
            ImGui::DragFloat("TransitionJumpLoop", &transitionJumpLoop_, 0.01f);

            ImGui::TreePop();
        }
    }

    // ----- アニメーション再生 -----
    void JumpEndToRunState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::JumpEndRun, false, 1.0f, animationStartFrame_, transitionJumpLoop_);
    }
}

// ---------- DoubleJumpState ----------
namespace PlayerState
{
    // 初期化 
    void DoubleJumpState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
        velocity.y = jumpPower_;
        owner_->SetVelocity(velocity);
    }

    // 更新 
    void DoubleJumpState::Update(const float& elapsedTime)
    {

        // ---------- 強攻撃 ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y)
        {
            owner_->ChangeState(Player::STATE::AttackAirToFloor);
            return;
        }

        //if (owner_->IsAnimationEnd())
        if (owner_->GetAnimationSeconds() >= animationEndFrame_)
        {
            owner_->ChangeState(Player::STATE::JumpLoop);
            return;
        }

        // 旋回処理
        owner_->Turn(elapsedTime);
    }

    // 終了化 
    void DoubleJumpState::Finalize()
    {
    }

    // ImGui 
    void DoubleJumpState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("AnimationEndFrame", &animationEndFrame_, 0.01f);
                ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.01f);

                ImGui::TreePop();
            }

            ImGui::DragFloat("JumpPower", &jumpPower_);

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void DoubleJumpState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::DoubleJump, false, animationSpeed_, animationStartFrame_);
        owner_->SetTransitionTime(0.1f);
    }
}

// ---------- AttackUpAirState ----------
namespace PlayerState
{
    // 初期化
    void AttackUpAirState::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        owner_->SetVelocity({});
        owner_->SetMoveDirection({});

        // 攻撃判定有効化
        owner_->SetAttackHitBoxActive(true);
    }

    // 更新
    void AttackUpAirState::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
            owner_->SetRootMotionValue(rootMotionValue_);
        }

        // TODO:空中攻撃
        // 先行入力を取る
        // 0.8 ~ 0.9 くらいで入力があれば遷移する
        // AttackAri_01

        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::AttackAir1_1);
        }

        if (owner_->GetNextState() == Player::STATE::AttackAir1_1 &&
            owner_->GetAnimationSeconds() >= attackAir1_1TransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::AttackAir1_1);
            return;
        }

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void AttackUpAirState::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
        owner_->SetRootMotionValue({ 1.0f, 1.0f, 1.0f });
    }

    // ImGui
    void AttackUpAirState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);
            ImGui::DragFloat3("RootMotionValue", &rootMotionValue_.x, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackUpAirState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackUpAir, false, 1.0f, animationStartFrame_);
    }
}

// ---------- Attack1_1State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_1State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        //アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();

        owner_->SetVelocity({});
        owner_->SetMoveDirection({});
    }

    // 更新
    void Attack1_1State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // ガード
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_2);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() == Player::STATE::Attack1_2)
        {
            if (owner_->GetAnimationSeconds() >= attack2TransitionFrame_)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }

        // アニメーション後隙キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // アニメーション再生しきったら、待機に遷移
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void Attack1_1State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void Attack1_1State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.01f);
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Run Transition Frame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack2 Transition Frame", &attack2TransitionFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.1f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void Attack1_1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_1, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- Attack1_2State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_2State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();
    }

    // 更新
    void Attack1_2State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // ガード
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_3);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() == Player::STATE::Attack1_3) {
            if (owner_->GetAnimationSeconds() >= attack3TransitionFrame_)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }

        // アニメーション後隙キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // アニメーション再生しきったら、待機に遷移
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void Attack1_2State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void Attack1_2State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.01f);
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Run Transition Frame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack3 Transition Frame", &attack3TransitionFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.1f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
    void Attack1_2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_2, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- Attack1_3State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_3State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();
    }

    // 更新
    void Attack1_3State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // ガード
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_4);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() == Player::STATE::Attack1_4) {
            if (owner_->GetAnimationSeconds() >= attack4TransitionFrame_)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }

        // アニメーション後隙キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // アニメーション再生しきったら、待機に遷移
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void Attack1_3State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void Attack1_3State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Run Transition Frame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack4 Transition Frame", &attack4TransitionFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.1f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void Attack1_3State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_3, false, animationSpeed_, animationStartFrame_);
    }
}

// ---------- Attack1_4State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_4State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();
    }

    // 更新
    void Attack1_4State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // ガード
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_1);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() == Player::STATE::Attack1_1) {
            if (owner_->GetAnimationSeconds() >= attack1TransitionFrame_)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }

        // アニメーション後隙キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // アニメーション再生しきったら、待機に遷移
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void Attack1_4State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void Attack1_4State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Run Transition Frame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack1 Transition Frame", &attack1TransitionFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.01f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void Attack1_4State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_4, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- AttackAir1_1State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_1State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();

        owner_->SetVelocity({});
        owner_->SetMoveDirection({});

        // 攻撃判定有効化
        owner_->SetAttackHitBoxActive(true);
    }

    // 更新
    void AttackAir1_1State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // 先行入力受付
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::AttackAir1_2);
        }

        // 先行入力判定
        if (owner_->GetAnimationSeconds() >= attackAir2TransitionFrame_)
        {
            if (owner_->GetNextState() == Player::STATE::AttackAir1_2)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }


        if (owner_->GetAnimationSeconds() >= jumpLoopTransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::JumpLoop);
            return;
        }
    }

    // 終了化
    void AttackAir1_1State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void AttackAir1_1State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNode("Animation"))
            {
                ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.01f);
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.01f);
                ImGui::DragFloat("AttackAir2 Transition Frame", &attackAir2TransitionFrame_, 0.01f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Transition Frame"))
            {
                ImGui::DragFloat("Jump Loop", &jumpLoopTransitionFrame_);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAir1_1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_1, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- AttackAir1_2State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_2State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();

        // 攻撃判定有効化
        owner_->SetAttackHitBoxActive(true);
    }

    // 更新
    void AttackAir1_2State::Update(const float& elapsedTime)
    {
        // ルートモーション使用
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // 先行入力受付
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::AttackAir1_3);
        }

        // 先行入力判定
        if (owner_->GetAnimationSeconds() >= attackAir3TransitionFrame_)
        {
            if (owner_->GetNextState() == Player::STATE::AttackAir1_3)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }

        if (owner_->GetAnimationSeconds() >= jumpLoopTransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::JumpLoop);
            return;
        }
    }

    // 終了化
    void AttackAir1_2State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void AttackAir1_2State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNode("Animation"))
            {
                ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.01f);
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.01f);
                ImGui::DragFloat("AttackAir3 Transition Frame", &attackAir3TransitionFrame_, 0.01f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Transition Frame"))
            {
                ImGui::DragFloat("Jump Loop", &jumpLoopTransitionFrame_);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAir1_2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_2, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- AttackAir1_3State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_3State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();

        // 攻撃判定有効化
        owner_->SetAttackHitBoxActive(true);
    }

    // 更新
    void AttackAir1_3State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // 先行入力受付
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::AttackAir1_4);
        }

        // 先行入力判定
        if (owner_->GetAnimationSeconds() >= attackAir4TransitionFrame_)
        {
            if (owner_->GetNextState() == Player::STATE::AttackAir1_4)
            {
                owner_->ChangeState(owner_->GetNextState());
                return;
            }
        }

        if (owner_->GetAnimationSeconds() >= jumpLoopTransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::JumpLoop);
            return;
        }
    }

    // 終了化
    void AttackAir1_3State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void AttackAir1_3State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNode("Animation"))
            {
                ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.01f);
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("Transition Time", &transitionTime_, 0.01f);
                ImGui::DragFloat("AttackAir4 Transition Frame", &attackAir4TransitionFrame_, 0.01f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Transition Frame"))
            {
                ImGui::DragFloat("Jump Loop", &jumpLoopTransitionFrame_);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAir1_3State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_3, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- AttackAir1_4State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_4State::Initialize()
    {
        // フラグリセット
        owner_->ResetFlags();

        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->AttackTurn();

        // 攻撃判定有効化
        owner_->SetAttackHitBoxActive(true);
    }

    // 更新
    void AttackAir1_4State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        if (owner_->GetAnimationSeconds() >= jumpLoopTransitionFrame_)
        {
            owner_->ChangeState(Player::STATE::JumpLoop);
            return;
        }
    }

    // 終了化
    void AttackAir1_4State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);
    }

    // ImGui用
    void AttackAir1_4State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNode("Animation"))
            {
                ImGui::DragFloat("Animation Speed", &animationSpeed_, 0.01f);
                ImGui::DragFloat("Animation Start Frame", &animationStartFrame_, 0.01f);
                ImGui::DragFloat("transitionTime", &transitionTime_, 0.01f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Transition Frame"))
            {
                ImGui::DragFloat("Jump Loop", &jumpLoopTransitionFrame_);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAir1_4State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_4, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- AttackAirToFloorState ----------
namespace PlayerState
{
    // 初期化
    void AttackAirToFloorState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        owner_->ChangeWeaponDataType(Player::WeaponDataType::AttackAirToFloor);

        // 移動処理を無くす
        owner_->SetVelocity({});
        owner_->SetMoveDirection({});
    }

    // 更新
    void AttackAirToFloorState::Update(const float& elapsedTime)
    {
        // <アニメーション変更>
        const Player::Animation animationIndex = owner_->GetAnimationIndex();
        if (owner_->IsAnimationEnd())
        {
            // Start -> Loop
            if (animationIndex == Player::Animation::AttackAirToFloor_Start)
            {
                owner_->PlayAnimation(Player::Animation::AttackAirToFloor_Loop, true);
            }
        }

        // <アニメーション変更> 地面に着いたら End
        if (owner_->GetTransform()->GetPositionY() <= landingTriggerPositionY_)
        {
            if (animationIndex == Player::Animation::AttackAirToFloor_Loop)
            {
                owner_->ChangeState(Player::STATE::AttackAirToFloorEnd);
                return;
            }
        }

        // 落下速度更新
        UpdateFallingSpeed();
    }

    // 終了化
    void AttackAirToFloorState::Finalize()
    {
    }

    // ImGui用
    void AttackAirToFloorState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("StartAnimation Speed", &startAnimationSpeed_, 0.01f);

                ImGui::TreePop();
            }

            ImGui::DragFloat("LandingTriggerPositionY", &landingTriggerPositionY_);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAirToFloorState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAirToFloor_Start, false, startAnimationSpeed_);

        owner_->SetTransitionTime(0.1f);
    }

    // 落下速度更新
    void AttackAirToFloorState::UpdateFallingSpeed()
    {
        const Player::Animation animationIndex = owner_->GetAnimationIndex();
        DirectX::XMFLOAT3 velocity = owner_->GetVelocity();

        if (animationIndex == Player::Animation::AttackAirToFloor_Start)
        {
            velocity.y = 0.0f;
        }
        else
        {
            velocity.y = fallingSpeed_;
        }

        owner_->SetVelocity(velocity);
    }
}

// ---------- AttackAirToFloorEndState ----------
namespace PlayerState
{
    // 初期化
    void AttackAirToFloorEndState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void AttackAirToFloorEndState::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            // 待機ステートに遷移
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }

        if (owner_->GetAnimationSeconds() >= 1.0f)
        {
            if (owner_->GetCurrentWeaponDataType() == static_cast<int>(Player::WeaponDataType::AttackAirToFloor))
            {
                owner_->ChangeWeaponDataType(Player::WeaponDataType::Default);
            }
        }

        // 移動入力により、攻撃の後隙キャンセル
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();
            if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
            {
                owner_->ChangeWeaponDataType(Player::WeaponDataType::Default);
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // 落下速度更新
        UpdateFallingSpeed();
    }

    // 終了化
    void AttackAirToFloorEndState::Finalize()
    {

    }

    // ImGui
    void AttackAirToFloorEndState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("StartAnimation Speed", &startAnimationSpeed_, 0.01f);

                ImGui::DragFloat("Run TransitionFrame", &runTransitionFrame_, 0.01f);

                ImGui::TreePop();
            }

            ImGui::DragFloat("FallingSpeed", &fallingSpeed_);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAirToFloorEndState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAirToFloor_End, false, startAnimationSpeed_, 0.1f);
    }

    void AttackAirToFloorEndState::UpdateFallingSpeed()
    {
        const Player::Animation animationIndex = owner_->GetAnimationIndex();
        DirectX::XMFLOAT3 velocity = owner_->GetVelocity();

        velocity.y = fallingSpeed_;

        owner_->SetVelocity(velocity);
    }
}

// ---------- FinisherAttack0State ----------
namespace PlayerState
{
    // 初期化
    void FinisherAttack0State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 対応する敵のステート変更
        for (Enemy* enemy : EnemyManager::Instance().GetEnemies())
        {
            if (enemy->GetEnemyType() == EnemyType::WoodMonster)
            {
                WoodMonster* woodMonster = dynamic_cast<WoodMonster*>(enemy);
                woodMonster->ChangeState(WoodMonster::STATE::FinisherTarget0);
            }
        }
    }

    // 更新
    void FinisherAttack0State::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void FinisherAttack0State::Finalize()
    {
    }

    // ImGui
    void FinisherAttack0State::DrawDebug()
    {
    }

    // アニメーション再生
    void FinisherAttack0State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Execution_1, false, 1.0f, 0.0, 0.1f);
    }
}

// ---------- FinisherAttack1State ----------
namespace PlayerState
{
    // 初期化
    void FinisherAttack1State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 対応する敵のステート変更
        for (Enemy* enemy : EnemyManager::Instance().GetEnemies())
        {
            if (enemy->GetEnemyType() == EnemyType::WoodMonster)
            {
                WoodMonster* woodMonster = dynamic_cast<WoodMonster*>(enemy);
                woodMonster->ChangeState(WoodMonster::STATE::FinisherTarget0);
            }
        }
    }

    // 更新
    void FinisherAttack1State::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void FinisherAttack1State::Finalize()
    {
    }

    // ImGui
    void FinisherAttack1State::DrawDebug()
    {
    }

    // アニメーション再生
    void FinisherAttack1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Execution_2, false, 1.0f, 0.0, 0.1f);
    }
}

// ---------- FinisherAttack2State ----------
namespace PlayerState
{
    // 初期化
    void FinisherAttack2State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 対応する敵のステート変更
        for (Enemy* enemy : EnemyManager::Instance().GetEnemies())
        {
            if (enemy->GetEnemyType() == EnemyType::WoodMonster)
            {
                WoodMonster* woodMonster = dynamic_cast<WoodMonster*>(enemy);
                woodMonster->ChangeState(WoodMonster::STATE::FinisherTarget0);
            }
        }
    }

    // 更新
    void FinisherAttack2State::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void FinisherAttack2State::Finalize()
    {
    }

    // ImGui
    void FinisherAttack2State::DrawDebug()
    {
    }

    // アニメーション再生
    void FinisherAttack2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Execution_3, false, 1.0f, 0.0, 0.1f);
    }
}

// ---------- BlockState ----------
namespace PlayerState
{
    // 初期化
    void BlockState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        isChangeState_ = false;
    }

    // 更新
    void BlockState::Update(const float& elapsedTime)
    {
        // ガード終了
        if (Input::Instance().GetGamePad().GetButtonUp() & GamePad::BTN_LEFT_SHOULDER)
        {
            isChangeState_ = true;
        }

        if (owner_->IsAnimationBlend() == false && isChangeState_)
        {
            owner_->ChangeState(Player::STATE::BlockEnd);
            return;
        }
    }

    // 終了化
    void BlockState::Finalize()
    {
    }

    // ImGui
    void BlockState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);
            ImGui::DragFloat("TransitionIdle", &transitionIdle_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void BlockState::PlayAnimation()
    {
        const Player::Animation animationIndex = owner_->GetAnimationIndex();
        float transitionTime = 0.1f;

        if (animationIndex == Player::Animation::Idle) transitionTime = transitionIdle_;

        owner_->PlayAnimationBlend(Player::Animation::BlockLoop, true, 1.0f, 0.0f, transitionTime);
    }
}

// ---------- BlockEndState ----------
namespace PlayerState
{
    // 初期化
    void BlockEndState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void BlockEndState::Update(const float& elapsedTime)
    {
        // 走り 遷移チェック
        if (owner_->GetAnimationSeconds() >= runTransitionFrame_)
        {
            const float aLx = Input::Instance().GetGamePad().GetAxisLx();
            const float aLy = Input::Instance().GetGamePad().GetAxisLy();

            if (fabsf(aLx) != 0.0f || fabsf(aLy) != 0.0f)
            {
                owner_->ChangeState(Player::STATE::Run);
                return;
            }
        }

        // ガード
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
        {
            owner_->ChangeState(Player::STATE::Block);
            return;
        }

        // 攻撃
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->ChangeState(Player::STATE::Attack1_1);
            return;
        }

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(Player::STATE::Idle);
            return;
        }
    }

    // 終了化
    void BlockEndState::Finalize()
    {
    }

    // ImGui
    void BlockEndState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("RunTransitionFrame", &runTransitionFrame_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void BlockEndState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::BlockEnd, false);
    }
}

// ---------- ParryState ----------
namespace PlayerState
{
    // 初期化
    void ParryState::Initialize()
    {
    }

    // 更新
    void ParryState::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void ParryState::Finalize()
    {
    }

    // ImGui
    void ParryState::DrawDebug()
    {
    }

    // アニメーション再生
    void ParryState::PlayAnimation()
    {
    }
}