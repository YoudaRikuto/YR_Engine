#pragma once
#include "Shader/Shader.h"
#include <memory>

CONST LONG      SCREEN_WIDTH = 1920;
CONST LONG      SCREEN_HEIGHT = 1080;
CONST BOOL      FULLSCREEN = FALSE;
CONST LPCWSTR   APPLICATION_NAME = L"YR_Engine";

class Graphics
{
public:
    Graphics(HWND hwnd);
    ~Graphics() {}

    static Graphics& Instance() { return *instance_; }

    void RenderInitialize();    // 描画初期化
    void Draw();                // 描画実行

    ID3D11Device*           GetDevice()             const { return device_.Get(); }
    IDXGISwapChain*         GetSwapChain()          const { return swapchain_.Get(); }
    ID3D11DeviceContext*    GetDeviceContext()      const { return deviceContext_.Get(); }
    ID3D11RenderTargetView* GetRenderTargetView()   const { return renderTargetView_.Get(); }
    ID3D11DepthStencilView* GetDepthStencilView()   const { return depthStencilView_.Get(); }

    HRESULT CreateVsFromCso(const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements) { return shader_->CreateVsFromCso(csoName, vertexShader, inputLayout, inputElementDesc, numElements); }
    HRESULT CreatePsFromCso(const char* csoName, ID3D11PixelShader** pixelShader) { return shader_->CreatePsFromCso(csoName, pixelShader); }
    HRESULT CreateGsFromCso(const char* csoName, ID3D11GeometryShader** geometryShader) { return shader_->CreateGsFromCso(csoName, geometryShader); }
    HRESULT CreateCsFromCso(const char* csoName, ID3D11ComputeShader** computeShader) { return shader_->CreateCsFromCso(csoName, computeShader); }
    HRESULT CreateDsFromCso(const char* csoName, ID3D11DomainShader** domainShader) { return shader_->CreateDsFromCso(csoName, domainShader); }
    HRESULT CreateHsFromCso(const char* csoName, ID3D11HullShader** hullShader) { return shader_->CreateHsFromCso(csoName, hullShader); }
    void SetDepthStencileState(const Shader::DepthState& depthStencileState) { shader_->SetDepthStencileState(depthStencileState); }
    void SetBlendState(const Shader::BlendState& blendState) { shader_->SetBlendState(blendState); }
    void SetRasterizerState(const Shader::RasterState& rasterizerState) { shader_->SetRasterizerState(rasterizerState); }

    void SetGBuffer() { shader_->SetGBuffer(); }
    void SetGBufferShaderResourceView() { shader_->SetGBufferShaderResourceView(); }
    ID3D11ShaderResourceView** GetGBufferBaseColorShaderResourceView() { return shader_->GetGBufferBaseColorShaderResourceView(); }
    ID3D11PixelShader* GetGBufferPixelShader() { return shader_->GetGBufferPixelShader(); }
    
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetDepthMap() { return shader_->GetDepthMap(); }

private:
    void CreateRenderTargetView(); // レンダーターゲットビュー作成
    void CreateDepthStencilView(); // デプスステンシルビュー作成

private:
    static Graphics* instance_;
    std::unique_ptr<Shader> shader_;

    Microsoft::WRL::ComPtr<ID3D11Device>                device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>         deviceContext_;
    Microsoft::WRL::ComPtr<IDXGISwapChain>              swapchain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      renderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>             depthStencilBuffer_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      depthStencilView_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    depthShaderResourceView_;

    DirectX::XMFLOAT4   clearColor_ = { 0.0f, 1.0f, 1.0f, 1.0f };   // 初期化色
    UINT                syncInterval_ = 0;                          // 垂直同期
};
