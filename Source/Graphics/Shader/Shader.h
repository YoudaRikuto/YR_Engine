#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

class Shader
{
public:
    enum class BlendState { None, Alpha, Add, Multiply, Max };
    enum class RasterState { Solid, Wireframe, CullNone, WireframeCullNone, Max };
    enum class DepthState { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF, Max };
    enum class SamplerState { Point, Linear, Anisotropic, LinearBorderBlack, LinearBorderWhite, Comparison, Max };

public:
    Shader();
    ~Shader() {}

public:
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

private:
    void CreateBlendStates();           // ブレンドステート作成
    void CreateRasterizerStates();      // ラスタライザステート作成
    void CreateDepthStencilStates();    // デプスステンシルステート作成
    void CreateSamplerStates();         // サンプラーステート作成

private:
    Microsoft::WRL::ComPtr<ID3D11BlendState>        blendStates_[static_cast<int>(BlendState::Max)];
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   rasterizerStates_[static_cast<int>(RasterState::Max)];
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates_[static_cast<int>(DepthState::Max)];
    Microsoft::WRL::ComPtr<ID3D11SamplerState>      samplerStates_[static_cast<int>(SamplerState::Max)];
};