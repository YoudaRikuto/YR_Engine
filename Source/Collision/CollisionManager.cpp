#include "CollisionManager.h"
#include "Math/MathHelper.h"

// XV
void CollisionManager::Update(const float& elapsedTime)
{
}

void CollisionManager::DrawDebug()
{
}

// Sphere VS Sphere
const bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float radiusA, const DirectX::XMFLOAT3& positionB, const float radiusB)
{
    const DirectX::XMFLOAT3 vec = positionB - positionA;
    const float lengthSq = XMFloat3LengthSq(vec);
    const float range = radiusA + radiusB;

    // “–‚½‚Á‚Ä‚¢‚È‚¢
    if (lengthSq > range * range) return false;

    return true;
}
