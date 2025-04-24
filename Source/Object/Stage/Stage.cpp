#include "Stage.h"

// ----- コンストラクタ -----
Stage::Stage()
    : Object("./Resources/Model/Stage/Stage.gltf", 1.0f)
{
    GetTransform()->SetScaleFactor(70.0f);
}

// ----- 描画 -----
void Stage::Render(ID3D11PixelShader* psShader)
{
    Object::Render(psShader);
}

// ----- ImGui -----
void Stage::DrawDebug()
{
    Object::DrawDebug();
}
