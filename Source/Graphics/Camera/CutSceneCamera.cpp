#include "CutSceneCamera.h"
#include "ImGui/ImGuiCtrl.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Object/Character/Enemy/EnemyManager.h"

// 更新
void CutSceneCamera::Update(const float& elapsedTime)
{
    // カットシーンカメラが有効でない
    if (isCutSceneCameraActive_ == false) return;

    // 補間タイマーを更新
    lerpTimer_ += elapsedTime;
    if (lerpTimer_ >= currentCutSceneCameraInfo_.lerpTime_)
    {
        ++currentDataIndex_;
        if (currentDataIndex_ < cutSceneCamera_.size())
        {
            oldCutSceneCameraInfo_ = currentCutSceneCameraInfo_;
            currentCutSceneCameraInfo_ = cutSceneCamera_.at(currentDataIndex_);
            lerpTimer_ = 0.0f;
        }
        else
        {
            currentDataIndex_ = 0;
            isCutSceneCameraActive_ = false;
        }
    }

    // Targetを更新する
    if (currentCutSceneCameraInfo_.targetName_ == "Player")
    {
        currentCutSceneCameraInfo_.target_ = PlayerManager::Instance().GetPlayer()->GetJointPosition(currentCutSceneCameraInfo_.targetJointName_);
    }
    else if (currentCutSceneCameraInfo_.targetName_ == "WoodMonster")
    {
        currentCutSceneCameraInfo_.target_ = EnemyManager::Instance().GetEnemy(0)->GetJointPosition(currentCutSceneCameraInfo_.targetJointName_);
    }

    // 更新
    const float weight = lerpTimer_ / currentCutSceneCameraInfo_.lerpTime_;

    cutSceneCameraInfo_.rotation_ = XMFloat3Lerp(oldCutSceneCameraInfo_.rotation_, currentCutSceneCameraInfo_.rotation_, weight);
    cutSceneCameraInfo_.target_ = XMFloat3Lerp(oldCutSceneCameraInfo_.target_, currentCutSceneCameraInfo_.target_, weight);
    cutSceneCameraInfo_.offset_ = XMFloat3Lerp(oldCutSceneCameraInfo_.offset_, currentCutSceneCameraInfo_.offset_, weight);
    cutSceneCameraInfo_.length_ = XMFloatLerp(oldCutSceneCameraInfo_.length_, currentCutSceneCameraInfo_.length_, weight);
}

// ImGui
void CutSceneCamera::DrawDebug()
{
    ImGui::Begin("CutSceneCamera");

    if (ImGui::Button("Register CutSceneCameraInfo", ImVec2(300, 100)))
    {
        ImGui::OpenPopup("CutSceneCameraInfo Register Check");
    }

    if (ImGui::BeginPopupModal("CutSceneCameraInfo Register Check", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Would you like to register??");

        if (ImGui::Button("Yes!", ImVec2(150, 150)))
        {
            cutSceneCamera_.emplace_back(debugCutSceneCameraInfo_);

            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(150, 150)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::Button("Play", ImVec2(300, 50)))
    {
        isCutSceneCameraActive_ = true;

        // 仮
        oldCutSceneCameraInfo_ = cutSceneCamera_.at(0);
    }


    ImGui::DragFloat("Length", &debugCutSceneCameraInfo_.length_, 0.1f);

    DirectX::XMFLOAT3 rotation = {};
    rotation.x = DirectX::XMConvertToDegrees(debugCutSceneCameraInfo_.rotation_.x);
    rotation.y = DirectX::XMConvertToDegrees(debugCutSceneCameraInfo_.rotation_.y);
    rotation.z = DirectX::XMConvertToDegrees(debugCutSceneCameraInfo_.rotation_.z);
    ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
    debugCutSceneCameraInfo_.rotation_.x = DirectX::XMConvertToRadians(rotation.x);
    debugCutSceneCameraInfo_.rotation_.y = DirectX::XMConvertToRadians(rotation.y);
    debugCutSceneCameraInfo_.rotation_.z = DirectX::XMConvertToRadians(rotation.z);

    ImGui::DragFloat3("Target", &debugCutSceneCameraInfo_.target_.x, 0.1f);
    ImGui::DragFloat3("Offset", &debugCutSceneCameraInfo_.offset_.x, 0.1f);
    ImGui::DragFloat("LerpTime", &debugCutSceneCameraInfo_.lerpTime_, 0.1f);
    
    const std::string targetName = "Target Name : " + debugCutSceneCameraInfo_.targetName_;
    ImGui::Text(targetName.c_str());
    if (ImGui::Button("Player")) debugCutSceneCameraInfo_.targetName_ = "Player";
    ImGui::SameLine();
    if (ImGui::Button("WoodMonster")) debugCutSceneCameraInfo_.targetName_ = "WoodMonster";

    if (debugCutSceneCameraInfo_.targetName_ == "Player")
    {
        std::vector<std::string> jointNames = PlayerManager::Instance().GetPlayer()->GetJointNames();
        static int currentJointIndex = 0;
        if (ImGui::ListBox("Target Joint Name", &currentJointIndex,
            [](void* data, int index, const char** outText) {
                auto* vec = static_cast<std::vector<std::string>*>(data);
                if (index < 0 || index >= static_cast<int>(vec->size())) return false;
                *outText = (*vec)[index].c_str();
                return true;
            }, static_cast<void*>(&jointNames), static_cast<int>(jointNames.size())))
        {
            debugCutSceneCameraInfo_.targetJointName_ = jointNames.at(currentJointIndex);
        }
    }
    else if (debugCutSceneCameraInfo_.targetName_ == "WoodMonster")
    {
        std::vector<std::string> jointNames = EnemyManager::Instance().GetEnemy(0)->GetJointNames();
        static int currentJointIndex = 0;
        if (ImGui::ListBox("Target Joint Name", &currentJointIndex,
            [](void* data, int index, const char** outText) {
                auto* vec = static_cast<std::vector<std::string>*>(data);
                if (index < 0 || index >= static_cast<int>(vec->size())) return false;
                *outText = (*vec)[index].c_str();
                return true;
            }, static_cast<void*>(&jointNames), static_cast<int>(jointNames.size())))
        {
            debugCutSceneCameraInfo_.targetJointName_ = jointNames.at(currentJointIndex);
        }
    }

    ImGui::End();
}

// 指定したカットシーンを再生する
void CutSceneCamera::PlayCutScene(const std::string& cutSceneName)
{
    
}
