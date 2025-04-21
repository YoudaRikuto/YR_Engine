#include "Camera.h"
#include "Graphics/Graphics.h"

// ----- çXêV -----
void Camera::Update(const float& elapsedTime)
{
}

// ----- ImGui -----
void Camera::DrawDebug()
{
}

void Camera::SetPerspectiveFov()
{
    const DirectX::XMFLOAT3 forward = transform_.CalcForward();
    eye_ = target_ + offset_ - forward * length_;
    focus_ = target_ + offset_;

    transform_.SetPosition(eye_);

    const float aspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    const float fov = DirectX::XMConvertToRadians(fov_);
    DirectX::XMStoreFloat4x4(&projection_, DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ_, farZ_));

    DirectX::XMVECTOR eye, focus, up;
    eye = DirectX::XMVectorSet(eye_.x, eye_.y, eye_.z, 1.0f);
    focus = DirectX::XMVectorSet(focus_.x, focus_.y, focus_.z, 1.0f);
    up = DirectX::XMVectorSet(up_.x, up_.y, up_.z, 0.0f);

    DirectX::XMStoreFloat4x4(&view_, DirectX::XMMatrixLookAtLH(eye, focus, up));
}
