#pragma once
#include "BaseScene.h"

class GameScene : public BaseScene
{
public:
    GameScene() {}
    ~GameScene() override {}

    void CreateResource()                   override;
    void Initialize()                       override;
    void Finalize()                         override;
    void Update(const float& elapsedTime)   override;
    void Render()                           override;
    void DrawDebug()                        override;
};

