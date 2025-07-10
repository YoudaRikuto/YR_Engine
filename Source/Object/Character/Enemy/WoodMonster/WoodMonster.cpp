#include "WoodMonster.h"
#include "ImGui/ImGuiCtrl.h"
#include "WoodMonsterState.h"
#include "Object/Character/Player/PlayerManager.h"

WoodMonster::WoodMonster()
    : Enemy("./Resources/Model/Enemy/WoodMonster/WoodMonster.gltf", 1.0f, "WoodMonster")
{
    PlayAnimation(Animation::Idle, true);

    // ステートマシン登録
    RegisterStateMachine();

    // Collision登録
    RegisterCollisionData();
}

// 初期化
void WoodMonster::Initialize()
{
    // サイズを設定
    GetTransform()->SetScale(1.2f, 1.0f, 1.5f);
    GetTransform()->SetScaleFactor(1.5f);

    SetRotationSpeed(10.0f);
}

// 終了化
void WoodMonster::Finalize()
{
    stateMachine_->GetCurrentState()->Finalize();

    Object::Finalize();
}

// 更新
void WoodMonster::Update(const float& elapsedTime)
{
    // ステートマシン更新
    stateMachine_->Update(elapsedTime);

    Character::Update(elapsedTime);

    // Collision更新
    UpdateCollisions(elapsedTime);
}

// 描画
void WoodMonster::Render(ID3D11PixelShader* psShader)
{
    Object::Render(psShader);
}

// ImGui用
void WoodMonster::DrawDebug()
{
    ImGui::Begin("WoodMonster");

    if (ImGui::Button("Attack State")) ChangeState(STATE::Attack);

    stateMachine_->DrawDebug();

    Character::DrawDebug();

    ImGui::End();
}

// デバッグ描画 
void WoodMonster::DebugRender(DebugRenderer* debugRenderer)
{
    Object::DebugRender(debugRenderer);
}

// ダメージをくらった
void WoodMonster::OnDamage()
{
    const Player::STATE playerState = PlayerManager::Instance().GetPlayer()->GetCurrentState();
    const WoodMonster::Animation woodMonsterAnimation = GetAnimationIndex();

    // Playerが 斬り上げ攻撃
    if (playerState == Player::STATE::AttackUpAir)
    {
        ChangeState(STATE::HitToAir);
    }

    if (woodMonsterAnimation == Animation::HitAir1)
    {
        ChangeState(STATE::HitAir2);
    }
    else if (woodMonsterAnimation == Animation::HitAir2)
    {
        ChangeState(STATE::HitAir3);
    }
    else if (woodMonsterAnimation == Animation::HitAir3)
    {
        ChangeState(STATE::HitAir1);
    }
}

// Targetに向かって旋回する
void WoodMonster::Turn(const float& elapsedTime, const DirectX::XMFLOAT3& target)
{
    const DirectX::XMFLOAT3 woodMonsterPosition = GetTransform()->GetPosition();
    const DirectX::XMFLOAT2 vec = XMFloat2Normalize({ target.x - woodMonsterPosition.x, target.z - woodMonsterPosition.z });
    const DirectX::XMFLOAT2 woodMonsterForward = XMFloat2Normalize({ GetTransform()->CalcForward().x, GetTransform()->CalcForward().z });

    const float cross = XMFloat2Cross(vec, woodMonsterForward);
    const float dot = std::clamp(XMFloat2Dot(vec, woodMonsterForward), -1.0f, 1.0f);
    const float angle = acosf(dot) * GetRotationSpeed() * elapsedTime;

    if (angle < DirectX::XMConvertToRadians(1)) return;

    if (cross > 0) GetTransform()->AddRotationY(-angle);
    else GetTransform()->AddRotationY(angle);
}

// プレイヤーの方向に旋回する
void WoodMonster::TurnToPlayer()
{
    const DirectX::XMFLOAT3 playerPosition = PlayerManager::Instance().GetTransform()->GetPosition();
    const DirectX::XMFLOAT3 woodMonsterPosition = GetTransform()->GetPosition();

    const DirectX::XMFLOAT2 vec = XMFloat2Normalize({ playerPosition.x - woodMonsterPosition.x, playerPosition.z - woodMonsterPosition.z });
    const DirectX::XMFLOAT2 woodMonsterForward = XMFloat2Normalize({ GetTransform()->CalcForward().x, GetTransform()->CalcForward().z });

    const float cross = XMFloat2Cross(vec, woodMonsterForward);
    const float dot = std::clamp(XMFloat2Dot(vec, woodMonsterForward), -1.0f, 1.0f);
    const float angle = acosf(dot);

    if (angle < DirectX::XMConvertToRadians(1)) return;

    if (cross > 0) GetTransform()->AddRotationY(-angle);
    else GetTransform()->AddRotationY(angle);
}

// ステートマシン登録
void WoodMonster::RegisterStateMachine()
{
    stateMachine_.reset(new StateMachine<State<WoodMonster>>);

    // ステート登録
    stateMachine_->RegisterState(new WoodMonsterState::IdleState(this));
    stateMachine_->RegisterState(new WoodMonsterState::AttackState(this));
    stateMachine_->RegisterState(new WoodMonsterState::Attack3_1State(this));
    stateMachine_->RegisterState(new WoodMonsterState::Attack3_2State(this));
    stateMachine_->RegisterState(new WoodMonsterState::Attack3_3State(this));
    stateMachine_->RegisterState(new WoodMonsterState::HitToAirState(this));
    stateMachine_->RegisterState(new WoodMonsterState::HitAirIdleState(this));
    stateMachine_->RegisterState(new WoodMonsterState::HitAir1State(this));
    stateMachine_->RegisterState(new WoodMonsterState::HitAir2State(this));
    stateMachine_->RegisterState(new WoodMonsterState::HitAir3State(this));
    stateMachine_->RegisterState(new WoodMonsterState::BlockHitBreakState(this));
    stateMachine_->RegisterState(new WoodMonsterState::FinisherTarget0State(this));

    // 1番最初のステート設定
    stateMachine_->SetState(static_cast<int>(STATE::Idle));
    currentState_ = STATE::Idle;
}

// ステート変更
void WoodMonster::ChangeState(const STATE& state)
{
    oldState_ = currentState_;
    currentState_ = state;
    stateMachine_->ChangeState(static_cast<int>(state));
}