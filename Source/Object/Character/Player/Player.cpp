#include "Player.h"
#include "PlayerState.h"
#include "Input/Input.h"
#include "Graphics/Camera/Camera.h"

// ----- コンストラクタ -----
Player::Player()
    : Character("./Resources/Model/Player/Manny.gltf", 1.0f),
    sword_("./Resources/Model/Sword/Sword.gltf", 1.0f)
{
    PlayAnimation(Animation::Idle, true);

    // ステートマシン登録
    RegisterStateMachine();
}

// ----- 初期化 -----
void Player::Initialize()
{
    SetAcceleration(50.0f);
    SetDeceleration(30.0f);
    SetMaxSpeed(6.0f);

    SetRotationSpeed(10.0f);

    
}

// ----- 終了化 -----
void Player::Finalize()
{
    // ステートマシン終了化
    stateMachine_->GetCurrentState()->Finalize();
}

// ----- 更新 -----
void Player::Update(const float& elapsedTime)
{
    // ステートマシン更新
    stateMachine_->Update(elapsedTime);

    // 移動処理
    Move(elapsedTime);

    Character::Update(elapsedTime);
}

// ----- 描画 -----
void Player::Render(ID3D11PixelShader* psShader)
{
    Object::Render(psShader);

    sword_.Render(psShader);
}

// ----- ImGui -----
void Player::DrawDebug()
{
    ImGui::Begin("Player");

    stateMachine_->DrawDebug();

    Character::DrawDebug();

    ImGui::End();

    ImGui::Begin("Sword");

    if (ImGui::TreeNodeEx("Transform"))
    {
        ImGui::DragFloat3("Location", &swordTransform_.location_.x);
        ImGui::DragFloat3("Rotation", &swordTransform_.rotation_.x);
        ImGui::DragFloat3("Scale", &swordTransform_.scale_.x);

        ImGui::TreePop();
    }

    ImGui::End();
}

// ----- デバッグ描画 -----
void Player::DebugRender()
{
}

// ----- ステートマシン登録 -----
void Player::RegisterStateMachine()
{
    stateMachine_.reset(new StateMachine<State<Player>>);

    // ステート登録
    stateMachine_->RegisterState(new PlayerState::IdleState(this));
    stateMachine_->RegisterState(new PlayerState::RunState(this));
    stateMachine_->RegisterState(new PlayerState::RollState(this));
    stateMachine_->RegisterState(new PlayerState::JumpState(this));
    stateMachine_->RegisterState(new PlayerState::DoubleJumpState(this));

    // 1番最初のステート設定
    stateMachine_->SetState(static_cast<int>(STATE::Idle));
    currentState_ = STATE::Idle;
}

// ----- ステート変更 -----
void Player::ChangeState(const STATE& state)
{
    oldState_       = currentState_;
    currentState_   = state;
    stateMachine_->ChangeState(static_cast<int>(state));
}

// ----- 移動処理 -----
void Player::Move(const float& elapsedTime)
{
    DirectX::XMFLOAT3 velocity = GetVelocity();
    float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);

    // 移動入力無。減速処理
    if (fabs(moveDirection_.x) + fabsf(moveDirection_.z) <= 0.001f && length != 0.0f)
    {
        const float deceleration = length - GetDeceleration() * elapsedTime;
        if (deceleration < 0.0f)
        {
            SetVelocity({});
            return;
        }

        velocity = XMFloat3Normalize(velocity) * deceleration;
    }
    // 移動入力有。加速処理
    else
    {
        const float acceleration    = GetAcceleration() * elapsedTime;
        const float maxSpeed        = GetMaxSpeed();

        velocity.x += moveDirection_.x * acceleration;
        velocity.z += moveDirection_.z * acceleration;

        length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);

        if (length > maxSpeed)
        {
            velocity = XMFloat3Normalize(velocity) * maxSpeed;
        }
    }

    SetVelocity(velocity);
    GetTransform()->AddPosition(velocity * elapsedTime);
}

// ----- 旋回処理 -----
void Player::Turn(const float& elapsedTime)
{
    const float aLx = Input::Instance().GetGamePad().GetAxisLx();
    const float aLy = Input::Instance().GetGamePad().GetAxisLy();

    DirectX::XMFLOAT2 moveDirection = Camera::Instance().ConvertTo2DVectorFromCamera({ aLx, aLy });
    moveDirection_ = XMFloat3Normalize({ moveDirection.x, 0.0f, moveDirection.y });

    if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
    {
        DirectX::XMFLOAT2 cameraForward = XMFloat2Normalize({ moveDirection_.x, moveDirection_.z });
        DirectX::XMFLOAT2 playerForward = XMFloat2Normalize({ GetTransform()->CalcForward().x, GetTransform()->CalcForward().z });

        // 外積で回転方向を判定
        const float cross = XMFloat2Cross(cameraForward, playerForward);

        // 内積で回転幅を算出
        const float dot = std::clamp(XMFloat2Dot(cameraForward, playerForward), -1.0f, 1.0f);
        float angle = acosf(dot);

        if (angle < DirectX::XMConvertToRadians(1)) return;

        const float rotationSpeed = GetRotationSpeed() * elapsedTime;
        angle *= rotationSpeed;

        if (cross > 0)
        {
            GetTransform()->AddRotationY(-angle);
        }
        else
        {
            GetTransform()->AddRotationY(angle);
        }
    }
}
