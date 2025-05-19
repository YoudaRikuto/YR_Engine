#include "WoodMonster.h"

WoodMonster::WoodMonster()
    : Enemy("./Resources/Model/Enemy/WoodMonster/WoodMonster.gltf", 1.0f)
{
}

// 初期化
void WoodMonster::Initialize()
{
}

// 終了化
void WoodMonster::Finalize()
{
}

// 更新
void WoodMonster::Update(const float& elapsedTime)
{
}

// 描画
void WoodMonster::Render(ID3D11PixelShader* psShader)
{
    Object::Render(psShader);
}

// ImGui用
void WoodMonster::DrawDebug()
{
}
