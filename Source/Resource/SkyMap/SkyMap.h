#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

class SkyMap
{
public:
    SkyMap();
    ~SkyMap() {}

    void Draw(const int& textureType = -1);
    void DrawDebug();

    enum class TextureType
    {
        Space,
        Space1,
        BlueSpace,
        Sunset,
        Max
    };

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          skyMapVS_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           skyBoxPS_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    shaderResourceView_[static_cast<int>(TextureType::Max)];
    TextureType                                         textureType_ = TextureType::Space;

    bool isTextureCube_ = false;
};

