#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "Math/Transform.h"

class Camera
{
private:
    Camera() {}
    ~Camera() {}

public:
    static Camera& Instance()
    {
        static Camera instance;
        return instance;
    }

    void Update(const float& elapsedTime);  // çXêV
    void DrawDebug();                       // ImGui

    void SetPerspectiveFov();

    [[nodiscard]] const DirectX::XMFLOAT4X4 GetView() const { return view_; }
    [[nodiscard]] const DirectX::XMFLOAT4X4 GetProjection() const { return projection_; }
    [[nodiscard]] const DirectX::XMFLOAT3 GetEye() const { return eye_; }

private:
    Transform3D         transform_ = {};
    DirectX::XMFLOAT4X4 view_ = {};
    DirectX::XMFLOAT4X4 projection_ = {};
    DirectX::XMFLOAT3   eye_ = {};
    DirectX::XMFLOAT3   focus_ = {};
    DirectX::XMFLOAT3   up_ = { 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT3   target_ = {};
    DirectX::XMFLOAT3   offset_ = {};
    float               nearZ_ = 0.1f;
    float               farZ_ = 150.0f;
    float               fov_ = 45.0f;
    float               length_ = 10.0f;
};

