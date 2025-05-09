#pragma once
#include <DirectXMath.h>
#include <Effekseer.h>

class Effect
{
public:
    Effect(const char* filename, const std::string& effectName);
    ~Effect() {}

    Effekseer::Handle Play(const DirectX::XMFLOAT3& position, const float& scale = 1.0f, const float& speed = 1.0f);
    void Stop(const Effekseer::Handle& handle);

    void SetPosition(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& position); 
    void SetRotation(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& rotation, const float& angle); 
    void SetScale(const Effekseer::Handle& handle, const float& scale);                   
    void SetSpeed(const Effekseer::Handle& handle, const float& speed);
    void SetColor(const Effekseer::Handle& handle, const DirectX::XMFLOAT4& color);

    void DrawDebug();

public:
    [[nodiscard]] const std::string GetName() const { return name_; }

private:
    Effekseer::EffectRef effekseerEffect_;

    DirectX::XMFLOAT3   position_   = {};
    float               scale_      = 1.0f;
    DirectX::XMFLOAT3   rotate_     = {};
    DirectX::XMFLOAT4   color_      = { 1.0f, 1.0f, 1.0f, 1.0f };
    float               speed_      = 0.0f;
    float               angle_      = 0.0f;
    std::string         name_       = "";
};

