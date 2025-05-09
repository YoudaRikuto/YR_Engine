#include "Graphics.h"
#include "Framework/Misc.h"

Graphics* Graphics::instance_ = nullptr;

Graphics::Graphics(HWND hwnd)
{
    // インスタンス設定
    _ASSERT_EXPR(instance_ == nullptr, "already instantiated");
    instance_ = this;

    HRESULT result = S_OK;
    UINT    createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif// _DEBUG

    D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
    swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = !FULLSCREEN;

    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
        createDeviceFlags, &featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc,
        swapchain_.GetAddressOf(), device_.GetAddressOf(), NULL, deviceContext_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    // レンダーターゲットビュー作成
    CreateRenderTargetView();

    // デプスステンシルビュー作成
    CreateDepthStencilView();

    // ビューポートの設定
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(SCREEN_WIDTH);
    viewport.Height = static_cast<float>(SCREEN_HEIGHT);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext_->RSSetViewports(1, &viewport);

    shader_ = std::make_unique<Shader>();
}

// 描画初期化 
void Graphics::RenderInitialize()
{
    FLOAT   color[] = { clearColor_.x, clearColor_.y, clearColor_.z, clearColor_.w };

    deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), color);
    deviceContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    deviceContext_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

    shader_->SetSamplerState();
}

// 描画実行 
void Graphics::Draw()
{
    swapchain_->Present(syncInterval_, 0);
}

// レンダーターゲットビュー作成 
void Graphics::CreateRenderTargetView()
{
    HRESULT                                 result = S_OK;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = {};

    result = swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(backBuffer.GetAddressOf()));
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    result = device_->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// デプスステンシルビュー作成 
void Graphics::CreateDepthStencilView()
{
    HRESULT                                 result = S_OK;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer = {};
    D3D11_TEXTURE2D_DESC                    texture2dDesc = {};

    texture2dDesc.Width = SCREEN_WIDTH;
    texture2dDesc.Height = SCREEN_HEIGHT;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texture2dDesc.CPUAccessFlags = 0;
    texture2dDesc.MiscFlags = 0;

    result = device_->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Flags = 0;

    result = device_->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));


    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    result = device_->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, depthShaderResourceView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}
