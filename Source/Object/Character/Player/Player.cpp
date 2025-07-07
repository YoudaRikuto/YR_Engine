#include "Player.h"
#include "PlayerState.h"
#include "Input/Input.h"
#include "Graphics/Camera/Camera.h"

Player::Player()
    : Character("./Resources/Model/Player/Player.gltf", 1.0f, "Player"),
    sword_("./Resources/Model/Sword/NodachiSword.gltf", 1.0f, "Sword")
{
    PlayAnimation(Animation::Idle, true);

    // ステートマシン登録
    RegisterStateMachine();

    // Collision登録
    RegisterCollisionData();
}

// 初期化 
void Player::Initialize()
{
    SetAcceleration(50.0f);
    SetDeceleration(30.0f);
    SetMaxSpeed(7.0f);

    SetRotationSpeed(10.0f);

    // ---------- Weapon ----------
    weaponData_[static_cast<int>(WeaponDataType::Default)].weaponTransform_.location_   = { -13.0f, -4.0f, 7.0f };
    weaponData_[static_cast<int>(WeaponDataType::Default)].weaponTransform_.rotation_   = {};
    weaponData_[static_cast<int>(WeaponDataType::Default)].weaponTransform_.scale_      = { 1.0f, 1.0f, 1.0f };
    weaponData_[static_cast<int>(WeaponDataType::Default)].transform_.SetRotationDegree(40.0f, -40.0f, 180.0f);

    weaponData_[static_cast<int>(WeaponDataType::AttackAirToFloor)].weaponTransform_.location_  = { 0.0f, 4.0f, -5.0f };
    weaponData_[static_cast<int>(WeaponDataType::AttackAirToFloor)].weaponTransform_.rotation_  = {};
    weaponData_[static_cast<int>(WeaponDataType::AttackAirToFloor)].weaponTransform_.scale_     = { 1.0f, 1.0f, 1.0f };
    weaponData_[static_cast<int>(WeaponDataType::AttackAirToFloor)].transform_.SetRotationDegree(-40.0f, 90.0f, 0.0f);

    sword_.GetTransform()->SetRotationDegree(40.0f, -40.0f, 180.0f);
}

// 終了化 
void Player::Finalize()
{
    // ステートマシン終了化
    stateMachine_->GetCurrentState()->Finalize();

    Object::Finalize();
}

// 更新 
void Player::Update(const float& elapsedTime)
{
    // ステートマシン更新
    stateMachine_->Update(elapsedTime);

    // 移動処理
    Move(elapsedTime);

    Character::Update(elapsedTime);

    // Collision 更新
    UpdateCollisions(elapsedTime);

    // 武器の座標更新
    UpdateWeaponTransform(elapsedTime);
}

// 描画 
void Player::Render(ID3D11PixelShader* psShader)
{
    Object::Render(psShader);

    sword_.Render(swordTransform_.world_, psShader);
}

// ImGui 
void Player::DrawDebug()
{
    ImGui::Begin("Player");

    stateMachine_->DrawDebug();

    Character::DrawDebug();

    ImGui::DragFloat("Gravity", &gravity_, 0.1f);

    ImGui::End();

    ImGui::Begin("Sword");

    ImGui::DragFloat("ChangeSpeed", &weaponDataTypeChangeSpeed_, 0.01f);

    sword_.DrawDebug();

    if (ImGui::TreeNodeEx("Transform"))
    {
        ImGui::DragFloat3("Location", &swordTransform_.location_.x);
        ImGui::DragFloat3("Rotation", &swordTransform_.rotation_.x);
        ImGui::DragFloat3("Scale", &swordTransform_.scale_.x);

        ImGui::TreePop();
    }

    ImGui::End();
}

// デバッグ描画 
void Player::DebugRender(DebugRenderer* debugRenderer)
{
    Object::DebugRender(debugRenderer);
}

// フラグをリセットする
void Player::ResetFlags()
{
    // 先行入力フラグをリセットする
    nextState_ = STATE::Idle;
}

// ステートマシン登録 
void Player::RegisterStateMachine()
{
    stateMachine_.reset(new StateMachine<State<Player>>);

    // ステート登録
    stateMachine_->RegisterState(new PlayerState::IdleState(this));
    stateMachine_->RegisterState(new PlayerState::RunState(this));
    stateMachine_->RegisterState(new PlayerState::RollState(this));
    stateMachine_->RegisterState(new PlayerState::JumpStartState(this));
    stateMachine_->RegisterState(new PlayerState::JumpLoopState(this));
    stateMachine_->RegisterState(new PlayerState::JumpEndState(this));
    stateMachine_->RegisterState(new PlayerState::JumpEndToRunState(this));
    stateMachine_->RegisterState(new PlayerState::DoubleJumpState(this));
    stateMachine_->RegisterState(new PlayerState::AttackUpAirState(this));
    stateMachine_->RegisterState(new PlayerState::Attack1_1State(this));
    stateMachine_->RegisterState(new PlayerState::Attack1_2State(this));
    stateMachine_->RegisterState(new PlayerState::Attack1_3State(this));
    stateMachine_->RegisterState(new PlayerState::Attack1_4State(this));
    stateMachine_->RegisterState(new PlayerState::AttackAir1_1State(this));
    stateMachine_->RegisterState(new PlayerState::AttackAir1_2State(this));
    stateMachine_->RegisterState(new PlayerState::AttackAir1_3State(this));
    stateMachine_->RegisterState(new PlayerState::AttackAir1_4State(this));
    stateMachine_->RegisterState(new PlayerState::AttackAirToFloorState(this));
    stateMachine_->RegisterState(new PlayerState::AttackAirToFloorEndState(this));
    stateMachine_->RegisterState(new PlayerState::FinisherAttack0State(this));
    stateMachine_->RegisterState(new PlayerState::FinisherAttack1State(this));
    stateMachine_->RegisterState(new PlayerState::FinisherAttack2State(this));
    stateMachine_->RegisterState(new PlayerState::BlockState(this));
    stateMachine_->RegisterState(new PlayerState::BlockEndState(this));
    stateMachine_->RegisterState(new PlayerState::ParryState(this));

    // 1番最初のステート設定
    stateMachine_->SetState(static_cast<int>(STATE::Idle));
    currentState_ = STATE::Idle;
}

// ステート変更 
void Player::ChangeState(const STATE& state)
{
    oldState_       = currentState_;
    currentState_   = state;
    stateMachine_->ChangeState(static_cast<int>(state));
}

// 移動処理 
void Player::Move(const float& elapsedTime)
{
    const Player::Animation animationIndex = static_cast<Player::Animation>(GetAnimationIndex());

    if (animationIndex == Player::Animation::RollForward ||
        animationIndex == Player::Animation::RollBack ||        
        animationIndex == Player::Animation::AttackAir1_1 ||
        animationIndex == Player::Animation::AttackAir1_2 ||
        animationIndex == Player::Animation::AttackAir1_3 ||
        animationIndex == Player::Animation::AttackAir1_4)
    {
        return;
    }

    DirectX::XMFLOAT3   velocity    = GetVelocity();
    DirectX::XMFLOAT2   velocityXZ  = { velocity.x, velocity.z };   // XZ平面用
    float               velocityY   = velocity.y;                   // Y軸用

    float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);

    // 移動入力無。減速処理
    if (fabs(moveDirection_.x) + fabsf(moveDirection_.z) <= 0.001f && length != 0.0f)
    {
        const float deceleration = length - GetDeceleration() * elapsedTime;
        if (deceleration < 0.0f)
        {
            SetVelocity({ 0.0f, velocityY, 0.0f });
            return;
        }

        velocityXZ = XMFloat2Normalize(velocityXZ) * deceleration;
    }
    // 移動入力有。加速処理
    else
    {
        const float acceleration    = GetAcceleration() * elapsedTime;
        const float maxSpeed        = GetMaxSpeed();

        velocityXZ.x += moveDirection_.x * acceleration;
        velocityXZ.y += moveDirection_.z * acceleration;

        length = sqrtf(velocityXZ.x * velocityXZ.x + velocityXZ.y * velocityXZ.y);

        if (length > maxSpeed)
        {
            velocityXZ = XMFloat2Normalize(velocityXZ) * maxSpeed;
        }
    }

    velocityY -= gravity_ * elapsedTime;

    velocity = { velocityXZ.x, velocityY, velocityXZ.y };

    GetTransform()->AddPosition(velocity * elapsedTime);

    if (GetTransform()->GetPositionY() <= 0.0f)
    {
        GetTransform()->SetPositionY(0.0f);
        velocity.y = 0.0f;
    }

    SetVelocity(velocity);
}

// 武器の座標更新 
void Player::UpdateWeaponTransform(const float& elapsedTime)
{
    // Transform補間
    if (isChangeWeaponData_)
    {
        weaponDataWeight_ += weaponDataTypeChangeSpeed_ * elapsedTime;
        weaponDataWeight_ = std::min(weaponDataWeight_, 1.0f);

        swordTransform_.location_ = XMFloat3Lerp(weaponData_[oldWeaponDataType_].weaponTransform_.location_,
            weaponData_[currentWeaponDataType_].weaponTransform_.location_, weaponDataWeight_);

        sword_.GetTransform()->SetRotation(XMFloat3Lerp(weaponData_[oldWeaponDataType_].transform_.GetRotation(),
            weaponData_[currentWeaponDataType_].transform_.GetRotation(), weaponDataWeight_));

        if (weaponDataWeight_ == 1.0f)
        {
            isChangeWeaponData_ = false;
        }
    }

    const float toRadian = 0.01745f;
    const float toMetric = 0.01f;
    const int weponJointIndex = GetNodeIndex("hand_r");
    const GltfModel::Node node = GetNodes()->at(weponJointIndex);
    const DirectX::XMMATRIX boneTransform = DirectX::XMLoadFloat4x4(&node.globalTransform_);

    DirectX::XMMATRIX transform = {};

    transform = DirectX::XMMatrixScaling(swordTransform_.scale_.x, swordTransform_.scale_.y, swordTransform_.scale_.z)
        * DirectX::XMMatrixRotationX(-swordTransform_.rotation_.x * toRadian)
        * DirectX::XMMatrixRotationX(-swordTransform_.rotation_.y * toRadian)
        * DirectX::XMMatrixRotationX(swordTransform_.rotation_.z * toRadian)
        * DirectX::XMMatrixTranslation(swordTransform_.location_.x * toMetric, swordTransform_.location_.y * toMetric, swordTransform_.location_.z * toMetric);
    
    DirectX::XMMATRIX dxUE5 = DirectX::XMMatrixSet(-1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1); // LHS Y-Up Z-Forward(DX) -> LHS Z-Up Y-Forward(UE5) 
    DirectX::XMMATRIX UE5Gltf = DirectX::XMMatrixSet(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1); // LHS Z-Up Y-Forward(UE5) -> RHS Y-Up Z-Forward(glTF) 

    DirectX::XMMATRIX W = sword_.GetTransform()->CalcWorldMatrix(sword_.GetScaleFactor());

    DirectX::XMStoreFloat4x4(&swordTransform_.world_, W * dxUE5 * transform * UE5Gltf * boneTransform * GetTransform()->CalcWorldMatrix(GetScaleFactor()));
}

// 武器の姿勢の種類を変更
void Player::ChangeWeaponDataType(const WeaponDataType& type)
{
    oldWeaponDataType_      = currentWeaponDataType_;
    currentWeaponDataType_  = static_cast<int>(type);
    weaponDataWeight_       = 0.0f;
    isChangeWeaponData_     = true;
}

// 旋回処理 
void Player::Turn(const float& elapsedTime)
{
    const float aLx = Input::Instance().GetGamePad().GetAxisLx();
    const float aLy = Input::Instance().GetGamePad().GetAxisLy();

    if (GetCurrentState() == STATE::JumpStart || GetCurrentState() == STATE::JumpLoop ||
        GetCurrentState() == STATE::DoubleJump)
    {
        if (fabsf(aLx) == 0.0f && fabsf(aLy) == 0.0f)
        {
            moveDirection_ = {};

            DirectX::XMFLOAT3 velocity = GetVelocity();
            velocity = { 0.0f, velocity.y, 0.0f };
            SetVelocity(velocity);

            return;
        }
    }

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

void Player::AttackTurn()
{
    const float aLx = Input::Instance().GetGamePad().GetAxisLx();
    const float aLy = Input::Instance().GetGamePad().GetAxisLy();

    if (fabsf(aLx) > 0.0f || fabsf(aLy) > 0.0f)
    {
        DirectX::XMFLOAT2 cameraForward = XMFloat2Normalize(Camera::Instance().ConvertTo2DVectorFromCamera({ aLx, aLy }));
        DirectX::XMFLOAT2 playerForward = XMFloat2Normalize({ GetTransform()->CalcForward().x, GetTransform()->CalcForward().z });

        // 外積で回転方向を判定
        const float cross = XMFloat2Cross(cameraForward, playerForward);

        // 内積で回転幅を算出
        const float dot = std::clamp(XMFloat2Dot(cameraForward, playerForward), -1.0f, 1.0f);
        float angle = acosf(dot);

        if (angle < DirectX::XMConvertToRadians(1)) return;

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