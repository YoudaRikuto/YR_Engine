#pragma once
#include "CollisionData.h"
#include <vector>

class CollisionManager
{
private:
    CollisionManager() {}
    ~CollisionManager() {}

public:
    static CollisionManager& Instance()
    {
        static CollisionManager instance;
        return instance;
    }

    void Update(const float& elapsedTime);
    void DrawDebug();

    // Player
    void RegisterPlayerPushCollider(const PushCollider& pushCollider) { playerPushCollider_.emplace_back(pushCollider); }

private:
    // Sphere VS Sphere
    const bool IntersectSphereVsSphere(
        const DirectX::XMFLOAT3& positionA, const float radiusA,
        const DirectX::XMFLOAT3& positionB, const float radiusB);

private:
    std::vector<PushCollider> playerPushCollider_;
    std::vector<PushCollider> enemyPushCollider_;

};

