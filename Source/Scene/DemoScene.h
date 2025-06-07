#pragma once
#include "BaseScene.h"
#include <memory>
#include "Resource/GltfModel/GltfModel.h"

class DemoScene : public BaseScene
{
public:
    DemoScene() {}
    ~DemoScene() {}

    void CreateResource()                   override; // リソース生成
    void Initialize()                       override; // 初期化
    void Finalize()                         override; // 終了化
    void Update(const float& elapsedTime)   override; // 更新
    void DeferredRender()                   override;
    void Render()                           override; // 描画
    void ShadowRender()                     override; // 影書き込み
    void DrawDebug()                        override; // ImGui

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> iblTextures_[4];

    std::unique_ptr<GltfModel> gltfModel_;
};