#pragma once
#include "Object/Character/Character.h"
#include "Ai/StateMachine.h"

class Player : public Character
{
public:
    enum class STATE
    {
        Idle,
        Run,
        Roll,
        Jump,
        DoubleJump,
    };
    enum class Animation
    {
        Idle,
        Run,
        RollForward,
        RollBack,
        JumpStart,
        JumpLoop,
        JumpEnd,
        DoubleJump,
    };

public:
    Player();
    ~Player() override {}

    void Initialize();
    void Finalize();
    void Update(const float& elapsedTime)       override;
    void Render(ID3D11PixelShader* psShader)    override;
    void DrawDebug()                            override;
    void DebugRender();

public:
    // ---------- Animation ----------
    void PlayAnimation(const Animation& index, const bool& loop, const float& speed = 1.0f, const float& startFrame = 0.0f) { Object::PlayAnimation(static_cast<int>(index), loop, speed, startFrame); }
    void PlayAnimationBlend(const Animation& index, const bool& loop, const float& speed = 1.0f, const float& blendStartFrame = 0.0f, const float& transitionTime = 1.0f) { Object::PlayAnimationBlend(static_cast<int>(index), loop, speed, blendStartFrame, transitionTime); }

    // ---------- StateMachine ----------
    void ChangeState(const STATE& state);

    // ---------- Move ----------
    void SetMoveDirection(const DirectX::XMFLOAT3& direction) { moveDirection_ = direction; }

    // ---------- Turn ----------
    void Turn(const float& elapsedTime); // 旋回処理

private:
    // ---------- StateMachine ----------
    void RegisterStateMachine(); // ステートマシン登録

    // ---------- Move ----------
    void Move(const float& elapsedTime); // 移動処理

private:
    // ---------- Weapon ----------
    struct WeaponTransform
    {
        DirectX::XMFLOAT3   location_   = {};
        DirectX::XMFLOAT3   rotation_   = {};
        DirectX::XMFLOAT3   scale_      = {};
        DirectX::XMFLOAT4X4 world_      = {};
    }swordTransform_;
    Object sword_;

    // ---------- StateMachine ----------
    std::unique_ptr<StateMachine<State<Player>>> stateMachine_;
    STATE currentState_ = STATE::Idle;
    STATE oldState_     = STATE::Idle;

    // ---------- Move ----------
    DirectX::XMFLOAT3 moveDirection_ = {};
};

