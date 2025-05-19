#pragma once
#include "Object/Character/Enemy/Enemy.h"

class WoodMonster : public Enemy
{
public:
    WoodMonster();
    ~WoodMonster() override {}

    void Initialize()                                   override;
    void Finalize()                                     override;
    void Update(const float& elapsedTime)               override;
    void Render(ID3D11PixelShader* psShader = nullptr)  override;
    void DrawDebug()                                    override;

};

