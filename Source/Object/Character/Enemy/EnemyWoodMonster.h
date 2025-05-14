#pragma once
#include "Enemy.h"

class EnemyWoodMonster : public Enemy
{
public:
    EnemyWoodMonster();
    ~EnemyWoodMonster() override {}

    void Initialize()                                   override;
    void Finalize()                                     override;
    void Update(const float& elapsedTime)               override;
    void Render(ID3D11PixelShader* psShader = nullptr)  override;
    void DrawDebug()                                    override;

};

