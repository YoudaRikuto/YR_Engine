#include "Transform.h"
#include "ImGui/ImGuiCtrl.h"

#pragma region ---------- Transform2D ----------
// ----- ImGui -----
void Transform2D::DrawDebug()
{
#ifdef USE_IMGUI
    if (ImGui::TreeNodeEx("Transform2D", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat2("Position", &position_.x);
        ImGui::DragFloat2("Size", &size_.x);
        ImGui::ColorEdit4("Color", &color_.x);
        ImGui::DragFloat2("TexPos", &texPos_.x);
        ImGui::DragFloat2("TexSize", &texSize_.x);
        ImGui::DragFloat2("Pivot", &pivot_.x);
        ImGui::DragFloat("Angle", &angle_);

        ImGui::TreePop();
    }

#endif// USE_IMGUI
}

#pragma endregion ---------- Transform2D ----------

#pragma region ---------- Transform3D ----------
// ----- ImGui -----
void Transform3D::DrawDebug()
{
#ifdef USE_IMGUI
    if (ImGui::TreeNodeEx("Transform3D", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Position", &position_.x, 0.1f);
        ImGui::DragFloat3("Scale", &scale_.x, 0.1f);

        DirectX::XMFLOAT3 rotation = {};
        rotation.x = DirectX::XMConvertToDegrees(rotation_.x);
        rotation.y = DirectX::XMConvertToDegrees(rotation_.y);
        rotation.z = DirectX::XMConvertToDegrees(rotation_.z);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
        rotation_.x = DirectX::XMConvertToRadians(rotation.x);
        rotation_.y = DirectX::XMConvertToRadians(rotation.y);
        rotation_.z = DirectX::XMConvertToRadians(rotation.z);

        ImGui::DragFloat("ScaleFactor", &scaleFactor_, 0.01f, 0.0f, 1000.0f);

        int coordinateSystem = static_cast<int>(coordinateSystem_);
        ImGui::SliderInt("Coordinate System", &coordinateSystem, 0, static_cast<int>(CoordinateSystem::cLeftZup));
        coordinateSystem_ = static_cast<CoordinateSystem>(coordinateSystem);
        ImGui::Text(coordinateSystemName_[static_cast<int>(coordinateSystem_)].c_str());

        if (ImGui::Button("Reset Transform"))
        {
            ResetTransform();
        }

        ImGui::TreePop();
    }

#endif// USE_IMGUI
}

// ----- Reset -----
void Transform3D::ResetTransform()
{
    position_ = DirectX::XMFLOAT3(0, 0, 0);
    scale_ = DirectX::XMFLOAT3(1, 1, 1);
    rotation_ = DirectX::XMFLOAT4(0, 0, 0, 0);
}

// ----- World行列算出 -----
const DirectX::XMMATRIX Transform3D::CalcWorld()
{
    const DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z) * DirectX::XMMatrixScaling(scaleFactor_, scaleFactor_, scaleFactor_);
    const DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    const DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);

    return S * R * T;
}

// ----- 座標系を考慮したWorld行列算出 -----
const DirectX::XMMATRIX Transform3D::CalcWorldMatrix(const float& scaleFactor)
{
    const DirectX::XMMATRIX C = DirectX::XMLoadFloat4x4(&coordinateSystemTransforms_[static_cast<int>(coordinateSystem_)])
        * DirectX::XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor);

    return C * CalcWorld();
}

// ----- 前方向ベクトル算出 -----
const DirectX::XMFLOAT3 Transform3D::CalcForward() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    DirectX::XMFLOAT3 forward = {};
    DirectX::XMStoreFloat3(&forward, DirectX::XMVector3Normalize(rotationMatrix.r[2]));
    return forward;
}

// ----- 上方向ベクトル算出 -----
const DirectX::XMFLOAT3 Transform3D::CalcUp() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    DirectX::XMFLOAT3 up = {};
    DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(rotationMatrix.r[1]));
    return up;
}

// ----- 右方向ベクトル算出 -----
const DirectX::XMFLOAT3 Transform3D::CalcRight() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    DirectX::XMFLOAT3 right = {};
    DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(rotationMatrix.r[0]));
    return right;
}

#pragma endregion ---------- Transform3D ----------