#include "FullscreenQuad.h"
#include "Graphics/Graphics.h"
#include "FrameWork/Misc.h"

FullscreenQuad::FullscreenQuad()
{
    Graphics::Instance().CreateVsFromCso("./Resources/Shader/FullScreenQuadVS.cso", vertexShader_.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/FullScreenQuadPS.cso", pixelShader_.ReleaseAndGetAddressOf());
}

// •`‰æ
void FullscreenQuad::Draw(ID3D11ShaderResourceView** shaderResourceView, const uint32_t& startSlot, const uint32_t& numViews, ID3D11PixelShader* replacedPixelShader)
{
    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

    deviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    deviceContext->IASetInputLayout(nullptr);

    deviceContext->VSSetShader(vertexShader_.Get(), 0, 0);
    replacedPixelShader ? deviceContext->PSSetShader(replacedPixelShader, 0, 0) : deviceContext->PSSetShader(pixelShader_.Get(), 0, 0);

    deviceContext->PSSetShaderResources(startSlot, numViews, shaderResourceView);

    deviceContext->Draw(4, 0);
}