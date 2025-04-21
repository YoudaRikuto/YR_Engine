#pragma once
#include <d3d11.h>

class BaseScene
{
public:
    BaseScene() {}
    virtual ~BaseScene() {}

    virtual void CreateResource()                   = 0; // リソース生成
    virtual void Initialize()                       = 0; // 初期化
    virtual void Finalize()                         = 0; // 終了化
    virtual void Update(const float& elapsedTime)   = 0; // 更新
    virtual void Render()                           = 0; // 描画
    virtual void DrawDebug()                        = 0; // ImGui

    [[nodiscard]] const bool IsReady() const { return ready_; }
    void SetReady() { ready_ = true; }

private:
    bool ready_ = false; // 準備完了フラグ
};

