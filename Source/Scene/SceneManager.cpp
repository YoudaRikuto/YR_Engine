#include "SceneManager.h"
#include "Framework/Common.h"
#include "ImGui/ImGuiCtrl.h"

#include "DemoScene.h"

// ----- 初期化 -----
void SceneManager::Initialize()
{
    SceneManager::Instance().ChangeScene(new DemoScene);
}

// ----- 終了化 -----
void SceneManager::Finalize()
{
    Clear();
}

// ----- 更新 -----
void SceneManager::Update(const float& elapsedTime)
{
    // 次のSceneに切り替える
    if (nextScene_)
    {
        // 古いSceneを終了させる
        Clear();

        // 新しいシーンを設定
        currentScene_ = nextScene_;
        nextScene_ = nullptr;

        // Scene初期化処理( マルチスレッド処理をしていない場合に行う )
        if (currentScene_->IsReady() == false)
        {
            currentScene_->CreateResource();
            currentScene_->Initialize();
        }
    }

    // 現在のScene更新処理
    if (currentScene_)
    {
        currentScene_->Update(elapsedTime);
    }
}

// ----- 描画 -----
void SceneManager::Render()
{
    if (currentScene_ == nullptr) return;

    currentScene_->Render();
}

// ----- ImGui -----
void SceneManager::DrawDebug()
{
#ifdef USE_IMGUI
    ImGui::Begin("SceneManager");
    if (ImGui::Button("DemoScene", ImVec2(100, 100)))
    {
        SceneManager::Instance().ChangeScene(new DemoScene);
    }
    ImGui::SameLine();
    if (ImGui::Button("TitleScene", ImVec2(100, 100)))
    {
        //SceneManager::Instance().ChangeScene(new DemoScene);
    }

    ImGui::End();


    if (currentScene_ != nullptr)
    {
        currentScene_->DrawDebug();
    }

#endif// USE_IMGUI
}

// ----- Sceneクリア -----
void SceneManager::Clear()
{
    if (currentScene_ == nullptr) return;

    currentScene_->Finalize();
    SafeDeletePtr(currentScene_);
}

// ----- Scene切り替え -----
void SceneManager::ChangeScene(BaseScene* scene)
{
    nextScene_ = scene;
}
