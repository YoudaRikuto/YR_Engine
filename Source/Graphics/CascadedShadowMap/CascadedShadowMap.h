#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include <functional>
#include <vector>
#include "Graphics/ConstantBuffer.h"

class CascadedShadowMap
{
public:
    CascadedShadowMap(const UINT& width, const UINT& height, const UINT& cascadeCount = 4);
    virtual ~CascadedShadowMap() = default;

    void DrawDebug();

    void Activate(const DirectX::XMFLOAT4& lightDirection, const UINT& cbSlot);
    void Deactivate();
    void Clear();

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetDepthMap() { return shaderResourceView_; }

private:
    D3D11_VIEWPORT  cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    UINT            viewportCount_ = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   cachedRenderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   cachedDepthStencilView_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          depthStencilBuffer_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   depthStencilView_;
    D3D11_VIEWPORT                                   viewport_;
    std::vector<DirectX::XMFLOAT4X4>                 cascadedMatrices_;
    std::vector<float>                               cascadedPlaneDistances_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;

    struct CascadedShadowConstants
    {
        DirectX::XMFLOAT4X4 cascadedMatrices_[4];
        float               cascadedPlaneDistances_[4];
    };
    std::unique_ptr<ConstantBuffer<CascadedShadowConstants>> cascadedShadowConstants_;

    const UINT  cascadeCount_;
    float       splitSchemeWeight_ = 0.7f;
    float       zMult_ = 10.0f;
    float       criticalDepthValue_ = 0.0f;
    bool        fitToCascade_ = true;
};