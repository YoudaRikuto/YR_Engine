#include "GameScene.h"
#include "Object/Character/Player/PlayerManager.h"

// ----- リソース生成 -----
void GameScene::CreateResource()
{
    // プレイヤー生成
    PlayerManager::Instance().GetPlayer() = std::make_unique<Player>();

    stage_ = std::make_unique<Stage>();
}

// ----- 初期化 -----
void GameScene::Initialize()
{
    // プレイヤー初期化
    PlayerManager::Instance().Initialize();
}

// ----- 終了化 -----
void GameScene::Finalize()
{
    // プレイヤー終了化
    PlayerManager::Instance().Finalize();
}

// ----- 更新 -----
void GameScene::Update(const float& elapsedTime)
{
    // プレイヤー更新
    PlayerManager::Instance().Update(elapsedTime);
}

// ----- 描画 -----
void GameScene::Render()
{
    Graphics::Instance().SetBlendState(Shader::BlendState::None);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::Solid);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);


    // プレイヤー描画
    PlayerManager::Instance().Render();

    //PlayerManager::Instance().DebugRender();

    stage_->Render(nullptr);
}

// ----- ImGui -----
void GameScene::DrawDebug()
{
    // プレイヤー ImGui
    PlayerManager::Instance().DrawDebug();

    stage_->DrawDebug();
}
