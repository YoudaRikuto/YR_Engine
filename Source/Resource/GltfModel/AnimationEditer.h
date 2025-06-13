#pragma once
#include <memory>
#include "Graphics/FrameBuffer.h"
#include "Graphics/ConstantBuffer.h"
#include "Object/Object.h"
#include "Resource/SkyMap/SkyMap.h"

class AnimationEditer
{
public:
    AnimationEditer();
    ~AnimationEditer() {}

    void Update(const float& elapsedTime);
    void Render();

    void DrawDebug();

private:
    void SetPerspectiveFov(); // ÉJÉÅÉâçXêV

private:
    SkyMap skyMap_;

    std::unique_ptr<FrameBuffer> frameBuffer_;

    std::unique_ptr<Object> stage_;

    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 viewProjection_;
        DirectX::XMFLOAT4   lightDirection_;
        DirectX::XMFLOAT4   cameraPosition_;

        DirectX::XMFLOAT4X4 inverseProjection_;
        DirectX::XMFLOAT4X4 inverseViewProjection_;
        DirectX::XMFLOAT4X4 inverseView_;
    };
    ConstantBuffer<SceneConstants> sceneConstants_;

    Transform3D transform_;
    DirectX::XMFLOAT4X4 view_       = {};
    DirectX::XMFLOAT4X4 projection_ = {};
    DirectX::XMFLOAT3   eye_        = {};
    DirectX::XMFLOAT3   focus_      = {};
    DirectX::XMFLOAT3   up_         = { 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT3   target_     = { 0.0f, 1.0f, -5.0f };
    DirectX::XMFLOAT3   offset_     = {};
    float               nearZ_      = 0.1f;
    float               farZ_       = 150.0f;
    float               fov_        = 45.0f;
    float               length_     = 6.0f;
};