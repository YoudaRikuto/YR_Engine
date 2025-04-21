#pragma once
#include "BaseScene.h"

class SceneManager
{
private:
    SceneManager() {}
    ~SceneManager() {}

public:
    static SceneManager& Instance()
    {
        static SceneManager instance;
        return instance;
    }

public:
    void Initialize();                      // 初期化
    void Finalize();                        // 終了化
    void Update(const float& elapsedTime);  // 更新
    void Render();                          // 描画
    void DrawDebug();                       // ImGui

    void Clear();                           // Sceneクリア
    void ChangeScene(BaseScene* scene);     // Scene切り替え

private:
    BaseScene* currentScene_ = nullptr;
    BaseScene* nextScene_ = nullptr;
};

