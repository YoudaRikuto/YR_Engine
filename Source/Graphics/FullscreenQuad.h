#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class FullscreenQuad
{
public:
    FullscreenQuad();
    virtual ~FullscreenQuad() = default;

    void Draw(ID3D11ShaderResourceView** shaderResourceView, const uint32_t& startSlot, const uint32_t& numViews, ID3D11PixelShader* replacedPixelShader = nullptr);

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>  vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   pixelShader_;
};