#pragma once
#include <WICTextureLoader.h>
#include <wrl.h>
#include <map>
#include <string>
#include <memory>

class Texture
{
private:
    Texture() {}
    ~Texture();

public:
    static Texture& Instance()
    {
        static Texture instance;
        return instance;
    }

    class TextureData
    {
    public:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    shaderResourceView_;
        D3D11_TEXTURE2D_DESC                                texture2dDesc_;
    };

public:
    [[nodiscard]] const TextureData LoadTexture(const wchar_t* filename);
    [[nodiscard]] const TextureData LoadTexture(const void* data, const size_t& size);
    [[nodiscard]] const TextureData MakeDummyTexture(const DWORD& value, const UINT& dimension);

private:
    using TextureMap = std::map<std::wstring, TextureData>;
    TextureMap textures_;
};

