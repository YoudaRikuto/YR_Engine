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

};

// Ç≠ÇÁÇ¢îªíË
class HurtBox
{
public:
    HurtBox() {}
    ~HurtBox() = default;

private:

};