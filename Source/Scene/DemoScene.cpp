#include "DemoScene.h"
#include "Resource/Texture.h"

// リソース生成 
void DemoScene::CreateResource()
{
    gltfModel_ = std::make_unique<GltfModel>("./Resources/Model/Player/Player.gltf");

    Texture::TextureData textureData0 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/sunset_jhbcentral_4k.dds");
    Texture::TextureData textureData1 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/diffuse_iem.dds");
    Texture::TextureData textureData2 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/specular_pmrem.dds");
    Texture::TextureData textureData3 = Texture::Instance().LoadTexture(L"./Resources/Image/Environments/Sunset/lut_ggx.DDS");

    iblTextures_[0] = textureData0.shaderResourceView_;
    iblTextures_[1] = textureData1.shaderResourceView_;
    iblTextures_[2] = textureData2.shaderResourceView_;
    iblTextures_[3] = textureData3.shaderResourceView_;
}

// 初期化 
void DemoScene::Initialize()
{
}

// 終了化 
void DemoScene::Finalize()
{
}

// 更新 
void DemoScene::Update(const float& elapsedTime)
{
}

// 描画 
void DemoScene::Render()
{
    Graphics::Instance().SetBlendState(Shader::BlendState::None);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::Solid);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);

    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(32, 1, iblTextures_[0].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(33, 1, iblTextures_[1].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(34, 1, iblTextures_[2].GetAddressOf());
    Graphics::Instance().GetDeviceContext()->PSSetShaderResources(35, 1, iblTextures_[3].GetAddressOf());

    gltfModel_->Render(1.0f);
}

// 影書き込み
void DemoScene::ShadowRender()
{
}

// ImGui用 
void DemoScene::DrawDebug()
{
    gltfModel_->DrawDebug();
}
