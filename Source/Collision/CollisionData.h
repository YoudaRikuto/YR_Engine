#pragma once
#include "Math/MathHelper.h"
#include <string>

// âüÇµèoÇµîªíË
class PushCollider
{
public:
    PushCollider(const std::string& name, const std::string& jointName, const float& radius,
        const DirectX::XMFLOAT3& offsetPosition = {}, const DirectX::XMFLOAT4& color = { 0.0f, 0.5f, 0.5f, 1.0f })
        : name_(name), jointName_(jointName), radius_(radius), offsetPosition_(offsetPosition), color_(color)
    {}
    ~PushCollider() = default;

    const std::string GetName() const { return name_; }
    const std::string GetJointName() const { return jointName_; }
    const float GetRadius() const { return radius_; }
    const DirectX::XMFLOAT3 GetPosition() const { return position_; }
    const DirectX::XMFLOAT3 GetOffsetPosition() const { return offsetPosition_; }
    const DirectX::XMFLOAT4 GetColor() const { return color_; }

    void SetName(const std::string& name) { name_ = name; }
    void SetJointName(const std::string& jointName) { jointName_ = jointName; }
    void SetRadius(const float& radius) { radius_ = radius; }
    void SetPosition(const DirectX::XMFLOAT3& position) { position_ = position; }
    void SetOffsetPosition(const DirectX::XMFLOAT3& offsetPosition) { offsetPosition_ = offsetPosition; }
    void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }

private:
    std::string         name_;
    std::string         jointName_;
    float               radius_         = 0.0f;
    DirectX::XMFLOAT3   position_       = {};
    DirectX::XMFLOAT3   offsetPosition_ = {};
    DirectX::XMFLOAT4   color_          = {};
};

// çUåÇîªíË
class HitBox
{
public:
    HitBox(const std::string& name, const std::string& jointName, const std::string& attackName, const float& radius,
        const DirectX::XMFLOAT3& offsetPosition = {}, const DirectX::XMFLOAT4& color = { 0.7f, 0.0f, 0.0f, 1.0f })
        : name_(name), jointName_(jointName), attackName_(attackName), radius_(radius), offsetPosition_(offsetPosition), color_(color)
    {}
    ~HitBox() = default;

    const std::string GetName() const { return name_; }
    const std::string GetJointName() const { return jointName_; }
    const std::string GetAttackName() const { return attackName_; }
    const float GetRadius() const { return radius_; }
    const DirectX::XMFLOAT3 GetPosition() const { return position_; }
    const DirectX::XMFLOAT3 GetOffsetPosition() const { return offsetPosition_; }
    const DirectX::XMFLOAT4 GetColor() const { return color_; }

    void SetName(const std::string& name) { name_ = name; }
    void SetJointName(const std::string& jointName) { jointName_ = jointName; }
    void SetAttackName(const std::string& attackName) { attackName_ = attackName; }
    void SetRadius(const float& radius) { radius_ = radius; }
    void SetPosition(const DirectX::XMFLOAT3& position) { position_ = position; }
    void SetOffsetPosition(const DirectX::XMFLOAT3& offsetPosition) { offsetPosition_ = offsetPosition; }
    void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }

private:
    std::string         name_;
    std::string         jointName_;
    std::string         attackName_;
    float               radius_         = 0.0f;
    DirectX::XMFLOAT3   position_       = {};
    DirectX::XMFLOAT3   offsetPosition_ = {};
    DirectX::XMFLOAT4   color_          = {};
    bool                isActive_       = false;
};

// Ç≠ÇÁÇ¢îªíË
class HurtBox
{
public:
    HurtBox(const std::string& name, const std::string& jointName, const float& radius,
        const DirectX::XMFLOAT3& offsetPosition = {}, const DirectX::XMFLOAT4& color = { 0.0f, 0.0f, 0.7f, 1.0f})
        : name_(name), jointName_(jointName), radius_(radius), offsetPosition_(offsetPosition), color_(color)
    {}
    ~HurtBox() = default;

    const std::string GetName() const { return name_; }
    const std::string GetJointName() const { return jointName_; }
    const float GetRadius() const { return radius_; }
    const DirectX::XMFLOAT3 GetPosition() const { return position_; }
    const DirectX::XMFLOAT3 GetOffsetPosition() const { return offsetPosition_; }
    const DirectX::XMFLOAT4 GetColor() const { return color_; }

    void SetName(const std::string& name) { name_ = name; }
    void SetJointName(const std::string& jointName) { jointName_ = jointName; }
    void SetRadius(const float& radius) { radius_ = radius; }
    void SetPosition(const DirectX::XMFLOAT3& position) { position_ = position; }
    void SetOffsetPosition(const DirectX::XMFLOAT3& offsetPosition) { offsetPosition_ = offsetPosition; }
    void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }

private:
    std::string         name_;
    std::string         jointName_;
    float               radius_         = 0.0f;
    DirectX::XMFLOAT3   position_       = {};
    DirectX::XMFLOAT3   offsetPosition_ = {};
    DirectX::XMFLOAT4   color_          = {};
    
    float damage_ = 0.0f;
    bool isHit_ = false;
    float hitTimer_ = 0.0f;
};