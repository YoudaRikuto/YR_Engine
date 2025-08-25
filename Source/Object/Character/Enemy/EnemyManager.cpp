#include "EnemyManager.h"
#include "FrameWork/Common.h"

// 終了化
void EnemyManager::Finalize()
{
    for (Enemy*& enemy : enemies_)
    {
        enemy->Finalize();
    }
    Clear();
}

// 更新
void EnemyManager::Update(const float& elapsedTime)
{
    // 生成処理
    for (Enemy* enemy : generates_)
    {
        enemies_.emplace_back(enemy);
        enemy->Initialize();
    }
    generates_.clear();

    for (Enemy*& enemy : enemies_)
    {
        enemy->Update(elapsedTime);
    }

    for (Enemy* enemy : removes_)
    {
        auto it = std::find(enemies_.begin(), enemies_.end(), enemy);

        if (it != enemies_.end()) enemies_.erase(it);

        SafeDeletePtr(enemy);
    }
    removes_.clear();
}

// 描画
void EnemyManager::Render(ID3D11PixelShader* psShader)
{
    for (Enemy*& enemy : enemies_)
    {
        enemy->Render(psShader);
    }
}

void EnemyManager::RenderUniqueModel()
{
    for (Enemy*& enemy : enemies_)
    {
        enemy->RenderUniqueModel();
    }
}

// ImGui用
void EnemyManager::DrawDebug()
{
    for (Enemy*& enemy : enemies_)
    {
        enemy->DrawDebug();
    }
}

// デバッグ描画 
void EnemyManager::DebugRender(DebugRenderer* debugRenderer)
{
    for (Enemy*& enemy : enemies_)
    {
        enemy->DebugRender(debugRenderer);
    }
}

// 全削除
void EnemyManager::Clear()
{
    for (Enemy*& enemy : enemies_)
    {
        SafeDeletePtr(enemy);
    }
    enemies_.clear();
    enemies_.shrink_to_fit();
}
