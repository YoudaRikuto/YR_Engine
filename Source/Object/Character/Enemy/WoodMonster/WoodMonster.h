#pragma once
#include "Object/Character/Enemy/Enemy.h"
#include "Ai/StateMachine.h"

class WoodMonster : public Enemy
{
public:
    enum class STATE
    {
        Idle,
        Attack,

        HitToAir,
        HitAirIdle,
        HitAir1,
        HitAir2,
        HitAir3,
        
        FinisherTarget0,
    };

    enum class Animation
    {
        Idle,
        Walk,
        WalkCombat,
        DodgeFront,
        DodgeBack,
        Attack1_1,
        Attack1_2,
        Attack1_3,
        Attack1_4,
        Attack2_1,
        Attack2_2,
        Attack2_3,
        Attack2_4,
        Attack3_1,
        Attack3_2,
        Attack3_3,
        Attack3_4,
        Attack4_1,
        Attack4_2,
        Attack4_3,
        Attack4_4,
        Attack5_1,
        Attack5_2,
        Attack5_3,
        Attack5_4,
        AttackAirToFloorStart,
        AttackAirToFloorLoop,
        AttackAirToFloorEnd,
        Execution_1,
        Execution_2,
        Execution_3,
        Target_1,
        Target_2,
        Target_3,

        HitFront,
        HitBack,
        HitRight,
        HitLeft,
        HitAir1,
        HitAir2,
        HitAir3,
        KnockDownStart,
        KnockDownLoop,
        KnockDownGetUp,
        KnockDownDeath,

        IdleCombat,
    };

public:
    WoodMonster();
    ~WoodMonster() override {}
    const EnemyType GetEnemyType() const override { return EnemyType::WoodMonster; }

    void Initialize()                                   override;
    void Finalize()                                     override;
    void Update(const float& elapsedTime)               override;
    void Render(ID3D11PixelShader* psShader = nullptr)  override;
    void DrawDebug()                                    override;
    void DebugRender(DebugRenderer* debugRenderer)    override;

    // ---------- Animation ----------
    void PlayAnimation(const Animation& index, const bool& loop, const float& speed = 1.0f, const float& startFrame = 0.0f) { Object::PlayAnimation(static_cast<int>(index), loop, speed, startFrame); }
    void PlayAnimationBlend(const Animation& index, const bool& loop, const float& speed = 1.0f, const float& blendStartFrame = 0.0f, const float& transitionTime = 0.1f) { Object::PlayAnimationBlend(static_cast<int>(index), loop, speed, blendStartFrame, transitionTime); }
    const WoodMonster::Animation GetAnimationIndex() const { return static_cast<WoodMonster::Animation>(Object::GetAnimationIndex()); }

    // ---------- StateMachine ----------
    void ChangeState(const STATE& state);
    const STATE GetCurrentState() const { return currentState_; }
    const STATE GetOldState() const { return oldState_; }

    const bool IsAttackHitBoxActive() const { return isAttackHitBoxActive_; }
    void SetAttackHitBoxActive(const bool& flag) { isAttackHitBoxActive_ = flag; }


    void OnDamage();

    void Turn(const float& elapsedTime, const DirectX::XMFLOAT3& target);
    void TurnToPlayer();

private:
    void RegisterStateMachine(); // ステートマシン登録

private:
    // ---------- StateMachine ----------
    std::unique_ptr<StateMachine<State<WoodMonster>>> stateMachine_;
    STATE currentState_ = STATE::Idle;
    STATE oldState_     = STATE::Idle;

    bool isAttackHitBoxActive_ = false;
};

