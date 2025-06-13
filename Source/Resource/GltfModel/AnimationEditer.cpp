#include "AnimationEditer.h"
#include "Graphics/Graphics.h"
#include "ImGui/ImGuiCtrl.h"

AnimationEditer::AnimationEditer()
{
    frameBuffer_ = std::make_unique<FrameBuffer>(SCREEN_WIDTH, SCREEN_HEIGHT);

    stage_ = std::make_unique<Object>("./Resources/Model/Stage/Stage.gltf", 1.0f);
    stage_->GetTransform()->SetScaleFactor(70.0f);
}

// 更新
void AnimationEditer::Update(const float& elapsedTime)
{
    // カメラ更新
    SetPerspectiveFov();
}

// 描画
void AnimationEditer::Render()
{
    // シーン定数を上書きする
    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->viewProjection_, DirectX::XMLoadFloat4x4(&view_) * DirectX::XMLoadFloat4x4(&projection_));
    sceneConstants_.GetData()->cameraPosition_ = { eye_.x, eye_.y, eye_.z, 0 };

    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->inverseProjection_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&projection_)));
    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->inverseViewProjection_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view_) * DirectX::XMLoadFloat4x4(&projection_)));
    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->inverseView_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view_)));

    sceneConstants_.Activate(0);

    // フレームバッファーに対して書き込みする
    frameBuffer_->Clear();
    frameBuffer_->Activate();

    skyMap_.Draw(2);

    stage_->Render();

    frameBuffer_->Deactivate();
}

// ImGui用
void AnimationEditer::DrawDebug()
{
    ImGui::Begin("Animation Editer");

    skyMap_.DrawDebug();

    if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::DragFloat3("Target", &target_.x);
        ImGui::DragFloat3("Offset", &offset_.x);
        ImGui::DragFloat("Length", &length_, 0.1f, 0.2f, 100.0f);

        transform_.DrawDebug();

        ImGui::TreePop();
    }

    ImGui::End();

    ImGui::Begin("Animation Editer Scene");

    const ImVec2 contentSize = ImGui::GetContentRegionAvail();

    // 1280 x 720 のアスペクト比を保つ
    const float targetAspect = 1280.0f / 720.0f;
    const float windowAspect = contentSize.x / contentSize.y;
    ImVec2 drawSize = {};

    if (windowAspect > targetAspect)
    {
        drawSize.y = contentSize.y;
        drawSize.x = drawSize.y * targetAspect;
    }
    else
    {
        drawSize.x = contentSize.x;
        drawSize.y = drawSize.x / targetAspect;
    }

    // 中央配置
    const ImVec2 cursorPos = ImGui::GetCursorPos();
    const ImVec2 setPosition = ImVec2(cursorPos.x + (contentSize.x - drawSize.x) * 0.5f, cursorPos.y + (contentSize.y - drawSize.y) * 0.5f);
    ImGui::SetCursorPos(setPosition);

    ImGui::Image(reinterpret_cast<ImTextureID>(frameBuffer_->GetColorMap().Get()), drawSize);

    ImGui::End();
}

// カメラ更新
void AnimationEditer::SetPerspectiveFov()
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
