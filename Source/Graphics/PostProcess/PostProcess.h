#pragma once
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include "Graphics/FrameBuffer.h"
#include "Graphics/FullscreenQuad.h"
#include "Graphics/PostProcess/Bloom/KawaseBloom.h"
#include "Graphics/CascadedShadowMap/CascadedShadowMap.h"

class PostProcess
{
private:
    PostProcess();
    ~PostProcess() {}

public:
    static PostProcess& Instance()
    {
        static PostProcess instance;
        return instance;
    }

    void Activate();
    void Deactivate();
    void Draw();

    void DrawDebug();

    void MakeCascadedShadowMap(const DirectX::XMFLOAT4& lightDirection, const UINT& cbSlot, std::function<void()> drawcallback);

private:
    std::unique_ptr<FullscreenQuad> renderer_;
    std::unique_ptr<FrameBuffer>    sceneBuffer_;
    std::unique_ptr<FrameBuffer>    postProcess_;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> postProcessPS_;

    KawaseBloom         kawaseBloom_;
    CascadedShadowMap   cascadedShadowMap_;

    struct PostProcessConstants
    {
        float brightness_   = -0.05f;
        float contrast_     = 0.2f;
        float hue_          = 0.0f;
        float saturation_   = 0.0f;
    };
    std::unique_ptr<ConstantBuffer<PostProcessConstants>> postProcessConstants_;
};

