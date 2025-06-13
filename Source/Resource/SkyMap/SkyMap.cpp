#include "SkyMap.h"
#include "Resource/Texture.h"
#include "FrameWork/Misc.h"
#include "Graphics/Graphics.h"
#include "ImGui/ImGuiCtrl.h"

SkyMap::SkyMap()
{
    Texture::TextureData textureData = {};
    const wchar_t* filenames[] =
    {
        L"./Resources/Image/SkyMap/Space.dds",
        L"./Resources/Image/SkyMap/Space1.dds",
        L"./Resources/Image/SkyMap/BlueSpace.dds",
        L"./Resources/Image/SkyMap/Sunset.dds",
    };

    for (int i = 0; i < static_cast<int>(TextureType::Max); ++i)
    {
        textureData = Texture::Instance().LoadTexture(filenames[i]);
        shaderResourceView_[i] = textureData.shaderResourceView_;
    }

    //if (textureData.texture2dDesc_.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
    //{
    //    isTextureCube_ = true;
    //}

    Graphics::Instance().CreateVsFromCso("./Resources/Shader/SkyMapVS.cso", skyMapVS_.GetAddressOf(), NULL, NULL, 0);
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/SkyBoxPS.cso", skyBoxPS_.GetAddressOf());
}

void SkyMap::Draw(const int& textureType)
{
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_OFF_ZW_OFF);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);

    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

    deviceContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    deviceContext->IASetInputLayout(NULL);

    deviceContext->VSSetShader(skyMapVS_.Get(), 0, 0);
    deviceContext->PSSetShader(skyBoxPS_.Get(), 0, 0);

    int textureId = textureType;
    if (textureId == -1) textureId = static_cast<int>(textureType_);
    deviceContext->PSSetShaderResources(0, 1, shaderResourceView_[textureId].GetAddressOf());

    deviceContext->Draw(4, 0);

    deviceContext->VSSetShader(NULL, 0, 0);
    deviceContext->PSSetShader(NULL, 0, 0);
}

void SkyMap::DrawDebug()
{
    int textureType = static_cast<int>(textureType_);
    ImGui::SliderInt("TextureType", &textureType, 0, static_cast<int>(TextureType::Max) - 1);
    textureType_ = static_cast<TextureType>(textureType);
}
