#include "SkyMap.h"
#include "Resource/Texture.h"
#include "FrameWork/Misc.h"
#include "Graphics/Graphics.h"

SkyMap::SkyMap()
{
    Texture::TextureData textureData = Texture::Instance().LoadTexture(L"./Resources/Image/SkyMap/Space.dds");
    shaderResourceView_ = textureData.shaderResourceView_;

    if (textureData.texture2dDesc_.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
    {
        isTextureCube_ = true;
    }

    Graphics::Instance().CreateVsFromCso("./Resources/Shader/SkyMapVS.cso", skyMapVS_.GetAddressOf(), NULL, NULL, 0);
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/SkyBoxPS.cso", skyBoxPS_.GetAddressOf());



}

void SkyMap::Draw()
{
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_OFF_ZW_OFF);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);

    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

    deviceContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    deviceContext->IASetInputLayout(NULL);

    deviceContext->VSSetShader(skyMapVS_.Get(), 0, 0);
    deviceContext->PSSetShader(skyBoxPS_.Get(), 0, 0);

    deviceContext->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());

    deviceContext->Draw(4, 0);

    deviceContext->VSSetShader(NULL, 0, 0);
    deviceContext->PSSetShader(NULL, 0, 0);
}

void SkyMap::DrawDebug()
{
}
