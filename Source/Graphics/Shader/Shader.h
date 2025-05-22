#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

class Shader
{
public:
    enum class BlendState { None, Alpha, Add, Multiply, MRT, Max };
    enum class RasterState { Solid, Wireframe, CullNone, WireframeCullNone, Max };
    enum class DepthState { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF, Max };
    enum class SamplerState { Point, Linear, Anisotropic, LinearBorderBlack, LinearBorderWhite, Comparison, Max };
    enum class GBufferId { BaseColor, Emissive, Normal, Parameters, Depth, Max };

public:
    Shader();
    ~Shader() {}

    // ---------- シェーダーオブジェクト生成関数 ----------
    HRESULT CreateVsFromCso(const char* csoName, ID3D11VertexShader** vertexShader, ID3D11InputLayout** inputLayout, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, UINT numElements);
    HRESULT CreatePsFromCso(const char* csoName, ID3D11PixelShader** pixelShader);
    HRESULT CreateGsFromCso(const char* csoName, ID3D11GeometryShader** geometryShader);
    HRESULT CreateCsFromCso(const char* csoName, ID3D11ComputeShader** computeShader);
    HRESULT CreateDsFromCso(const char* csoName, ID3D11DomainShader** domainShader);
    HRESULT CreateHsFromCso(const char* csoName, ID3D11HullShader** hullShader);

    void SetBlendState(const BlendState& blendState);
    void SetRasterizerState(const RasterState& rasterizerState);
    void SetDepthStencileState(const DepthState& depthStencileState);
    void SetSamplerState();

    void SetGBuffer();
    void SetGBufferShaderResourceView();
    ID3D11ShaderResourceView** GetGBufferBaseColorShaderResourceView() { return gBufferShaderResourceView_[0].GetAddressOf(); }
    ID3D11PixelShader* GetGBufferPixelShader() { return gBufferPixelShader_.Get(); }

    ID3D11DepthStencilView* GetGBufferDepthStencilView() { return gBufferDepthStencilView_.Get(); }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetDepthMap() { return gBufferDepthShaderResourceView_; }

private:
    void CreateBlendStates();           // ブレンドステート作成
    void CreateRasterizerStates();      // ラスタライザステート作成
    void CreateDepthStencilStates();    // デプスステンシルステート作成
    void CreateSamplerStates();         // サンプラーステート作成
    void CreateGBuffer();

private:
    Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStates_[static_cast<int>(BlendState::Max)];
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   rasterizerStates_[static_cast<int>(RasterState::Max)];
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates_[static_cast<int>(DepthState::Max)];
    Microsoft::WRL::ComPtr<ID3D11SamplerState>      samplerStates_[static_cast<int>(SamplerState::Max)];

    // ---------- G-Buffer ----------
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      gBufferRenderTargetView_[static_cast<int>(GBufferId::Max)];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    gBufferShaderResourceView_[static_cast<int>(GBufferId::Max)];
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           gBufferPixelShader_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>             gBufferDepthStencilBuffer_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      gBufferDepthStencilView_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    gBufferDepthShaderResourceView_;
};