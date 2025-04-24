#pragma once
#include "BaseScene.h"
#include "Object/Stage/Stage.h"

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

private:
    std::unique_ptr<Stage> stage_;
};

