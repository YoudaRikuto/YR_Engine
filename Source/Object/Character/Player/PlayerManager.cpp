#include "PlayerManager.h"

// 初期化 
void PlayerManager::Initialize()
{
    player_->Initialize();
}

// 終了化 
void PlayerManager::Finalize()
{
    player_->Finalize();
}

// 更新 
void PlayerManager::Update(const float& elapsedTime)
{
    player_->Update(elapsedTime);
}

// 描画 
void PlayerManager::Render(ID3D11PixelShader* psShader)
{
    player_->Render(psShader);
}

// ImGui 
void PlayerManager::DrawDebug()
{
    player_->DrawDebug();
}

// デバッグ描画 
void PlayerManager::DebugRender()
{
    player_->DebugRender();
}
