#include "GameScene.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Graphics/Camera/Camera.h"
#include "Resource/Texture.h"
#include "Resource/Effect.h"
#include "Resource/EffectManager.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Object/Character/Enemy/WoodMonster/WoodMonster.h"

// リソース生成 
void GameScene::CreateResource()
{
    // プレイヤー生成
    PlayerManager::Instance().GetPlayer() = std::make_unique<Player>();

    EnemyManager::Instance().Register(new WoodMonster);

    stage_ = std::make_unique<Stage>();

    Texture::TextureData textureData0 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/sunset_jhbcentral_4k.dds");
    Texture::TextureData textureData1 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/diffuse_iem.dds");
    Texture::TextureData textureData2 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/specular_pmrem.dds");
    Texture::TextureData textureData3 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/lut_ggx.DDS");

    iblTextures_[0] = textureData0.shaderResourceView_;
    iblTextures_[1] = textureData1.shaderResourceView_;
    iblTextures_[2] = textureData2.shaderResourceView_;
    iblTextures_[3] = textureData3.shaderResourceView_;

    Effect* effect = new Effect("./Resources/Effect/kemuri.efk", "kemuri");
}

// 初期化 
void GameScene::Initialize()
{
    // プレイヤー初期化
    PlayerManager::Instance().Initialize();
}

// 終了化 
void GameScene::Finalize()
{
    // プレイヤー終了化
    PlayerManager::Instance().Finalize();

    EnemyManager::Instance().Finalize();
}

// 更新 
void GameScene::Update(const float& elapsedTime)
{
    // プレイヤー更新
    PlayerManager::Instance().Update(elapsedTime);

    EnemyManager::Instance().Update(elapsedTime);
}

// 描画 
void GameScene::Render()
{
    Graphics::Instance().SetBlendState(Shader::BlendState::None);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::Solid);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);

    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(32, 1, iblTextures_[0].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(33, 1, iblTextures_[1].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(34, 1, iblTextures_[2].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(35, 1, iblTextures_[3].GetAddressOf());

    // プレイヤー描画
    PlayerManager::Instance().Render();

    EnemyManager::Instance().Render();

    //PlayerManager::Instance().DebugRender();

    stage_->Render(nullptr);
}

// 影書き込み
void GameScene::ShadowRender()
{
}

// ImGui 
void GameScene::DrawDebug()
{
    ImGui::Begin("Game");

    // プレイヤー ImGui
    PlayerManager::Instance().DrawDebug();

    EnemyManager::Instance().DrawDebug();

    stage_->DrawDebug();

    Camera::Instance().DrawDebug();

    if (ImGui::Button("Effect"))
    {
        EffectManager::Instance().GetEffect("kemuri")->Play({}, 1.0f);
    }

    ImGui::End();
}
