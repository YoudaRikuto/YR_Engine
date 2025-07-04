#pragma once
#include "WoodMonster.h"

namespace WoodMonsterState
{
    class IdleState : public State<WoodMonster>
    {
    public:
        IdleState(WoodMonster* woodMonster) : State(woodMonster, "IdleState") {}
        ~IdleState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();

    private:
        float animationSpeed_ = 0.6f;
        float transitionAttack_ = 0.1f;

        float attackTransitionTime_ = 2.0f;
        float transitionTimer_ = 0.0f;
    };

    // TODO:後でクラス名変更する。とりあえずAttackにする
    class AttackState : public State<WoodMonster>
    {
    public:
        AttackState(WoodMonster* woodMonster) : State(woodMonster, "AttackState") {}
        ~AttackState() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
        void UpdateAnimationSpeed();

    private:
        float animationStartFrame_  = 0.12f;
        float animationEndFrame_    = 1.6f;

        float transitionIdle_ = 0.2f;

        float attackRecoveryStartFrame_ = 0.8f;
        float attackRecoveryEndFrame_   = 1.5f;
        float attackRecoverySpeed_      = 0.6f;

        DirectX::XMFLOAT3 rootMotionValue_ = { 1.0f, 1.0f, 1.0f };
    };

    class FinisherTarget0State : public State<WoodMonster>
    {
    public:
        FinisherTarget0State(WoodMonster* woodMonster) : State(woodMonster, "FinisherTarget0State") {}
        ~FinisherTarget0State() {}

        void Initialize()                       override;
        void Update(const float& elapsedTime)   override;
        void Finalize()                         override;
        void DrawDebug()                        override;

    private:
        void PlayAnimation();
    };

}

