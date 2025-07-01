#include "GameScene.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Graphics/Camera/Camera.h"
#include "Resource/Texture.h"
#include "Resource/Effect.h"
#include "Resource/EffectManager.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Object/Character/Enemy/WoodMonster/WoodMonster.h"
#include "Resource/ComputeParticle/ComputeParticleSystem.h"
#include "Resource/Audio/AudioManager.h"

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

    ComputeParticleSystem::Instance().LoadEmitDataFromJsonFile("BBB");
}

// 初期化 
void GameScene::Initialize()
{
    // プレイヤー初期化
    PlayerManager::Instance().Initialize();

    // BGM再生
    AudioManager::Instance().PlayBGM(BGM::Game);
}

// 終了化 
void GameScene::Finalize()
{
    // プレイヤー終了化
    PlayerManager::Instance().Finalize();

    EnemyManager::Instance().Finalize();

    ComputeParticleSystem::Instance().ClearResource();

    // BGM停止
    AudioManager::Instance().StopBGM(BGM::Game);
}

// 更新 
void GameScene::Update(const float& elapsedTime)
{
    // プレイヤー更新
    PlayerManager::Instance().Update(elapsedTime);

    EnemyManager::Instance().Update(elapsedTime);

    // エフェクト更新
    ComputeParticleSystem::Instance().Update(elapsedTime);
}

void GameScene::DeferredRender()
{
    Graphics::Instance().SetBlendState(Shader::BlendState::None);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::Solid);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);

    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(32, 1, iblTextures_[0].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(33, 1, iblTextures_[1].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(34, 1, iblTextures_[2].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(35, 1, iblTextures_[3].GetAddressOf());

    ID3D11PixelShader* gBufferPS = Graphics::Instance().GetGBufferPixelShader();

    // プレイヤー描画
    PlayerManager::Instance().Render(gBufferPS);

    EnemyManager::Instance().Render(gBufferPS);

    //PlayerManager::Instance().DebugRender();

    stage_->Render(gBufferPS);
}

// 描画 
void GameScene::Render()
{
    Graphics::Instance().SetBlendState(Shader::BlendState::Alpha);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_OFF);
    ComputeParticleSystem::Instance().Render();

    Graphics::Instance().SetBlendState(Shader::BlendState::Alpha);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);
    DebugRenderer* debugRenderer = Graphics::Instance().GetDebugRenderer();

    PlayerManager::Instance().DebugRender(debugRenderer);
    EnemyManager::Instance().DebugRender(debugRenderer);

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

    ComputeParticleSystem::Instance().DrawDebug();

    ImGui::End();
}
