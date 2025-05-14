#include "EnemyWoodMonster.h"

EnemyWoodMonster::EnemyWoodMonster()
    : Enemy("./Resources/Model/WoodMonster/WoodMonster.gltf", 1.0f)
{
}

// 初期化
void EnemyWoodMonster::Initialize()
{
}

// 終了化
void EnemyWoodMonster::Finalize()
{
}

// 更新
void EnemyWoodMonster::Update(const float& elapsedTime)
{
}

// 描画
void EnemyWoodMonster::Render(ID3D11PixelShader* psShader)
{
}

// ImGui用
void EnemyWoodMonster::DrawDebug()
{
}
