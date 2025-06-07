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
    void DeferredRender()                   override;
    void Render()                           override;
    void ShadowRender()                     override;
    void DrawDebug()                        override;

private:
    std::unique_ptr<Stage> stage_;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> iblTextures_[4];
};

