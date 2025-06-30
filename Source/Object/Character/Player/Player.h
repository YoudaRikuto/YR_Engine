#pragma once
#include "Object/Character/Character.h"
#include "Ai/StateMachine.h"
#include "Collision/CollisionData.h"

class Player : public Character
{
public:
    enum class STATE
    {
        Idle,
        Run,
        Roll,
        JumpStart,
        JumpLoop,
        JumpEnd,
        JumpEndToRun,
        DoubleJump,
        Attack1_1,
        Attack1_2,
        Attack1_3,
        Attack1_4,
        AttackAir1_1,
        AttackAir1_2,
        AttackAir1_3,
        AttackAir1_4,
        AttackAirToFloor,
        FinisherAttack0,
        FinisherAttack1,
        FinisherAttack2,
    };

    enum class Animation
    {
        Idle,
        Run,
        RollForward,
        RollBack,
        AirDodgeForward,
        AirDodgeBack,
        JumpStart,
        JumpStartForward,
        JumpLoop,
        JumpEnd,
        JumpEndRun,
        DoubleJump,
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
        AttackAir1_1,
        AttackAir1_2,
        AttackAir1_3,
        AttackAir1_4,
        AttackAirToFloor_Start,
        AttackAirToFloor_Loop,
        AttackAirToFloor_End,
        Execution_1,
        Execution_2,
        Execution_3,
        Target_1,
        Target_2,
        Target_3,
    };

    enum class WeaponDataType
    {
        Default,
        AttackAirToFloor,
        Max
    };    

public:
    Player();
    ~Player() override {}

    void Initialize();
    void Finalize();
    void Update(const float& elapsedTime)       override;
    void Render(ID3D11PixelShader* psShader)    override;
    void DrawDebug()                            override;
    void DebugRender(DebugRenderer* debugRenderer);

public:
    void ResetFlags(); // Flagリセットする

    // ---------- Animation ----------
    void PlayAnimation(const Animation& index, const bool& loop, const float& speed = 1.0f, const float& startFrame = 0.0f) { Object::PlayAnimation(static_cast<int>(index), loop, speed, startFrame); }
    void PlayAnimationBlend(const Animation& index, const bool& loop, const float& speed = 1.0f, const float& blendStartFrame = 0.0f, const float& transitionTime = 0.1f) { Object::PlayAnimationBlend(static_cast<int>(index), loop, speed, blendStartFrame, transitionTime); }

    const Player::Animation GetAnimationIndex() const { return static_cast<Player::Animation>(Object::GetAnimationIndex()); }

    // ---------- StateMachine ----------
    void ChangeState(const STATE& state);
    const STATE GetCurrentState()   const { return currentState_; }
    const STATE GetOldState()       const { return oldState_; }
    const STATE GetNextState()       const { return nextState_; }
    void SetNextState(const STATE next) { this->nextState_ = next; }

    // ---------- Move ----------
    void SetMoveDirection(const DirectX::XMFLOAT3& direction) { moveDirection_ = direction; }

    // ---------- Turn ----------
    void Turn(const float& elapsedTime); // 旋回処理

    // ---------- Weapon ----------
    void ChangeWeaponDataType(const WeaponDataType& type);
    const int GetCurrentWeaponDataType() const { return currentWeaponDataType_; }

private:
    // ---------- StateMachine ----------
    void RegisterStateMachine(); // ステートマシン登録

    // ---------- Move ----------
    void Move(const float& elapsedTime); // 移動処理

    // ---------- Weapon ----------
    void UpdateWeaponTransform(const float& elapsedTime);
    
private:
    // ---------- StateMachine ----------
    std::unique_ptr<StateMachine<State<Player>>> stateMachine_;
    STATE currentState_ = STATE::Idle;
    STATE oldState_     = STATE::Idle;
    STATE nextState_ = STATE::Attack1_1;   //先行入力ステートを保持する変数

    // ---------- Move ----------
    DirectX::XMFLOAT3   moveDirection_  = {};
    float               gravity_        = 25.0f;

    // ---------- Weapon ----------
    struct WeaponTransform
    {
        DirectX::XMFLOAT3   location_   = { -13.0f, -4.0f, 7.0f };
        DirectX::XMFLOAT3   rotation_   = {};
        DirectX::XMFLOAT3   scale_      = { 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4X4 world_      = {};
    }swordTransform_;
    Object sword_;

    struct WeaponData
    {
        WeaponTransform weaponTransform_;
        Transform3D     transform_;
    };
    WeaponData weaponData_[static_cast<int>(WeaponDataType::Max)];

    int     currentWeaponDataType_      = static_cast<int>(WeaponDataType::Default);
    int     oldWeaponDataType_          = static_cast<int>(WeaponDataType::Default);
    float   weaponDataWeight_           = 0.0f;
    float   weaponDataTypeChangeSpeed_  = 5.0f;
    bool    isChangeWeaponData_         = false;
};

