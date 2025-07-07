#include "Object.h"
#include "ImGui/ImGuiCtrl.h"
#include "Resource/ResourceManager.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

// ----- コンストラクタ -----
Object::Object(const std::string& filename, const float& scaleFactor, const std::string& objectName)
    : gltfModel_(ResourceManager::Instance().LoadGltfModel(filename)),
    scaleFactor_(scaleFactor), objectName_(objectName),
    pushCollider_("PushCollider", "head", 1.0f, {}, { 1.0f, 1.0f, 1.0f, 1.0f }),
    hitBox_("HitBox", "head", "Test", 1.0f, {}, { 1.0f, 0.3f, 0.3f, 1.0f }),
    hurtBox_("HurtBox", "head", 1.0f, 1.0f, {}, { 0.3f, 0.3f, 1.0f, 1.0f })
{
}

void Object::Finalize()
{
    nlohmann::json pushColliders;
    for (const auto& dataList : pushColliders_)
    {
        nlohmann::json data;
        data["Name"] = dataList.GetName();
        data["JointName"] = dataList.GetJointName();
        data["Radius"] = dataList.GetRadius();
        data["OffsetPosition"] =
        {
            { "x", dataList.GetOffsetPosition().x },
            { "y", dataList.GetOffsetPosition().y },
            { "z", dataList.GetOffsetPosition().z },
        };
        data["Color"] =
        {
            { "x", dataList.GetColor().x },
            { "y", dataList.GetColor().y },
            { "z", dataList.GetColor().z },
            { "w", dataList.GetColor().w },
        };

        pushColliders["PushCollider"].push_back(data);
    }
    
    nlohmann::json hitBoxes;
    for (const auto& dataList : hitBoxes_)
    {
        nlohmann::json data;
        data["Name"] = dataList.GetName();
        data["JointName"] = dataList.GetJointName();
        data["AttackName"] = dataList.GetAttackName();
        data["Radius"] = dataList.GetRadius();
        data["OffsetPosition"] =
        {
            { "x", dataList.GetOffsetPosition().x },
            { "y", dataList.GetOffsetPosition().y },
            { "z", dataList.GetOffsetPosition().z },
        };
        data["Color"] =
        {
            { "x", dataList.GetColor().x },
            { "y", dataList.GetColor().y },
            { "z", dataList.GetColor().z },
            { "w", dataList.GetColor().w },
        };

        hitBoxes["HitBox"].push_back(data);
    }

    nlohmann::json hurtBoxes;
    for (const auto& dataList : hurtBoxes_)
    {
        nlohmann::json data;
        data["Name"] = dataList.GetName();
        data["JointName"] = dataList.GetJointName();
        data["Radius"] = dataList.GetRadius();
        data["OffsetPosition"] =
        {
            { "x", dataList.GetOffsetPosition().x },
            { "y", dataList.GetOffsetPosition().y },
            { "z", dataList.GetOffsetPosition().z },
        };
        data["Color"] =
        {
            { "x", dataList.GetColor().x },
            { "y", dataList.GetColor().y },
            { "z", dataList.GetColor().z },
            { "w", dataList.GetColor().w },
        };
        data["Damage"] = dataList.GetDamage();

        hurtBoxes["HurtBox"].push_back(data);
    }

    std::ofstream writingFile;
    std::string filepath = "./Resources/JsonParameters/Collision/" + objectName_ + "PushColliders";
    writingFile.open(filepath, std::ios::out);
    writingFile << pushColliders.dump() << std::endl;
    writingFile.close();

    filepath = "./Resources/JsonParameters/Collision/" + objectName_ + "HitBox";
    writingFile.open(filepath, std::ios::out);
    writingFile << hitBoxes.dump() << std::endl;
    writingFile.close();

    filepath = "./Resources/JsonParameters/Collision/" + objectName_ + "HurtBox";
    writingFile.open(filepath, std::ios::out);
    writingFile << hurtBoxes.dump() << std::endl;
    writingFile.close();
}

// ----- 更新 -----
void Object::Update(const float& elapsedTime)
{
    // 回転値制御
    DirectX::XMFLOAT3 rotation = GetTransform()->GetRotation();
    if (rotation.y > DirectX::XM_2PI)   rotation.y -= DirectX::XM_2PI;
    if (rotation.y < 0.0f)              rotation.y += DirectX::XM_2PI;

    // アニメーション更新
    gltfModel_->UpdateAnimation(elapsedTime);

    // ルートモーション更新
    gltfModel_->UpdateRootMotion(scaleFactor_);
}

// ----- 描画 -----
void Object::Render(ID3D11PixelShader* psShader)
{
    gltfModel_->Render(scaleFactor_, psShader);
}

void Object::Render(const DirectX::XMFLOAT4X4& world, ID3D11PixelShader* psShader)
{
    gltfModel_->Render(world, psShader);
}

// ----- ImGui -----
void Object::DrawDebug()
{
    if (ImGui::TreeNodeEx("Collision", ImGuiTreeNodeFlags_Framed))
    {
        if (ImGui::TreeNodeEx("Collision Data List", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("PushCollider List", &isPushColliderListActive_);
            ImGui::Checkbox("HitBox List", &isHitBoxListActive_);
            ImGui::Checkbox("HurtBox List", &isHurtBoxListActive_);

            ImGui::TreePop(); // Collision Data List
        }

        if (isPushColliderListActive_)
        {
            const std::string text = objectName_ + "PushCollider List";
            ImGui::Begin(text.c_str());

            for (int i = 0; i < pushColliders_.size(); ++i)
            {
                const std::string name = pushColliders_.at(i).GetName();
                if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed))
                {
                    if (ImGui::Button("Delete PushCollider"))
                    {
                        pushColliders_.erase(pushColliders_.begin() + i);

                        --i;
                        ImGui::TreePop();
                        continue;
                    }
                    const std::string jointName = "JointName : " + pushColliders_.at(i).GetJointName();
                    float radius = pushColliders_.at(i).GetRadius();
                    DirectX::XMFLOAT3 offsetPosition = pushColliders_.at(i).GetOffsetPosition();

                    ImGui::Text(jointName.c_str());
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputFloat("Radius", &radius);
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputFloat3("OffsetPosition", &offsetPosition.x);

                    ImGui::TreePop(); // pushCollider name
                }
            }

            ImGui::End();
        }

        if (isHitBoxListActive_)
        {
            const std::string text = objectName_ + "HitBox List";
            ImGui::Begin(text.c_str());

            for (int i = 0; i < hitBoxes_.size(); ++i)
            {
                const std::string name = hitBoxes_.at(i).GetName();
                if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed))
                {
                    const std::string jointName = "JointName :" + hitBoxes_.at(i).GetJointName();
                    float radius = hitBoxes_.at(i).GetRadius();
                    DirectX::XMFLOAT3 offsetPosition = hitBoxes_.at(i).GetOffsetPosition();

                    ImGui::Text(jointName.c_str());
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputFloat("Radius", &radius);
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputFloat3("OffsetPosition", &offsetPosition.x);

                    ImGui::TreePop(); // hitBox name
                }
            }

            ImGui::End();
        }

        if (isHurtBoxListActive_)
        {
            const std::string text = objectName_ + "HurtBox List";
            ImGui::Begin(text.c_str());

            for (int i = 0; i < hurtBoxes_.size(); ++i)
            {
                const std::string name = hurtBoxes_.at(i).GetName();
                if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed))
                {
                    const std::string jointName = "JointName" + hurtBoxes_.at(i).GetJointName();
                    float radius = hurtBoxes_.at(i).GetRadius();
                    DirectX::XMFLOAT3 offsetPosition = hurtBoxes_.at(i).GetOffsetPosition();

                    ImGui::Text(jointName.c_str());
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputFloat("Radius", &radius);
                    ImGui::SetNextItemWidth(200);
                    ImGui::InputFloat3("OffsetPosition", &offsetPosition.x);

                    ImGui::TreePop(); // hurtBox name
                }
            }

            ImGui::End();
        }

        ImGui::Text("----- DebugDraw -----");
        ImGui::Checkbox("PushColliders ", &isDebugDrawPushColliders_);
        ImGui::SameLine();
        ImGui::Checkbox("HitBoxes ", &isDebugDrawHitBoxes_);
        ImGui::SameLine();
        ImGui::Checkbox("HurtBoxes ", &isDebugDrawHurtBoxes_);

        ImGui::Checkbox("PushCollider ", &isDebugDrawPushCollider_);
        ImGui::SameLine();
        ImGui::Checkbox("HitBox ", &isDebugDrawHitBox_);
        ImGui::SameLine();
        ImGui::Checkbox("HurtBox ", &isDebugDrawHurtBox_);

        if (ImGui::TreeNodeEx("PushCollider", ImGuiTreeNodeFlags_Framed))
        {
            static char name[128] = "";
            ImGui::InputText("PushCollider Name", name, ARRAYSIZE(name));
            pushCollider_.SetName(name);
            std::vector<std::string> jointNames = gltfModel_->GetJointNames();
            static int currentJointIndex = 0;
            if (ImGui::ListBox("Joint Name", &currentJointIndex,
                [](void* data, int index, const char** outText) {
                    auto* vec = static_cast<std::vector<std::string>*>(data);
                    if (index < 0 || index >= static_cast<int>(vec->size())) return false;
                    *outText = (*vec)[index].c_str();
                    return true;
                }, static_cast<void*>(&jointNames), static_cast<int>(jointNames.size())))
            {
                pushCollider_.SetJointName(jointNames.at(currentJointIndex));
            }
            float radius = pushCollider_.GetRadius();
            ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 100.0f);
            pushCollider_.SetRadius(radius);
            DirectX::XMFLOAT3 offsetPosition = pushCollider_.GetOffsetPosition();
            ImGui::DragFloat3("OffsetPosition", &offsetPosition.x, 0.1f);
            pushCollider_.SetOffsetPosition(offsetPosition);

            if (ImGui::Button("Register", ImVec2(300, 100)))
            {
                ImGui::OpenPopup("PushCollider Register Check");
            }

            if (ImGui::BeginPopupModal("PushCollider Register Check", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Would you like to register??");

                if (ImGui::Button("Yes!", ImVec2(150, 150)))
                {
                    pushCollider_.SetColor({ 0.0f, 0.5f, 0.5f, 1.0f });
                    pushColliders_.emplace_back(pushCollider_);
                    pushCollider_.SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(150, 150)))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::TreePop(); // PushCollider
        }

        if (ImGui::TreeNodeEx("HitBox", ImGuiTreeNodeFlags_Framed))
        {
            static char name[128] = "";
            ImGui::InputText("HitBox Name", name, ARRAYSIZE(name));
            hitBox_.SetName(name);
            std::vector<std::string> jointNames = gltfModel_->GetJointNames();
            static int currentJointIndex = 0;
            if (ImGui::ListBox("Joint Name", &currentJointIndex,
                [](void* data, int index, const char** outText) {
                    auto* vec = static_cast<std::vector<std::string>*>(data);
                    if (index < 0 || index >= static_cast<int>(vec->size())) return false;
                    *outText = (*vec)[index].c_str();
                    return true;
                }, static_cast<void*>(&jointNames), static_cast<int>(jointNames.size())))
            {
                hitBox_.SetJointName(jointNames.at(currentJointIndex));
            }
            static char attackName[128] = "";
            ImGui::InputText("AttackName", attackName, ARRAYSIZE(attackName));
            hitBox_.SetAttackName(attackName);
            float radius = hitBox_.GetRadius();
            ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 100.0f);
            hitBox_.SetRadius(radius);
            DirectX::XMFLOAT3 offsetPosition = hitBox_.GetOffsetPosition();
            ImGui::DragFloat3("OffsetPosition", &offsetPosition.x, 0.1f);
            hitBox_.SetOffsetPosition(offsetPosition);

            if (ImGui::Button("Register", ImVec2(300, 100)))
            {
                ImGui::OpenPopup("HitBox Register Check");
            }

            if (ImGui::BeginPopupModal("HitBox Register Check", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Would you like to register??");

                if (ImGui::Button("Yes!", ImVec2(150, 150)))
                {
                    hitBox_.SetColor({ 0.7f, 0.0f, 0.0f, 1.0f });
                    hitBoxes_.emplace_back(hitBox_);
                    hitBox_.SetColor({ 1.0f, 0.3f, 0.3f, 1.0f });

                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(150, 150)))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::TreePop(); // HitBox
        }

        if (ImGui::TreeNodeEx("HurtBox", ImGuiTreeNodeFlags_Framed))
        {
            static char name[128] = "";
            ImGui::InputText("HurtBox Name", name, ARRAYSIZE(name));
            hurtBox_.SetName(name);
            std::vector<std::string> jointNames = gltfModel_->GetJointNames();
            static int currentJointIndex = 0;
            if (ImGui::ListBox("Joint Name", &currentJointIndex,
                [](void* data, int index, const char** outText) {
                    auto* vec = static_cast<std::vector<std::string>*>(data);
                    if (index < 0 || index >= static_cast<int>(vec->size())) return false;
                    *outText = (*vec)[index].c_str();
                    return true;
                }, static_cast<void*>(&jointNames), static_cast<int>(jointNames.size())))
            {
                hurtBox_.SetJointName(jointNames.at(currentJointIndex));
            }
            float radius = hurtBox_.GetRadius();
            ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 100.0f);
            hurtBox_.SetRadius(radius);
            DirectX::XMFLOAT3 offsetPosition = hurtBox_.GetOffsetPosition();
            ImGui::DragFloat3("OffsetPosition", &offsetPosition.x, 0.1f);
            hurtBox_.SetOffsetPosition(offsetPosition);

            if (ImGui::Button("Register", ImVec2(300, 100)))
            {
                ImGui::OpenPopup("HurtBox Register Check");
            }

            if (ImGui::BeginPopupModal("HurtBox Register Check", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Would you like to register??");

                if (ImGui::Button("Yes!", ImVec2(150, 150)))
                {
                    hurtBox_.SetColor({ 0.0f, 0.0f, 0.7f, 1.0f });
                    hurtBoxes_.emplace_back(hurtBox_);
                    hurtBox_.SetColor({ 0.3f, 0.3f, 1.0f, 1.0f });

                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(150, 150)))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::TreePop(); // HurtBox
        }

        ImGui::TreePop(); // Collision
    }

    gltfModel_->DrawDebug();
}

void Object::DebugRender(DebugRenderer* debugRenderer)
{
    if (isDebugDrawPushCollider_) debugRenderer->DrawSphere(pushCollider_.GetPosition(), pushCollider_.GetRadius(), pushCollider_.GetColor());
    if (isDebugDrawHitBox_) debugRenderer->DrawSphere(hitBox_.GetPosition(), hitBox_.GetRadius(), hitBox_.GetColor());
    if (isDebugDrawHurtBox_) debugRenderer->DrawSphere(hurtBox_.GetPosition(), hurtBox_.GetRadius(), hurtBox_.GetColor());

    if (isDebugDrawPushColliders_)
    {
        for (auto& pushCollider : pushColliders_)
        {
            debugRenderer->DrawSphere(pushCollider.GetPosition(), pushCollider.GetRadius(), pushCollider.GetColor());
        }
    }

    if (isDebugDrawHitBoxes_)
    {
        for (auto& hitBox : hitBoxes_)
        {
            debugRenderer->DrawSphere(hitBox.GetPosition(), hitBox.GetRadius(), hitBox.GetColor());
        }
    }

    if (isDebugDrawHurtBoxes_)
    {
        for (auto& hurtBox : hurtBoxes_)
        {
            debugRenderer->DrawSphere(hurtBox.GetPosition(), hurtBox.GetRadius(), hurtBox.GetColor());
        }
    }
}

// Collision登録
void Object::RegisterCollisionData()
{
    RegisterPushColliders();
    RegisterHitBoxes();
    RegisterHurtBoxes();
}

void Object::UpdateCollisions(const float& elapsedTime)
{
    // 押し出し判定
    UpdatePushColliders();

    // 攻撃判定
    for (auto& hitBox : hitBoxes_)
    {
        DirectX::XMFLOAT3 pos = GetJointPosition(hitBox.GetJointName(), hitBox.GetOffsetPosition());

        hitBox.SetPosition(pos);
    }

    // くらい判定
    for (auto& hurtBox : hurtBoxes_)
    {
        DirectX::XMFLOAT3 pos = GetJointPosition(hurtBox.GetJointName(), hurtBox.GetOffsetPosition());

        hurtBox.SetPosition(pos);
    }

    // デバッグ用
    pushCollider_.SetPosition(GetJointPosition(pushCollider_.GetJointName(), pushCollider_.GetOffsetPosition()));
    hitBox_.SetPosition(GetJointPosition(hitBox_.GetJointName(), hitBox_.GetOffsetPosition()));
    hurtBox_.SetPosition(GetJointPosition(hurtBox_.GetJointName(), hurtBox_.GetOffsetPosition()));
}

void Object::UpdatePushColliders()
{
    for (auto& pushCollider : pushColliders_)
    {
        DirectX::XMFLOAT3 pos = GetJointPosition(pushCollider.GetJointName(), pushCollider.GetOffsetPosition());

        pushCollider.SetPosition(pos);
    }
}

void Object::RegisterPushColliders()
{
    std::string filepath = "./Resources/JsonParameters/Collision/" + objectName_ + "PushColliders";
    std::ifstream ifs(filepath);
    nlohmann::json dataList;
    if (ifs.good()) ifs >> dataList;
    else _ASSERT_EXPR(false, L"Json Asset が見つかりません");

    for (const auto& data : dataList["PushCollider"])
    {
        PushCollider pushCollider =
        {
            data["Name"].get<std::string>(),
            data["JointName"].get<std::string>(),
            data["Radius"].get<float>(),
            {
                data["OffsetPosition"]["x"].get<float>(),
                data["OffsetPosition"]["y"].get<float>(),
                data["OffsetPosition"]["z"].get<float>()
            },
            {
                data["Color"]["x"].get<float>(),
                data["Color"]["y"].get<float>(),
                data["Color"]["z"].get<float>(),
                data["Color"]["w"].get<float>(),
            }
        };

        pushColliders_.emplace_back(pushCollider);
    }
}

void Object::RegisterHitBoxes()
{
    std::string filepath = "./Resources/JsonParameters/Collision/" + objectName_ + "HitBox";
    std::ifstream ifs(filepath);
    nlohmann::json dataList;
    if (ifs.good()) ifs >> dataList;
    else _ASSERT_EXPR(false, L"Json Asset が見つかりません");

    for (const auto& data : dataList["HitBox"])
    {
        HitBox hitBox =
        {
            data["Name"].get<std::string>(),
            data["JointName"].get<std::string>(),
            data["AttackName"].get<std::string>(),
            data["Radius"].get<float>(),
            {
                data["OffsetPosition"]["x"].get<float>(),
                data["OffsetPosition"]["y"].get<float>(),
                data["OffsetPosition"]["z"].get<float>()
            },
            {
                data["Color"]["x"].get<float>(),
                data["Color"]["y"].get<float>(),
                data["Color"]["z"].get<float>(),
                data["Color"]["w"].get<float>(),
            }
        };

        hitBoxes_.emplace_back(hitBox);
    }
}

void Object::RegisterHurtBoxes()
{
    std::string filepath = "./Resources/JsonParameters/Collision/" + objectName_ + "HurtBox";
    std::ifstream ifs(filepath);
    nlohmann::json dataList;
    if (ifs.good()) ifs >> dataList;
    else _ASSERT_EXPR(false, L"Json Asset が見つかりません");

    for (const auto& data : dataList["HurtBox"])
    {
        HurtBox hurtBox =
        {
            data["Name"].get<std::string>(),
            data["JointName"].get<std::string>(),
            data["Radius"].get<float>(),
            data["Damage"].get<float>(),
            {
                data["OffsetPosition"]["x"].get<float>(),
                data["OffsetPosition"]["y"].get<float>(),
                data["OffsetPosition"]["z"].get<float>()
            },
            {
                data["Color"]["x"].get<float>(),
                data["Color"]["y"].get<float>(),
                data["Color"]["z"].get<float>(),
                data["Color"]["w"].get<float>(),
            },
        };

        hurtBoxes_.emplace_back(hurtBox);
    }
}
