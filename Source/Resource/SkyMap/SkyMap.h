#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

class SkyMap
{
public:
    SkyMap();
    ~SkyMap() {}

    void Draw();
    void DrawDebug();

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          skyMapVS_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           skyBoxPS_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    shaderResourceView_;

    struct Constants
    {
        DirectX::XMFLOAT4X4 inverseViewProjection_;
    };

    bool isTextureCube_ = false;
};

