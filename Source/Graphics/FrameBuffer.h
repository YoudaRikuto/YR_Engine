#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

class FrameBuffer
{
public:
    FrameBuffer(const uint32_t& width, const uint32_t& height);
    virtual ~FrameBuffer() = default;

    void Clear(float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);
    void Activate(ID3D11DepthStencilView* depthStencilVew = nullptr);
    void Deactivate();

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetColorMap() { return shaderResourceViews_[0]; }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetDepthMap() { return shaderResourceViews_[1]; }

private:
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews_[2];
    D3D11_VIEWPORT viewport_;

    UINT viewportCount_ = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3D11_VIEWPORT cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView_;
};

