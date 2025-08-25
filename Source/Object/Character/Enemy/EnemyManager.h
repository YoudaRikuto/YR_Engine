#pragma once
#include "Enemy.h"
#include <d3d11.h>
#include <vector>
#include <set>

class EnemyManager
{
private:
    EnemyManager() {}
    ~EnemyManager() {}

public:
    static EnemyManager& Instance()
    {
        static EnemyManager instance;
        return instance;
    }

    void Finalize();
    void Update(const float& elapsedTime);
    void Render(ID3D11PixelShader* psShader = nullptr);
    void RenderUniqueModel();
    void DrawDebug();
    void DebugRender(DebugRenderer* debugRenderer);

    void Register(Enemy* enemy) { generates_.insert(enemy); }
    void Remove(Enemy* enemy) { removes_.insert(enemy); }
    void Clear();

    const int GetEnemyCount() const { return enemies_.size(); }
    Enemy* GetEnemy(const int& index) { return enemies_.at(index); }

    std::vector<Enemy*> GetEnemies() { return enemies_; }

private:
    std::vector<Enemy*> enemies_;
    std::set<Enemy*>    removes_;
    std::set<Enemy*>    generates_;
};

