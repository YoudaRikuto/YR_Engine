#include "Texture.h"
#include <DDSTextureLoader.h>
#include <filesystem>
#include "Graphics/Graphics.h"
#include "Framework/Misc.h"

Texture::~Texture()
{
    textures_.clear();
}

// テクスチャ読み込み 
const Texture::TextureData Texture::LoadTexture(const wchar_t* filename)
{
    const TextureMap::iterator it = textures_.find(filename);
    if (it != textures_.end())
    {
        return it->second;
    }

    HRESULT result = S_OK;

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    TextureData textureData = {};

    // DDS ファイルがあれば、それを使用して読み込む
    std::filesystem::path ddsFilename(filename);
    ddsFilename.replace_extension("dds");
    if (std::filesystem::exists(ddsFilename.c_str()))
    {
        result = DirectX::CreateDDSTextureFromFile(Graphics::Instance().GetDevice(), ddsFilename.c_str(), resource.GetAddressOf(), textureData.shaderResourceView_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }
    // 通常読み込み
    else
    {
        result = DirectX::CreateWICTextureFromFile(Graphics::Instance().GetDevice(), filename, resource.GetAddressOf(), textureData.shaderResourceView_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
    result = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    texture2d->GetDesc(&textureData.texture2dDesc_);

    // リストに登録する
    textures_.insert(std::make_pair(filename, textureData));

    return textureData;
}

// テクスチャ読み込み 
const Texture::TextureData Texture::LoadTexture(const void* data, const size_t& size)
{
    HRESULT         result = S_OK;
    ID3D11Device* device = Graphics::Instance().GetDevice();

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    TextureData textureData = {};

    result = DirectX::CreateDDSTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), textureData.shaderResourceView_.GetAddressOf());

    if (result != S_OK)
    {
        result = DirectX::CreateWICTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data), size, resource.GetAddressOf(), textureData.shaderResourceView_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    return textureData;
}

// ダミーテクスチャ生成 
const Texture::TextureData Texture::MakeDummyTexture(const DWORD& value, const UINT& dimension)
{
    HRESULT     result = S_OK;
    TextureData textureData = {};

    textureData.texture2dDesc_.Width = dimension;
    textureData.texture2dDesc_.Height = dimension;
    textureData.texture2dDesc_.MipLevels = 1;
    textureData.texture2dDesc_.ArraySize = 1;
    textureData.texture2dDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureData.texture2dDesc_.SampleDesc.Count = 1;
    textureData.texture2dDesc_.SampleDesc.Quality = 0;
    textureData.texture2dDesc_.Usage = D3D11_USAGE_DEFAULT;
    textureData.texture2dDesc_.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    const size_t texels = dimension * dimension;
    std::unique_ptr<DWORD[]> sysmem = std::make_unique<DWORD[]>(texels);
    for (size_t i = 0; i < texels; ++i) sysmem[i] = value;

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = sysmem.get();
    subresourceData.SysMemPitch = sizeof(DWORD) * dimension;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
    result = Graphics::Instance().GetDevice()->CreateTexture2D(&textureData.texture2dDesc_, &subresourceData, &texture2d);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = textureData.texture2dDesc_.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    result = Graphics::Instance().GetDevice()->CreateShaderResourceView(texture2d.Get(), &shaderResourceViewDesc, textureData.shaderResourceView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    return textureData;
}
