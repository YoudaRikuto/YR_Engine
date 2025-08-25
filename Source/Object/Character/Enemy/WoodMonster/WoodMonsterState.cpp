#include "WoodMonsterState.h"
#include "Object/Character/Player/PlayerManager.h"

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
            if (changeStateType_ == 0) owner_->ChangeState(WoodMonster::STATE::Attack3_1);
            else if (changeStateType_ == 1) owner_->ChangeState(WoodMonster::STATE::Attack3_2);
            else owner_->ChangeState(WoodMonster::STATE::Attack3_3);

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
            ImGui::SliderInt("ChangeStateType", &changeStateType_, 0, 2);

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

// ---------- Attack3_1State ----------
namespace WoodMonsterState
{
    // 初期化
    void Attack3_1State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();   

        isAttackHitBoxActive_ = false;
    }

    // 更新
    void Attack3_1State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // アニメーション再生速度更新
        UpdateAnimationSpeed();

        // 攻撃判定有効化
        const float animationSeconds = owner_->GetAnimationSeconds();
        if(animationSeconds >= attackHitBoxActiveStartFrame_&&
            isAttackHitBoxActive_ == false)
        {
            owner_->SetAttackActiveFlag("Attack3_1");
            owner_->SetAttackHitBoxActive(true);
            isAttackHitBoxActive_ = true;
        }
        // 攻撃判定無効化
        if (animationSeconds > attackHitBoxActiveEndFrame_ &&
            owner_->IsAttackHitBoxActive())
        {
            owner_->SetAttackHitBoxActive(false);
        }


        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Attack3_2);
            return;
        }
    }

    // 終了化
    void Attack3_1State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);

        // 攻撃判定を無効化
        owner_->AttackActiveFlagAllClear();
        owner_->SetAttackHitBoxActive(false);
    }

    // ImGui
    void Attack3_1State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.1f);
            ImGui::DragFloat("PreAttackAnimationSpeed", &preAttackAnimationSpeed_, 0.1f);
            ImGui::DragFloat("AttackAnimationSpeed", &attackAnimationSpeed_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void Attack3_1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Attack3_1, false, animationSpeed_);
    }

    // アニメーション再生速度更新
    void Attack3_1State::UpdateAnimationSpeed()
    {
        const float animationSeconds = owner_->GetAnimationSeconds();
        const float currentAnimationSpeed = owner_->GetAnimationSpeed();
        float targetSpeed = 0.0f;        

        if (animationSeconds <= preAttackEndFrame_) targetSpeed = preAttackAnimationSpeed_;
        else targetSpeed = attackAnimationSpeed_;

        const float animationSpeed = XMFloatLerp(currentAnimationSpeed, targetSpeed, 0.5f);
        owner_->SetAnimationSpeed(animationSpeed);
    }
}

// ---------- Attack3_2State ----------
namespace WoodMonsterState
{
    // 初期化
    void Attack3_2State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        isAttackHitBoxActive_ = false;
    }

    // 更新
    void Attack3_2State::Update(const float& elapsedTime)
    {
        // ルートモーションを使用する
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // 攻撃判定有効化
        const float animationSeconds = owner_->GetAnimationSeconds();
        if (animationSeconds >= attackHitBoxActiveStartFrame_ &&
            isAttackHitBoxActive_ == false)
        {
            owner_->SetAttackActiveFlag("Attack3_2");
            owner_->SetAttackHitBoxActive(true);
            isAttackHitBoxActive_ = true;
        }
        // 攻撃判定無効化
        if (animationSeconds > attackHitBoxActiveEndFrame_ &&
            owner_->IsAttackHitBoxActive())
        {
            owner_->SetAttackHitBoxActive(false);
        }

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
        }
    }

    // 終了化
    void Attack3_2State::Finalize()
    {
        // ルートモーション使用終了
        owner_->UseRootMotion(false);

        // 攻撃判定を無効化
        owner_->AttackActiveFlagAllClear();
        owner_->SetAttackHitBoxActive(false);
    }

    // ImGui
    void Attack3_2State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::TreeNodeEx("AttackHitBoxActive", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::DragFloat("StartFrame",  &attackHitBoxActiveStartFrame_, 0.01f);
                ImGui::DragFloat("EndFrame",    &attackHitBoxActiveEndFrame_,   0.01f);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void Attack3_2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Attack3_2, false);
    }
}

// ---------- Attack3_3State ----------
namespace WoodMonsterState
{
    // 初期化
    void Attack3_3State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void Attack3_3State::Update(const float& elapsedTime)
    {
    }

    // 終了化
    void Attack3_3State::Finalize()
    {
    }

    // ImGui
    void Attack3_3State::DrawDebug()
    {
    }

    // アニメーション再生
    void Attack3_3State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Attack3_3, false);
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

        isAttackHitBoxActive_ = false;
        isRightHandAttackEndActive_ = false;
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

        // 旋回処理 (Playerの方に向く)
        owner_->Turn(elapsedTime, PlayerManager::Instance().GetTransform()->GetPosition());

        // 攻撃判定を有効化する
        if (isAttackHitBoxActive_ == false &&
            owner_->GetAnimationSeconds() >= rightHandAttackActiveFrame_)
        {
            owner_->SetAttackActiveFlag("RightHandAttack");
            owner_->SetAttackHitBoxActive(true);
            isAttackHitBoxActive_ = true;
        }
        if (isRightHandAttackEndActive_ == false &&
            owner_->GetAnimationSeconds() >= rightHandAttackEndActiveFrame_)
        {
            owner_->SetAttackActiveFlag("RightHandAttack_Last");
            isRightHandAttackEndActive_ = true;
        }

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

        // 攻撃判定を無効化
        owner_->AttackActiveFlagAllClear();
        owner_->SetAttackHitBoxActive(false);
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

            if (ImGui::TreeNodeEx("Attack", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::Text("RightHandAttackActiveFrame");
                ImGui::DragFloat("##RightHandAttackActiveFrame ", &rightHandAttackActiveFrame_, 0.01f);
                ImGui::Text("RightHandAttackEndActiveFrame");
                ImGui::DragFloat("##RightHandAttackEndActiveFrame ", &rightHandAttackEndActiveFrame_, 0.01f);


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

// ---------- HitToAirState ----------
namespace WoodMonsterState
{
    // 初期化
    void HitToAirState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->TurnToPlayer();

        // 変数初期化
        oldPositionY_ = 0.0f;
    }

    // 更新
    void HitToAirState::Update(const float& elapsedTime)
    {
        const DirectX::XMFLOAT3 playerPosition = PlayerManager::Instance().GetTransform()->GetPosition();
        const DirectX::XMFLOAT2 playerXZ = { playerPosition.x, playerPosition.z };
        const DirectX::XMFLOAT2 woodMonsterXZ = { owner_->GetTransform()->GetPositionX(), owner_->GetTransform()->GetPositionZ() };
        const DirectX::XMFLOAT2 targetPosition = playerXZ + XMFloat2Normalize(woodMonsterXZ - playerXZ) * length_;
        const DirectX::XMFLOAT2 position = XMFloat2Lerp(woodMonsterXZ, targetPosition, lerpWeight_);
        owner_->GetTransform()->SetPositionX(position.x);
        owner_->GetTransform()->SetPositionZ(position.y);

        // 斬り上げ攻撃をくらっているので, 高さをPlayerに合わせる
        float positionY = PlayerManager::Instance().GetTransform()->GetPositionY() - offsetPositionY_;
        positionY = positionY < 0.0f ? 0.0f : positionY;
        owner_->GetTransform()->SetPositionY(positionY);

        // アニメーションが終端まで行くか、落下し始めたらステート変更する
        if(owner_->IsAnimationEnd() ||
            oldPositionY_ > owner_->GetTransform()->GetPositionY())
        {
            owner_->ChangeState(WoodMonster::STATE::HitAirIdle);
            return;
        }
        
        // 現在の高さを保存する
        oldPositionY_ = owner_->GetTransform()->GetPositionY();
    }

    // 終了化
    void HitToAirState::Finalize()
    {
        // 次のステートにつながりやすいように設定する
        float playerPositionY = PlayerManager::Instance().GetTransform()->GetPositionY();
        playerPositionY -= offsetPositionY_;
        owner_->GetTransform()->SetPositionY(playerPositionY);
    }

    // ImGui
    void HitToAirState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("OffsetPositionY", &offsetPositionY_, 0.01f);

            ImGui::DragFloat("Length", &length_, 0.1f);
            ImGui::DragFloat("LerpWEight", &lerpWeight_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void HitToAirState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::HitAir1, false);
    }
}

// ---------- HitAirIdleState ----------
namespace WoodMonsterState
{
    // 初期化
    void HitAirIdleState::Initialize()
    {
        fallStartTimer_ = fallStartTime_;
    }

    // 更新
    void HitAirIdleState::Update(const float& elapsedTime)
    {
        fallStartTimer_ -= elapsedTime;

        DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
        if (fallStartTimer_ > 0.0f)
        {
            velocity.y = -fallSpeed_;
        }
        else
        {
            velocity.y -= gravity_ * elapsedTime;
        }
        owner_->SetVelocity(velocity);
        owner_->GetTransform()->AddPosition(velocity * elapsedTime);


        if (owner_->GetTransform()->GetPositionY() <= 0.0f)
        {
            owner_->GetTransform()->SetPositionY(0.0f);
            owner_->ChangeState(WoodMonster::STATE::Idle);
        }
    }

    // 終了化
    void HitAirIdleState::Finalize()
    {
        owner_->SetVelocity({});
    }

    // ImGui
    void HitAirIdleState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("FallTimer", &fallStartTimer_);
            ImGui::DragFloat("FallTime", &fallStartTime_, 0.1f);
            ImGui::DragFloat("Gravity", &gravity_, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("FallSpeed", &fallSpeed_, 0.1f, 0.0f, 100.0f);

            ImGui::TreePop();
        }
    }
}

// ---------- HitAir1State ----------
namespace WoodMonsterState
{
    // 初期化
    void HitAir1State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->TurnToPlayer();
    }

    // 更新
    void HitAir1State::Update(const float& elapsedTime)
    {
        const DirectX::XMFLOAT3 vec = PlayerManager::Instance().GetPlayer()->GetRootMotionDelta();
        owner_->GetTransform()->AddPosition(vec);        

        if(owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::HitAirIdle);
            return;
        }
    }

    // 終了化
    void HitAir1State::Finalize()
    {
    }

    // ImGui
    void HitAir1State::DrawDebug()
    {
    }

    // アニメーション再生
    void HitAir1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::HitAir1, false);
    }
}

// ---------- HitAir2State ----------
namespace WoodMonsterState
{
    // 初期化
    void HitAir2State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->TurnToPlayer();
    }

    // 更新
    void HitAir2State::Update(const float& elapsedTime)
    {
        DirectX::XMFLOAT3 vec = PlayerManager::Instance().GetPlayer()->GetRootMotionDelta();
        owner_->GetTransform()->AddPosition(vec);

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::HitAirIdle);
            return;
        }
    }

    // 終了化
    void HitAir2State::Finalize()
    {
    }

    // ImGui
    void HitAir2State::DrawDebug()
    {
    }

    // アニメーション再生
    void HitAir2State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::HitAir2, false);
    }
}

// ---------- HitAir3State ----------
namespace WoodMonsterState
{
    // 初期化
    void HitAir3State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        // 旋回処理
        owner_->TurnToPlayer();
    }

    // 更新
    void HitAir3State::Update(const float& elapsedTime)
    {
        const DirectX::XMFLOAT3 vec = PlayerManager::Instance().GetPlayer()->GetRootMotionDelta();
        owner_->GetTransform()->AddPosition(vec);

        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::HitAirIdle);
            return;
        }
    }

    // 終了化
    void HitAir3State::Finalize()
    {
    }

    // ImGui
    void HitAir3State::DrawDebug()
    {
    }

    // アニメーション再生
    void HitAir3State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::HitAir3, false);
    }
}

// ---------- BlockHitBreakState ----------
namespace WoodMonsterState
{
    // 初期化
    void BlockHitBreakState::Initialize()
    {
        const WoodMonster::Animation animationIndex = owner_->GetAnimationIndex();
        if (animationIndex == WoodMonster::Animation::Attack3_1)
        {
            owner_->SetNextState(WoodMonster::STATE::Attack3_2);
        }

        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void BlockHitBreakState::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationBlend() == false && owner_->IsRootMotionActive() == false)
        {
            owner_->UseRootMotion(true);
        }

        // コンボ攻撃時は、指定のフレームで遷移する
        if (owner_->GetNextState() == WoodMonster::STATE::Attack3_2 &&
            owner_->GetAnimationSeconds() >= attack3_2TransitionFrame_)
        {
            owner_->ChangeState(WoodMonster::STATE::Attack3_2);
            return;
        }

        if (owner_->GetAnimationSeconds() >= downTransitionFrame_)
        {
            owner_->ChangeState(WoodMonster::STATE::Down);
            return;
        }

        // アニメーション再生終了
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
            return;
        }
    }

    // 終了化
    void BlockHitBreakState::Finalize()
    {
        owner_->UseRootMotion(false);

        // TODO:仮
        owner_->SetNextState(WoodMonster::STATE::Idle);
    }

    // ImGui
    void BlockHitBreakState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("Attack3_2TransitionFrame", &attack3_2TransitionFrame_, 0.01f);

            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.01f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void BlockHitBreakState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::BlockHitBreak, false, 1.0f, animationStartFrame_);
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

// ---------- FinisherTarget1State ----------
namespace WoodMonsterState
{
    // 初期化
    void FinisherTarget1State::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void FinisherTarget1State::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
            return;
        }
    }

    // 終了化
    void FinisherTarget1State::Finalize()
    {
    }

    // ImGui
    void FinisherTarget1State::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.1f);
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.1f);
            ImGui::DragFloat("TransitionTime", &transitionTime_, 0.1f);

            ImGui::TreePop();
        }
    }
    
    // アニメーション再生
    void FinisherTarget1State::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::Target_2, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- DownState ----------
namespace WoodMonsterState
{
    // 初期化
    void DownState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        downTimer_          = 0.0f;
        isDownTimerActive_  = false;

        owner_->SetSphereDraw(true);
        owner_->SetSphereScale(0.0f);
    }

    // 更新
    void DownState::Update(const float& elapsedTime)
    {
        const WoodMonster::Animation animationIndex = owner_->GetAnimationIndex();

        // ループアニメーションに切り替える
        if (owner_->IsAnimationEnd() && animationIndex == WoodMonster::Animation::KnockDownStart)
        {
            owner_->PlayAnimation(WoodMonster::Animation::KnockDownLoop, true);
            isDownTimerActive_ = true;
        }
        
        const float animationSeconds = owner_->GetAnimationSeconds();
        if (animationIndex == WoodMonster::Animation::KnockDownStart &&
            animationSeconds >= sphereScalingStartFrame_ && animationSeconds <= sphereScalingEndFrame_)
        {
            const float maxTime = sphereScalingEndFrame_ - sphereScalingStartFrame_;
            const float currentTime = animationSeconds - sphereScalingStartFrame_;
            const float scale = XMFloatLerp(0.0f, 1.0f, currentTime / maxTime);
            owner_->SetSphereScale(scale);
        }

        if (isDownTimerActive_)
        {
            downTimer_ += elapsedTime;
        }

        if (downTimer_ >= downTime_)
        {
            //owner_->ChangeState(WoodMonster::STATE::DownEnd);
            return;
        }
    }

    // 終了化
    void DownState::Finalize()
    {
        owner_->SetSphereDraw(false);
    }

    // ImGui
    void DownState::DrawDebug()
    {
    }

    // アニメーション再生
    void DownState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::KnockDownStart, false);
    }
}

// ---------- DownEndState ----------
namespace WoodMonsterState
{
    // 初期化
    void DownEndState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();
    }

    // 更新
    void DownEndState::Update(const float& elapsedTime)
    {
        if (owner_->IsAnimationEnd())
        {
            owner_->ChangeState(WoodMonster::STATE::Idle);
            return;
        }
    }

    // 終了化
    void DownEndState::Finalize()
    {
    }

    // ImGui
    void DownEndState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("AnimationSpeed", &animationSpeed_, 0.1f);
            ImGui::DragFloat("AnimationStartFrame", &animationStartFrame_, 0.1f);
            ImGui::DragFloat("TransitionTime", &transitionTime_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void DownEndState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::KnockDownGetUp, false, animationSpeed_, animationStartFrame_, transitionTime_);
    }
}

// ---------- HitFrontState ----------
namespace WoodMonsterState
{
    // 初期化
    void HitFrontState::Initialize()
    {
        // アニメーション再生
        PlayAnimation();

        knockBackDirection_ = owner_->GetTransform()->CalcForward() * -1.0f;
        moveSpeed_ = maxMoveSpeed_;
    }

    // 更新
    void HitFrontState::Update(const float& elapsedTime)
    {
        // ノックバック処理
        owner_->GetTransform()->AddPosition(knockBackDirection_ * moveSpeed_ * elapsedTime);
        moveSpeed_ -= elapsedTime;
    }

    // 終了化
    void HitFrontState::Finalize()
    {
    }

    // ImGui
    void HitFrontState::DrawDebug()
    {
        if (ImGui::TreeNodeEx(GetName(), ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat("MaxMoveSpeed", &maxMoveSpeed_, 0.1f);
            ImGui::DragFloat("MoveSpeed", &moveSpeed_, 0.1f);

            ImGui::TreePop();
        }
    }

    // アニメーション再生
    void HitFrontState::PlayAnimation()
    {
        owner_->PlayAnimationBlend(WoodMonster::Animation::HitFront, false);
    }
}