#include "CutSceneCamera.h"
#include "ImGui/ImGuiCtrl.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Camera.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

// 更新
void CutSceneCamera::Update(const float& elapsedTime)
{
    // カットシーンカメラが有効でない
    if (isCutSceneCameraActive_ == false) return;

    // Targetを更新する
    if (currentCutSceneCameraInfo_.targetName_ == "Player")
    {
        cutSceneCameraInfo_.targetName_ = "Player";

        currentCutSceneCameraInfo_.target_ = PlayerManager::Instance().GetPlayer()->GetJointPosition(currentCutSceneCameraInfo_.targetJointName_);
    }
    else if (currentCutSceneCameraInfo_.targetName_ == "WoodMonster")
    {
        cutSceneCameraInfo_.targetName_ = "WoodMonster";

        currentCutSceneCameraInfo_.target_ = EnemyManager::Instance().GetEnemy(0)->GetJointPosition(currentCutSceneCameraInfo_.targetJointName_);
    }

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

    if (ImGui::TreeNodeEx("DebugRenderer", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::DragFloat("BoxScale", &boxScale_, 0.01f);

        ImGui::DragFloat("SphereForwardLength", &forwardSphereLength_, 0.01f);
        ImGui::DragFloat("SphereRadius", &sphereRadius_, 0.01f);

        ImGui::TreePop();
    }

    // ---------- CutSceneを保存(Json) ----------
    if (ImGui::TreeNodeEx("----- Asset Action -----", ImGuiTreeNodeFlags_Framed))
    {
        static char filename[128] = "";
        ImGui::InputText("Asset Name", filename, ARRAYSIZE(filename));

        if (ImGui::Button("Export CutSceneCamera Asset", ImVec2(300, 50)))
        {
            ImGui::OpenPopup("Export Check");
        }
        if (ImGui::BeginPopupModal("Export Check", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Export CutSceneCamera Asset??");
            
            if (ImGui::Button("Export", ImVec2(150, 150)))
            {
                AssetCreation(filename);

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(150, 150)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }


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
        currentDataIndex_ = 0;
        currentCutSceneCameraInfo_ = cutSceneCamera_.at(0);
        oldCutSceneCameraInfo_ = cutSceneCamera_.at(0);
        lerpTimer_ = currentCutSceneCameraInfo_.lerpTime_;
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

    ImGui::End(); // CutSceneCamera


    ImGui::Begin("CutSceneCameraList"); // CutSceneCameraList

    // --------------- ResourceからListに情報を書き込む　---------------
    if (ImGui::TreeNodeEx("Load Resource", ImGuiTreeNodeFlags_Framed))
    {
        static char fileName[128] = "";
        ImGui::InputText("Asset Name", fileName, ARRAYSIZE(fileName));

        if (ImGui::Button("Load Resouece From File"))
        {
            cutSceneCamera_.clear();

            const std::string filePath = fileDirectory_ + fileName;
            std::ifstream ifs(filePath);

            nlohmann::json cutSceneCameraData;
            if (ifs.good()) ifs >> cutSceneCameraData;
            else _ASSERT_EXPR(false, L"Json Asset が見つかりません");

            for (const auto& data : cutSceneCameraData["CutSceneCameraInfo"])
            {
                CutSceneCameraInfo cutSceneCameraInfo =
                {
                    // ----- Rotation -----
                    {
                        data["Rotation"]["x"].get<float>(),
                        data["Rotation"]["y"].get<float>(),
                        data["Rotation"]["z"].get<float>(),
                    },
                    // ----- Target -----
                    { 0, 0, 0 },
                    // ----- Offset -----
                    {
                        data["Offset"]["x"].get<float>(),
                        data["Offset"]["y"].get<float>(),
                        data["Offset"]["z"].get<float>(),
                    },
                    // ----- Length -----
                    data["Length"].get<float>(),
                    // ----- LerpTime -----
                    data["LerpTime"].get<float>(),
                    // ----- TargetName -----
                    data["TargetName"].get<std::string>(),
                    // ----- TargetJointName -----
                    data["TargetJointName"].get<std::string>(),
                };

                cutSceneCamera_.emplace_back(cutSceneCameraInfo);
            }
        }

        ImGui::TreePop();
    }

    // --------------- CutSceneCameraList表示 ---------------
    for (int cutSceneCameraIndex = 0; cutSceneCameraIndex < cutSceneCamera_.size(); ++cutSceneCameraIndex)
    {
        const std::string name = std::to_string(cutSceneCameraIndex);
        if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed))
        {
            // --------------- 消去処理 ---------------
            if (ImGui::Button("Delete This CameraInfo", ImVec2(300, 50)))
            {
                cutSceneCamera_.erase(cutSceneCamera_.begin() + cutSceneCameraIndex);
                --cutSceneCameraIndex;
                ImGui::TreePop();
                continue;
            }

            CutSceneCameraInfo& cutSceneCameraInfo = cutSceneCamera_.at(cutSceneCameraIndex);
            // --------------- TargetName ---------------
            const std::string targetName = "TargetName : " + cutSceneCameraInfo.targetName_;
            ImGui::Text(targetName.c_str());
            // --------------- TargetJointName ---------------
            const std::string targetJointName = "TargetJointName : " + cutSceneCameraInfo.targetJointName_;
            ImGui::Text(targetJointName.c_str());            
            // --------------- Rotation ---------------
            DirectX::XMFLOAT3 rotation = {};
            rotation.x = DirectX::XMConvertToDegrees(cutSceneCameraInfo.rotation_.x);
            rotation.y = DirectX::XMConvertToDegrees(cutSceneCameraInfo.rotation_.y);
            rotation.z = DirectX::XMConvertToDegrees(cutSceneCameraInfo.rotation_.z);
            ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
            cutSceneCameraInfo.rotation_.x = DirectX::XMConvertToRadians(rotation.x);
            cutSceneCameraInfo.rotation_.y = DirectX::XMConvertToRadians(rotation.y);
            cutSceneCameraInfo.rotation_.z = DirectX::XMConvertToRadians(rotation.z);
            // --------------- Target ---------------
            ImGui::DragFloat3("Target", &cutSceneCameraInfo.target_.x);
            // --------------- Offset ---------------
            ImGui::DragFloat3("Offset", &cutSceneCameraInfo.offset_.x);
            // --------------- Length ---------------
            ImGui::DragFloat("Length", &cutSceneCameraInfo.length_);
            // --------------- LerpTime ---------------
            ImGui::DragFloat("LerpTime", &cutSceneCameraInfo.lerpTime_);

            ImGui::TreePop();
        }
    }

    ImGui::End(); // CutSceneCameraList
}

void CutSceneCamera::DebugRender(DebugRenderer* debugRenderer)
{
    for (int cutSceneCameraIndex = 0; cutSceneCameraIndex < cutSceneCamera_.size(); ++cutSceneCameraIndex)
    {
        CutSceneCameraInfo cutSceneCameraInfo = cutSceneCamera_.at(cutSceneCameraIndex);

        if (cutSceneCameraInfo.targetName_ == "Player")
        {
            cutSceneCameraInfo.target_ = PlayerManager::Instance().GetPlayer()->GetJointPosition(cutSceneCameraInfo.targetJointName_);
        }
        else if (cutSceneCameraInfo.targetName_ == "WoodMonster")
        {
            cutSceneCameraInfo.target_ = EnemyManager::Instance().GetEnemy(0)->GetJointPosition(cutSceneCameraInfo.targetJointName_);
        }

        DirectX::XMFLOAT3 position, forwardVec;
        Camera::Instance().CalcCameraVectorFromCutSceneCameraInfo(cutSceneCameraInfo, position, forwardVec);

        debugRenderer->DrawBox(position, cutSceneCameraInfo.rotation_, { boxScale_, boxScale_, boxScale_ }, { 1,1,1,1 });

        debugRenderer->DrawSphere(position + XMFloat3Normalize(forwardVec) * forwardSphereLength_, sphereRadius_, { 1.0f, 0.3f, 0.3f, 1.0f });
    }
}

// 指定したカットシーンを再生する
void CutSceneCamera::PlayCutScene(const std::string& cutSceneName)
{
    
}

// CutSceneを保存(Json)
void CutSceneCamera::AssetCreation(const std::string& filename)
{
    nlohmann::json cutSceneCameraData;
    for (const auto& cutSceneCamera : cutSceneCamera_)
    {
        nlohmann::json data;

        data["Rotation"] =
        {
            { "x", cutSceneCamera.rotation_.x },
            { "y", cutSceneCamera.rotation_.y },
            { "z", cutSceneCamera.rotation_.z },
        };
        data["Offset"] =
        {
            { "x", cutSceneCamera.offset_.x },
            { "y", cutSceneCamera.offset_.y },
            { "z", cutSceneCamera.offset_.z },
        };
        data["Length"]          = cutSceneCamera.length_;
        data["LerpTime"]        = cutSceneCamera.lerpTime_;
        data["TargetName"]      = cutSceneCamera.targetName_;
        data["TargetJointName"] = cutSceneCamera.targetJointName_;

        cutSceneCameraData["CutSceneCameraInfo"].push_back(data);
    }

    std::ofstream writingFile;
    const std::string filepath = fileDirectory_ + filename;
    writingFile.open(filepath, std::ios::out);
    writingFile << cutSceneCameraData.dump() << std::endl;
    writingFile.close();
}
