#include "Character.h"
#include "ImGui/ImGuiCtrl.h"

// ----- コンストラクタ -----
Character::Character(const std::string& filename, const float& scaleFactor)
    : Object(filename, scaleFactor)
{
}

// ----- 更新 -----
void Character::Update(const float& elapsedTime)
{
    Object::Update(elapsedTime);
}

// ----- 描画 -----
void Character::Render(ID3D11PixelShader* psShader)
{
}

// ----- ImGui -----
void Character::DrawDebug()
{
    Object::DrawDebug();

    if (ImGui::TreeNodeEx("Movement", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::DragFloat3("Velocity", &velocity_.x);
        ImGui::DragFloat("Acceleration", &acceleration_);
        ImGui::DragFloat("Deceleration", &deceleration_);

        ImGui::TreePop();
    }
}
