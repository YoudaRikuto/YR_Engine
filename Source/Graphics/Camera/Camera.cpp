#include "Camera.h"
#include "Graphics/Graphics.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Input/Input.h"

// 更新 
void Camera::Update(const float& elapsedTime)
{
    CutSceneCamera::Instance().Update(elapsedTime);
    // カットシーンカメラが有効
    if (CutSceneCamera::Instance().IsCutSceneCameraActive())
    {
        const CutSceneCameraInfo cutSceneCameraInfo = CutSceneCamera::Instance().GetCutSceneCameraInfo();
        transform_.SetRotation(cutSceneCameraInfo.rotation_);
        target_ = cutSceneCameraInfo.target_;
        offset_ = cutSceneCameraInfo.offset_;
        length_ = cutSceneCameraInfo.length_;

        // =================================================
        //  これより下の処理をCutSceneCameraに持っていきたい
        // =================================================
        DirectX::XMFLOAT3 targetForward = {};
        if (cutSceneCameraInfo.targetName_ == "Player")
        {
            targetForward = PlayerManager::Instance().GetTransform()->CalcForward();
        }
        else if (cutSceneCameraInfo.targetName_ == "WoodMonster")
        {
            targetForward = EnemyManager::Instance().GetEnemy(0)->GetTransform()->CalcForward();
        }

        DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
        DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, DirectX::XMLoadFloat3(&targetForward)));
        up = DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&targetForward), right);

        DirectX::XMMATRIX basis = DirectX::XMMATRIX(right, up, DirectX::XMLoadFloat3(&targetForward), DirectX::XMVectorSet(0, 0, 0, 1));
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(cutSceneCameraInfo.rotation_.x, cutSceneCameraInfo.rotation_.y, cutSceneCameraInfo.rotation_.z);
        DirectX::XMVECTOR offset = DirectX::XMVector3TransformCoord(DirectX::XMVectorSet(0, 0, -length_, 0), rotation);
        DirectX::XMStoreFloat3(&cutSceneCameraPosition, DirectX::XMVector3TransformCoord(offset, basis));

        // =================================================
        //  これより上の処理をCutSceneCameraに持っていきたい
        // =================================================

        return;
    }

    //DirectX::XMFLOAT3 cameraTargetPosition = { PlayerManager::Instance().GetTransform()->GetPositionX(), 0.0f, PlayerManager::Instance().GetTransform()->GetPositionZ() };
    DirectX::XMFLOAT3 cameraTargetPosition = PlayerManager::Instance().GetTransform()->GetPosition();

    target_ = XMFloat3Lerp(target_, cameraTargetPosition, 0.12f);
    //target_ = XMFloat3Lerp(target_, cameraTargetPosition, lerpWeight_);

    Rotate(elapsedTime);
}

// ImGui 
void Camera::DrawDebug()
{
    ImGui::Begin("Camera");

    ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
    ImGui::DragFloat("Length", &length_, 0.1f);

    ImGui::DragFloat("Fov", &fov_);

    ImGui::End();

    CutSceneCamera::Instance().DrawDebug();
}

void Camera::SetPerspectiveFov()
{
    const DirectX::XMFLOAT3 forward = transform_.CalcForward();
    eye_    = target_ + offset_ - forward * length_;
    focus_  = target_ + offset_;

    // カットシーンカメラが有効
    if (CutSceneCamera::Instance().IsCutSceneCameraActive())
    {
        if (cutSceneCameraPosition.x == 0.0f &&
            cutSceneCameraPosition.y == 0.0f &&
            cutSceneCameraPosition.z == 0.0f)
        {

        }
        else
        {
        eye_ = target_ + offset_ + cutSceneCameraPosition;

        }

    }

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

void Camera::CalcCameraVectorFromCutSceneCameraInfo(const CutSceneCameraInfo& cutSceneCameraInfo, DirectX::XMFLOAT3& position, DirectX::XMFLOAT3& forwardVec)
{
    Transform3D transform = {};
    transform.SetRotation(cutSceneCameraInfo.rotation_);
    const DirectX::XMFLOAT3 forward = transform.CalcForward();
    const DirectX::XMFLOAT3 eye = cutSceneCameraInfo.target_ + cutSceneCameraInfo.offset_ - forward * cutSceneCameraInfo.length_;
    const DirectX::XMFLOAT3 focus = cutSceneCameraInfo.target_ + cutSceneCameraInfo.offset_;

    position = eye;
    forwardVec = focus - eye;
}

// カメラから見たベクトルに変換する 
const DirectX::XMFLOAT2 Camera::ConvertTo2DVectorFromCamera(const DirectX::XMFLOAT2& v)
{
    DirectX::XMFLOAT2 result = {};

    const DirectX::XMFLOAT3 forward = CalcForward();
    const DirectX::XMFLOAT3 right   = CalcRight();

    result.x = forward.x * v.y + right.x * v.x;
    result.y = forward.z * v.y + right.z * v.x;

    return XMFloat2Normalize(result);
}

// 前ベクトル算出 
const DirectX::XMFLOAT3 Camera::CalcForward()
{
    return XMFloat3Normalize(focus_ - eye_);
}

// 右ベクトル算出 
const DirectX::XMFLOAT3 Camera::CalcRight()
{
    const DirectX::XMFLOAT3 forward = CalcForward();
    const DirectX::XMFLOAT3 up      = DirectX::XMFLOAT3(0, 1, 0);

    return XMFloat3Normalize(XMFloat3Cross(up, forward));
}

// 回転処理 
void Camera::Rotate(const float& elapsedTime)
{
    const float aRx = Input::Instance().GetGamePad().GetAxisRx();
    const float aRy = Input::Instance().GetGamePad().GetAxisRy();
    
    DirectX::XMFLOAT3 rotation = transform_.GetRotation();

    rotation.y += aRx * horizontalRotationSpeed_ * elapsedTime;

    rotation.x += aRy * verticalRotationSpeed_ * elapsedTime;
    rotation.x = std::clamp(rotation.x, minRotationX_, maxRotationX_);

    transform_.SetRotation(rotation);
}
