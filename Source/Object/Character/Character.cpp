#include "Character.h"
#include "ImGui/ImGuiCtrl.h"

Character::Character(const std::string& filename, const float& scaleFactor)
    : Object(filename, scaleFactor)
{
}

// çXêV 
void Character::Update(const float& elapsedTime)
{
    Object::Update(elapsedTime);
}

// ï`âÊ 
void Character::Render(ID3D11PixelShader* psShader)
{
}

// ImGui 
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
