#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include "Graphics/FrameBuffer.h"
#include "Graphics/FullscreenQuad.h"
#include "Graphics/ConstantBuffer.h"

class KawaseBloom
{
public:
    KawaseBloom();
    ~KawaseBloom() = default;

    void Execute(ID3D11ShaderResourceView* colorMap);
    void DrawDebug();

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetColorMap() { return kawaseBloom_->GetColorMap(); }

private:
    std::unique_ptr<FullscreenQuad> renderer_;

    // ---------- 輝度抽出 ----------
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   luminancePS_;
    std::unique_ptr<FrameBuffer>                luminanceExtraction_;

    // ---------- ぼかし ( ブラー ) ----------
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   gaussianBlurHorizontalPS_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   gaussianBlurVerticalPS_;
    static const int                            maxBlurBufferCount_ = 8;
    std::unique_ptr<FrameBuffer>                gaussianBlur_[maxBlurBufferCount_];

    // ---------- 最終結果 ----------
    Microsoft::WRL::ComPtr<ID3D11PixelShader>   kawaseBloomPS_;
    std::unique_ptr<FrameBuffer>                kawaseBloom_;

    struct BloomConstants
    {
        float bloomExtractionThreshold_ = 0.9f; // 輝度抽出閾値
        float bloomIntensity_           = 0.1f; // ブルーム強度
        float dummy_[2]                 = {};       
    };
    std::unique_ptr<ConstantBuffer<BloomConstants>> bloomConstants_;
};

