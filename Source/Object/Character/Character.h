#pragma once
#include "Object/Object.h"
#include "Math/MathHelper.h"

class Character : public Object
{
public:
    Character(const std::string& filename, const float& scaleFactor, const std::string& objectName);
    virtual ~Character() {}

    virtual void Update(const float& elapsedTime);
    virtual void Render(ID3D11PixelShader* psShader) = 0;
    virtual void DrawDebug();

public:
    // ---------- Movement ----------
    void SetVelocity(const DirectX::XMFLOAT3& velocity) { velocity_ = velocity; }
    const DirectX::XMFLOAT3 GetVelocity() const { return velocity_; }
    void SetAcceleration(const float& acceleration) { acceleration_ = acceleration; }
    const float GetAcceleration() const { return acceleration_; }
    void SetDeceleration(const float& deceleration) { deceleration_ = deceleration; }
    const float GetDeceleration() const { return deceleration_; }
    void SetMaxSpeed(const float& speed) { maxSpeed_ = speed; }
    const float GetMaxSpeed() const { return maxSpeed_; }

    // ---------- Rotation ----------
    void SetRotationSpeed(const float& speed) { rotationSpeed_ = speed; }
    const float GetRotationSpeed() const { return rotationSpeed_; }

private:
    // ---------- Movement ----------
    DirectX::XMFLOAT3   velocity_       = {};   // 速度
    float               acceleration_   = 0.0f; // 加速度
    float               deceleration_   = 0.0f; // 減速
    float               maxSpeed_       = 0.0f; // 最大速度

    // ---------- Rotation ----------
    float rotationSpeed_ = 1.0f; // 回転速度

};

