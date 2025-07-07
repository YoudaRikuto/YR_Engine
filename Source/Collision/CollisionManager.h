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

private:
    void PlayerPushColliderVsEnemyPushCollider();
    void PlayerHitBoxVsEnemyHurtBox();

private:
    // Sphere VS Sphere
    const bool IntersectSphereVsSphere(
        const DirectX::XMFLOAT3& positionA, const float& radiusA,
        const DirectX::XMFLOAT3& positionB, const float& radiusB);

    // Sphere VS Sphere ( âüÇµèoÇµ )
    const bool IntersectSphereVsSphere(
        const DirectX::XMFLOAT3& positionA, const float& radiusA,
        const DirectX::XMFLOAT3& positionB, const float& radiusB,
        DirectX::XMFLOAT3& outPositionB);
};

