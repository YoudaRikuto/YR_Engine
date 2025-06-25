#pragma once
#include "Math/MathHelper.h"
#include <string>

// âüÇµèoÇµîªíË
class PushCollider
{
public:
    PushCollider(const std::string& name, const std::string& jointName, const float& radius,
        const DirectX::XMFLOAT3& offsetPosition = {}, const DirectX::XMFLOAT4& color = { 1, 1, 1, 1 })
        : name_(name), jointName_(jointName), radius_(radius), offsetPosition_(offsetPosition), color_(color)
    {}
    ~PushCollider() = default;

    const DirectX::XMFLOAT3 GetJointPosition() const { return jointPosition_; }
    const DirectX::XMFLOAT3 GetOffsetPosition() const { return offsetPosition_; }
    const DirectX::XMFLOAT4 GetColor() const { return color_; }

private:
    const std::string   name_;
    const std::string   jointName_;
    float               radius_         = 0.0f;
    DirectX::XMFLOAT3   jointPosition_  = {};
    DirectX::XMFLOAT3   offsetPosition_ = {};
    DirectX::XMFLOAT4   color_          = {};
};

// çUåÇîªíË
class HitBox
{
public:
    HitBox() {}
    ~HitBox() = default;

private:
    const std::string name_;
    const std::string jointName_;
    float radius_ = 0.0f;
    DirectX::XMFLOAT3   jointPosition_ = {};
    DirectX::XMFLOAT3   offsetPosition_ = {};
    DirectX::XMFLOAT4   color_ = {};
    bool isActive_ = false;
};

// Ç≠ÇÁÇ¢îªíË
class HurtBox
{
public:
    HurtBox() {}
    ~HurtBox() = default;

private:
    const std::string name_;
    const std::string jointName_;
    float radius_ = 0.0f;
    DirectX::XMFLOAT3   jointPosition_ = {};
    DirectX::XMFLOAT3   offsetPosition_ = {};
    DirectX::XMFLOAT4   color_ = {};
    
    float damage_ = 0.0f;
    bool isHit_ = false;
    float hitTimer_ = 0.0f;
};