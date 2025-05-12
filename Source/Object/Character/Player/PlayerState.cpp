#include "PlayerState.h"
#include "Input/Input.h"

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
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A)
        {
            owner_->ChangeState(Player::STATE::JumpStart);
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
        if (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_A)
        {
            owner_->ChangeState(Player::STATE::JumpStart);
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
    }

    // アニメーション再生 
    void RunState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::Run, true);
        owner_->SetTransitionTime(0.1f);
    }
}

// ---------- RollState ----------
namespace PlayerState
{
    // 初期化 
    void RollState::Initialize()
    {
    }

    // 更新 
    void RollState::Update(const float& elapsedTime)
    {
    }

    // 終了化 
    void RollState::Finalize()
    {
    }

    // ImGui 
    void RollState::DrawDebug()
    {
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
        isDoubleJumpEnabled_ = owner_->GetAnimationIndex() != static_cast<int>(Player::Animation::DoubleJump);

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
        if(owner_->GetTransform()->GetPositionY() <= landingTriggerPositionY_)
        {
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
            ImGui::DragFloat("LandingTriggerPositionY", &landingTriggerPositionY_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生 
    void JumpLoopState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::JumpLoop, false);
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

        // 移動処理をしない
        const DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
        owner_->SetVelocity({ 0.0f, velocity.y, 0.0f });
        owner_->SetMoveDirection({});
    }

    // 更新 
    void JumpEndToRunState::Update(const float& elapsedTime)
    {
        if (owner_->GetAnimationSeconds() >= animationEndFrame_)
        {
            owner_->ChangeState(Player::STATE::Idle);
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
            ImGui::DragFloat("AnimationEndFrame", &animationEndFrame_, 0.01f);

            ImGui::TreePop();
        }
    }

    // ----- アニメーション再生 -----
    void JumpEndToRunState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::JumpEndRun, false);
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
    }

    // 更新
    void Attack1_1State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void Attack1_1State::Finalize()
    {
    }

    // ImGui用
    void Attack1_1State::DrawDebug()
    {
    }
}

// ---------- Attack1_2State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_2State::Initialize()
    {
    }

    // 更新
    void Attack1_2State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void Attack1_2State::Finalize()
    {
    }

    // ImGui用
    void Attack1_2State::DrawDebug()
    {
    }
}

// ---------- Attack1_3State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_3State::Initialize()
    {
    }

    // 更新
    void Attack1_3State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void Attack1_3State::Finalize()
    {
    }

    // ImGui用
    void Attack1_3State::DrawDebug()
    {
    }
}

// ---------- Attack1_4State ----------
namespace PlayerState
{
    // 初期化
    void Attack1_4State::Initialize()
    {
    }

    // 更新
    void Attack1_4State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void Attack1_4State::Finalize()
    {
    }

    // ImGui用
    void Attack1_4State::DrawDebug()
    {
    }
}

// ---------- AttackAir1_1State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_1State::Initialize()
    {
    }

    // 更新
    void AttackAir1_1State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void AttackAir1_1State::Finalize()
    {
    }

    // ImGui用
    void AttackAir1_1State::DrawDebug()
    {
    }
}

// ---------- AttackAir1_2State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_2State::Initialize()
    {
    }

    // 更新
    void AttackAir1_2State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void AttackAir1_2State::Finalize()
    {
    }

    // ImGui用
    void AttackAir1_2State::DrawDebug()
    {
    }
}

// ---------- AttackAir1_3State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_3State::Initialize()
    {
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
}

// ---------- AttackAir1_4State ----------
namespace PlayerState
{
    // 初期化
    void AttackAir1_4State::Initialize()
    {
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

        owner_->SetVelocity({});
        owner_->SetMoveDirection({});
    }

    // 更新
    void AttackAirToFloorState::Update(const float& elapsedTime)
    {
        // <アニメーション変更>
        const Player::Animation animationIndex = static_cast<Player::Animation>(owner_->GetAnimationIndex());
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

            ImGui::DragFloat("FallingSpeed", &fallingSpeed_);

            ImGui::DragFloat("LandingTriggerPositionY", &landingTriggerPositionY_);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void AttackAirToFloorState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(Player::Animation::AttackAirToFloor_Start, false, startAnimationSpeed_);
    }

    // 落下速度更新
    void AttackAirToFloorState::UpdateFallingSpeed()
    {
        const Player::Animation animationIndex = static_cast<Player::Animation>(owner_->GetAnimationIndex());
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