#include "WoodMonster.h"
#include "ImGui/ImGuiCtrl.h"
#include "WoodMonsterState.h"

WoodMonster::WoodMonster()
    : Enemy("./Resources/Model/Enemy/WoodMonster/WoodMonster.gltf", 1.0f, "WoodMonster")
{
    PlayAnimation(Animation::Idle, true);

    // ステートマシン登録
    RegisterStateMachine();
}

// 初期化
void WoodMonster::Initialize()
{
    // サイズを設定
    GetTransform()->SetScale(1.2f, 1.0f, 1.5f);
    GetTransform()->SetScaleFactor(1.5f);
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

    stateMachine_->DrawDebug();

    Object::DrawDebug();

    ImGui::End();
}

// デバッグ描画 
void WoodMonster::DebugRender(DebugRenderer* debugRenderer)
{
    Object::DebugRender(debugRenderer);
}

// ステートマシン登録
void WoodMonster::RegisterStateMachine()
{
    stateMachine_.reset(new StateMachine<State<WoodMonster>>);

    // ステート登録
    stateMachine_->RegisterState(new WoodMonsterState::IdleState(this));
    stateMachine_->RegisterState(new WoodMonsterState::AttackState(this));
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