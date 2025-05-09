#include "Effect.h"
#include "EffectManager.h"
#include "ImGui/ImGuiCtrl.h"

Effect::Effect(const char* filename, const std::string& effectName)
    : name_(effectName)
{

    char16_t utf16Filename[256];
    Effekseer::ConvertUtf8ToUtf16(utf16Filename, 256, filename);

    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    effekseerEffect_ = Effekseer::Effect::Create(effekseerManager, (EFK_CHAR*)utf16Filename);

    EffectManager::Instance().Register(this);
}

// Ä¶ 
Effekseer::Handle Effect::Play(const DirectX::XMFLOAT3& position, const float& scale, const float& speed)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    Effekseer::Handle handle = effekseerManager->Play(effekseerEffect_, position.x, position.y, position.z);

    effekseerManager->SetScale(handle, scale, scale, scale);
    effekseerManager->SetSpeed(handle, speed);

    return handle;
}

void Effect::Stop(const Effekseer::Handle& handle)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    effekseerManager->StopEffect(handle);
}

// ˆÊ’uÝ’è 
void Effect::SetPosition(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& position)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    const Effekseer::Vector3D pos = Effekseer::Vector3D(position.x, position.y, position.z);
    effekseerManager->SetLocation(handle, pos);
}

// Šp“xÝ’è 
void Effect::SetRotation(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& rotation, const float& angle)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    const Effekseer::Vector3D axis = Effekseer::Vector3D(rotation.x, rotation.y, rotation.z);
    effekseerManager->SetRotation(handle, axis, angle);
}

// ‘å‚«‚³Ý’è 
void Effect::SetScale(const Effekseer::Handle& handle, const float& scale)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    effekseerManager->SetScale(handle, scale, scale, scale);
}

void Effect::SetSpeed(const Effekseer::Handle& handle, const float& speed)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    effekseerManager->SetSpeed(handle, speed);
}

void Effect::SetColor(const Effekseer::Handle& handle, const DirectX::XMFLOAT4& color)
{
    Effekseer::ManagerRef effekseerManager = EffectManager::Instance().GetEffekseerManager();

    Effekseer::Color c = Effekseer::Color(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f, color.w * 255.0f);
    effekseerManager->SetAllColor(handle, c);
}

void Effect::DrawDebug()
{
    if (ImGui::TreeNode(name_.c_str()))
    {
        if (ImGui::Button("Play"))
        {
            Play(position_, scale_, speed_);
        }

        ImGui::DragFloat3("Position", &position_.x);
        ImGui::DragFloat("Scale", &scale_);

        DirectX::XMFLOAT3 rotate = {};
        rotate.x = DirectX::XMConvertToDegrees(rotate_.x);
        rotate.y = DirectX::XMConvertToDegrees(rotate_.y);
        rotate.z = DirectX::XMConvertToDegrees(rotate_.z);

        ImGui::DragFloat3("Rotate", &rotate.x);

        rotate_.x = DirectX::XMConvertToRadians(rotate.x);
        rotate_.y = DirectX::XMConvertToRadians(rotate.y);
        rotate_.z = DirectX::XMConvertToRadians(rotate.z);

        ImGui::ColorEdit4("Color", &color_.x);

        ImGui::DragFloat("Speed", &speed_);

        ImGui::TreePop();
    }
}