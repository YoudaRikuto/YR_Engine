#include "Camera.h"
#include "Graphics/Graphics.h"
#include "Object/Character/Player/PlayerManager.h"

// ----- 更新 -----
void Camera::Update(const float& elapsedTime)
{
    DirectX::XMFLOAT3 cameraTargetPosition = { PlayerManager::Instance().GetTransform()->GetPositionX(), 0.0f, PlayerManager::Instance().GetTransform()->GetPositionZ() };

    target_ = XMFloat3Lerp(target_, cameraTargetPosition, 0.12f);
    //target_ = XMFloat3Lerp(target_, cameraTargetPosition, lerpWeight_);
}

// ----- ImGui -----
void Camera::DrawDebug()
{
}

void Camera::SetPerspectiveFov()
{
    const DirectX::XMFLOAT3 forward = transform_.CalcForward();
    eye_    = target_ + offset_ - forward * length_;
    focus_  = target_ + offset_;

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

// ----- カメラから見たベクトルに変換する -----
const DirectX::XMFLOAT2 Camera::ConvertTo2DVectorFromCamera(const DirectX::XMFLOAT2& v)
{
    DirectX::XMFLOAT2 result = {};

    const DirectX::XMFLOAT3 forward = CalcForward();
    const DirectX::XMFLOAT3 right   = CalcRight();

    result.x = forward.x * v.y + right.x * v.x;
    result.y = forward.z * v.y + right.z * v.x;

    return XMFloat2Normalize(result);
}

// ----- 前ベクトル算出 -----
const DirectX::XMFLOAT3 Camera::CalcForward()
{
    return XMFloat3Normalize(focus_ - eye_);
}

// ----- 右ベクトル算出 -----
const DirectX::XMFLOAT3 Camera::CalcRight()
{
    const DirectX::XMFLOAT3 forward = CalcForward();
    const DirectX::XMFLOAT3 up      = DirectX::XMFLOAT3(0, 1, 0);

    return XMFloat3Normalize(XMFloat3Cross(up, forward));
}
