#include "Object.h"

// ----- コンストラクタ -----
Object::Object(const std::string& filename, const float& scaleFactor)
    : gltfModel_(filename), scaleFactor_(scaleFactor)
{
}

// ----- 更新 -----
void Object::Update(const float& elapsedTime)
{
    // 回転値制御
    DirectX::XMFLOAT3 rotation = GetTransform()->GetRotation();
    if (rotation.y > DirectX::XM_2PI)   rotation.y -= DirectX::XM_2PI;
    if (rotation.y < 0.0f)              rotation.y += DirectX::XM_2PI;

    // アニメーション更新
    gltfModel_.UpdateAnimation(elapsedTime);

    // ルートモーション更新
    gltfModel_.UpdateRootMotion(scaleFactor_);
}

// ----- 描画 -----
void Object::Render(ID3D11PixelShader* psShader)
{
    gltfModel_.Render(scaleFactor_, psShader);
}

void Object::Render(const DirectX::XMFLOAT4X4& world, ID3D11PixelShader* psShader)
{
    gltfModel_.Render(world, psShader);
}

// ----- ImGui -----
void Object::DrawDebug()
{
    gltfModel_.DrawDebug();
}