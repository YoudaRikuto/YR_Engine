#pragma once
#include "Ai/State.h"
#include "Player.h"

namespace PlayerState
{
    class IdleState : public State<Player>
    {
    public:
        IdleState(Player* player) : State(player, "IdleState") {}
        ~IdleState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

    class RunState : public State<Player>
    {
    public:
        RunState(Player* player) : State(player, "RunState") {}
        ~RunState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ------------ Animation ------------
        float animationSpeed_ = 0.9f;

        float transitionJumpEnd_                = 0.1f;
        float transitionAttackAirToFloorEnd_    = 0.3f;
    };

    class RollState : public State<Player>
    {
    public:
        RollState(Player* player) : State(player, "RollState") {}
        ~RollState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationSpeed_ = 1.5f;
        
        DirectX::XMFLOAT3 rootMotionValue_ = { 1.2f, 1.0f, 1.2f };

        // ---------- Transition Frame ----------
        float runTransitionFrame_ = 0.9f;
    };

    class JumpStartState : public State<Player>
    {
    public:
        JumpStartState(Player* player) : State(player, "JumpStartState") {}
        ~JumpStartState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ---------- Animation ----------
        float   animationStartFrame_    = 0.13f;
        float   transitionIdle_         = 0.1f;
        float   transitionRun_          = 0.1f;

        float   attackUpAirTransitionEndFrame_      = 0.3f;
        float   attackAir1_1TransitionFrame_        = 0.3f;
        float   attackAirToFloorTransitionFrame_    = 0.3f;

        float   jumpPower_  = 10.0f;

        float   jumpFrame_  = 0.2f;
        bool    isJumped_   = false;
    };

    class JumpLoopState : public State<Player>
    {
    public:
        JumpLoopState(Player* player) : State(player, "JumpLoopState") {}
        ~JumpLoopState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ---------- Animation ----------
        float transitionAttackAir1_ = 0.2f;

        float landingTriggerPositionY_ = 0.05f;

        bool isDoubleJumpEnabled_ = true; // ダブルジャンプが可能か
    };

    class JumpEndState : public State<Player>
    {
    public:
        JumpEndState(Player* player) : State(player, "JumpEndState") {}
        ~JumpEndState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationStartFrame_  = 0.18f;
        float transitionTime_       = 0.1f;

        float runTransitionFrame_ = 0.25f;
        //float attackAir1TransitionFrame_ = 0.3f;
        
    };

    class JumpEndToRunState : public State<Player>
    {
    public:
        JumpEndToRunState(Player* player) : State(player, "JumpEndToRunState") {}
        ~JumpEndToRunState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationStartFrame_  = 0.15f;
        float animationEndFrame_    = 0.2f;
        float transitionJumpLoop_   = 0.1f;
        
    };

    class DoubleJumpState : public State<Player>
    {
    public:
        DoubleJumpState(Player* player) : State(player, "DoubleJumpState") {}
        ~DoubleJumpState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ---------- Animation ----------
        float animationSpeed_ = 1.0f;

        float animationStartFrame_  = 0.26f;
        float animationEndFrame_    = 0.8f;

        float jumpPower_ = 13.0f;
    };

    class AttackUpAirState : public State<Player>
    {
    public:
        AttackUpAirState(Player* player) : State(player, "AttackUpAirState") {}
        ~AttackUpAirState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation(); // アニメーション再生

    private:
        float               animationStartFrame_    = 0.2f;
        DirectX::XMFLOAT3   rootMotionValue_        = { 0.5f, 2.0f, 0.5f };

        // 遷移フレーム
        float attackAir1_1TransitionFrame_ = 0.8f;
    };

    class Attack1_1State : public State<Player>
    {
    public:
        Attack1_1State(Player* player) : State(player, "Attack1_1State") {}
        ~Attack1_1State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationSpeed_ = 1.0f;
        float animationStartFrame_ = 0.16f;
        float transitionTime_ = 0.1f;
        float runTransitionFrame_ = 0.6f;
        float attack2TransitionFrame_ = 0.5f;
        
    };

    class Attack1_2State : public State<Player>
    {
    public:
        Attack1_2State(Player* player) : State(player, "Attack1_2State") {}
        ~Attack1_2State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationSpeed_ = 1.0f;
        float animationStartFrame_ = 0.08f;
        float transitionTime_ = 0.1f;
        float runTransitionFrame_ = 0.4f;
        float attack3TransitionFrame_ = 0.3f;
    };

    class Attack1_3State : public State<Player>
    {
    public:
        Attack1_3State(Player* player) : State(player, "Attack1_3State") {}
        ~Attack1_3State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationSpeed_ = 1.0f;
        float animationStartFrame_ = 0.08f;
        float transitionTime_ = 0.1f;
        float runTransitionFrame_ = 0.4f;
        float attack4TransitionFrame_ = 0.3f;
    };

    class Attack1_4State : public State<Player>
    {
    public:
        Attack1_4State(Player* player) : State(player, "Attack1_4State") {}
        ~Attack1_4State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationSpeed_ = 1.0f;
        float animationStartFrame_ = 0.1f;
        float transitionTime_ = 0.1f;
        float runTransitionFrame_ = 0.6f;
        float attack1TransitionFrame_ = 0.5f;
    };

    class AttackAir1_1State : public State<Player>
    {
    public:
        AttackAir1_1State(Player* player) : State(player, "AttackAir1_1State") {}
        ~AttackAir1_1State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ---------- Animation ---------- 
        float animationSpeed_ = 1.2f;
        float animationStartFrame_ = 0.14f;
        float transitionTime_ = 0.1f;

        DirectX::XMFLOAT3 rootMotionValue_ = { 1.0f, 1.0f, 1.0f };

        float jumpLoopTransitionFrame_ = 0.6f;

        float attackAir2TransitionFrame_ = 0.37f;
    };

    class AttackAir1_2State : public State<Player>
    {
    public:
        AttackAir1_2State(Player* player) : State(player, "AttackAir1_2State") {}
        ~AttackAir1_2State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ---------- Animation ----------
        float animationSpeed_ = 1.2f;
        float animationStartFrame_ = 0.0f;
        float transitionTime_ = 0.1f;

        float attackAir3TransitionFrame_ = 0.6f;

        float jumpLoopTransitionFrame_ = 0.9f;
    };

    class AttackAir1_3State : public State<Player>
    {
    public:
        AttackAir1_3State(Player* player) : State(player, "AttackAir1_3State") {}
        ~AttackAir1_3State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        // ---------- Animation ----------
        float animationSpeed_ = 1.2f;
        float animationStartFrame_ = 0.0f;
        float transitionTime_ = 0.1f;

        float attackAir4TransitionFrame_ = 0.77f;
        float jumpLoopTransitionFrame_ = 1.3f;
    };

    class AttackAir1_4State : public State<Player>
    {
    public:
        AttackAir1_4State(Player* player) : State(player, "AttackAir1_4State") {}
        ~AttackAir1_4State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
       
    private:
        // ---------- Animation ----------
        float animationSpeed_ = 1.1f;
        float animationStartFrame_ = 0.0f;
        float transitionTime_ = 0.1f;
        float jumpLoopTransitionFrame_ = 1.3f;
    };

    class AttackAirToFloorState : public State<Player>
    {
    public:
        AttackAirToFloorState(Player* player) : State(player, "AttackAirToFloorState") {}
        ~AttackAirToFloorState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation(); // アニメーション再生

        void UpdateFallingSpeed(); // 落下速度更新

    private:
        // ---------- Animation ----------
        float startAnimationSpeed_ = 0.9f;

        float landingTriggerPositionY_ = 0.05f;

        float fallingSpeed_ = -40.0f;
    };

    class AttackAirToFloorEndState : public State<Player>
    {
    public:
        AttackAirToFloorEndState(Player* player) : State(player, "AttackAirToFloorEndState") {}
        ~AttackAirToFloorEndState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation(); // アニメーション再生

    private:
        // ---------- Animation ----------
        float startAnimationSpeed_ = 0.9f;

        float runTransitionFrame_ = 0.2f;

        float fallingSpeed_ = -40.0f;
    };

    class FinisherAttack0State : public State<Player>
    {
    public:
        FinisherAttack0State(Player* player) : State(player, "FinisherAttack0State") {}
        ~FinisherAttack0State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

    class FinisherAttack1State : public State<Player>
    {
    public:
        FinisherAttack1State(Player* player) : State(player, "FinisherAttack1State") {}
        ~FinisherAttack1State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

    class FinisherAttack2State : public State<Player>
    {
    public:
        FinisherAttack2State(Player* player) : State(player, "FinisherAttack2State") {}
        ~FinisherAttack2State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

    class BlockState : public State<Player>
    {
    public:
        BlockState(Player* player) : State(player, "BlockState") {}
        ~BlockState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation(); // アニメーション再生

    private:
        float animationStartFrame_ = 0.35f;

        float transitionIdle_ = 0.1f;

        bool isChangeState_ = false;
    };

    class BlockEndState : public State<Player>
    {
    public:
        BlockEndState(Player* player) : State(player, "BlockEndState") {}
        ~BlockEndState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation(); // アニメーション再生

    private:
        float runTransitionFrame_ = 0.18f;

    };

    class ParryState : public State<Player>
    {
    public:
        ParryState(Player* player) : State(player, "ParryState") {}
        ~ParryState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation(); // アニメーション再生

    private:
        float parryEffectSize_ = 0.3f;
    };

    class SkillAttackState : public State<Player>
    {
    public:
        SkillAttackState(Player* player) : State(player, "SkillAttackState") {}
        ~SkillAttackState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };
}
