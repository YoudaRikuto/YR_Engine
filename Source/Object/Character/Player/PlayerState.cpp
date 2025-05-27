#include "PlayerState.h"
#include "Input/Input.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Object/Character/Enemy/WoodMonster/WoodMonster.h"

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
    }

    // アニメーション再生 
    void IdleState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Idle, true);
        owner_->SetTransitionTime(0.1f);
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
            ImGui::DragFloat3("RootMotionValue", &rootMotionValue_.x, 0.1f);

            ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void RollState::PlayAnimation()
    {
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
        // ---------- 空中攻撃 ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->ChangeState(Player::STATE::AttackAir1_1);
            return;
        }

        // ---------- 強攻撃 ----------
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_Y)
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
        // アニメーション再生
        PlayAnimation();

        // 移動処理をしない
        const DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
        owner_->SetVelocity({ 0.0f, velocity.y, 0.0f });
        owner_->SetMoveDirection({});
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

// ---------- Attack1_1State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_1State::Initialize()
    {
        //アニメーション再生
        PlayAnimation();

        //先行入力を判定するためにセットしておく
        owner_->SetNextState(Player::STATE::Attack1_1);
    }

    // 更新
    void Attack1_1State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // 先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_2);
        }

        // 先行入力があった場合切り替える
        if (owner_->GetNextState() != Player::STATE::Attack1_1)
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
                ImGui::DragFloat("RunTransitionFrame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack2TransitionFrame_", &attack2TransitionFrame_, 0.01f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void Attack1_1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_1, false);
    }
}

// ---------- Attack1_2State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_2State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        //先行入力を判定するためにセットしておく
        owner_->SetNextState(Player::STATE::Attack1_2);
    }

    // 更新
    void Attack1_2State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        //先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_3);
        }

        //先行入力があった場合切り替える
        if (owner_->GetNextState() != Player::STATE::Attack1_2) {
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
                ImGui::DragFloat("RunTransitionFrame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack3TransitionFrame_", &attack3TransitionFrame_, 0.01f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
    void Attack1_2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_2, false);
    }
}

// ---------- Attack1_3State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_3State::Initialize()
    {
        PlayAnimation();

        //先行入力を判定するためにセットしておく
        owner_->SetNextState(Player::STATE::Attack1_3);
    }

    // 更新
    void Attack1_3State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        //先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_4);
        }

        //先行入力があった場合切り替える
        if (owner_->GetNextState() != Player::STATE::Attack1_3) {
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
                ImGui::DragFloat("RunTransitionFrame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack4TransitionFrame_", &attack4TransitionFrame_, 0.01f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void Attack1_3State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_3, false);
    }
}

// ---------- Attack1_4State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_4State::Initialize()
    {
        PlayAnimation();

        //先行入力を判定するためにセットしておく
        owner_->SetNextState(Player::STATE::Attack1_4);
    }

    // 更新
    void Attack1_4State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        //先行入力ステート記録
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X)
        {
            owner_->SetNextState(Player::STATE::Attack1_1);
        }

        //先行入力があった場合切り替える
        if (owner_->GetNextState() != Player::STATE::Attack1_4) {
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
                ImGui::DragFloat("RunTransitionFrame", &runTransitionFrame_, 0.01f);
                ImGui::DragFloat("Attack1TransitionFrame_", &attack1TransitionFrame_, 0.01f);
                ImGui::DragFloat("TransitionTime", &transitionTime_, 0.01f);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void Attack1_4State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Attack1_4, false, 1.0f, 0.0f, transitionTime_);
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

        owner_->SetVelocity({});
        owner_->SetMoveDirection({});
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
            if (owner_->GetNextState() == Player::STATE::AttackAir1_2) owner_->ChangeState(owner_->GetNextState());
        }


        if(owner_->GetAnimationSeconds() >= jumpLoopTransitionFrame_)
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

                ImGui::TreePop();
            }

            ImGui::DragFloat("JumpLoop Transition Frame", &jumpLoopTransitionFrame_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAir1_1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_1, false, animationSpeed_, animationStartFrame_);
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
    }

    // 更新
    void AttackAir1_2State::Update(const float& elapsedTime)
    {
        // ルートモーション使用
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
            ImGui::DragFloat("Animation Speed", &animationSpeed_);
            ImGui::DragFloat("Transition AttackAir1_1", &transitionAttackAir1_1_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAir1_2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_2, false, animationSpeed_, 0.0f, transitionAttackAir1_1_);
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
    }

    // 更新
    void AttackAir1_3State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void AttackAir1_3State::Finalize()
    {
    }

    // ImGui用
    void AttackAir1_3State::DrawDebug()
    {
    }

    // アニメーション再生
    void AttackAir1_3State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_3, false);
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
    }

    // 更新
    void AttackAir1_4State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void AttackAir1_4State::Finalize()
    {
    }

    // ImGui用
    void AttackAir1_4State::DrawDebug()
    {
    }

    // アニメーション再生
    void AttackAir1_4State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAir1_4, false);
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
            // 待機ステートに遷移
            else if (animationIndex == Player::Animation::AttackAirToFloor_End)
            {
                owner_->ChangeState(Player::STATE::Idle);
                return;
            }
        }

        if (animationIndex == Player::Animation::AttackAirToFloor_End &&
            owner_->GetAnimationSeconds() >= 1.0f)
        {
            if (owner_->GetCurrentWeaponDataType() == static_cast<int>(Player::WeaponDataType::AttackAirToFloor))
            {
                owner_->ChangeWeaponDataType(Player::WeaponDataType::Default);
            }
        }

        // <アニメーション変更> 地面に着いたら End
        if (owner_->GetTransform()->GetPositionY() <= landingTriggerPositionY_)
        {
            if (animationIndex == Player::Animation::AttackAirToFloor_Loop)
            {
                owner_->PlayAnimationBlend(Player::Animation::AttackAirToFloor_End, false, 1.0f, 0.1f);
            }
        }

        // 移動入力により、攻撃の後隙キャンセル
        if (animationIndex == Player::Animation::AttackAirToFloor_End &&
            owner_->GetAnimationSeconds() >= runTransitionFrame_)
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

                ImGui::DragFloat("Run TransitionFrame", &runTransitionFrame_, 0.01f);

                ImGui::TreePop();
            }

            ImGui::DragFloat("FallingSpeed", &fallingSpeed_);

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