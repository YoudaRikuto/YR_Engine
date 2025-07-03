#include "CollisionManager.h"
#include "Math/MathHelper.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Object/Character/Enemy/EnemyManager.h"

// çXêV
void CollisionManager::Update(const float& elapsedTime)
{
    const int enemyCount = EnemyManager::Instance().GetEnemyCount();
    for (int enemyIndex = 0; enemyIndex < enemyCount; ++enemyIndex)
    {
        const std::vector<PushCollider> playerPushColliders = PlayerManager::Instance().GetPushColliders();
        const std::vector<PushCollider> enemyPushColliders = EnemyManager::Instance().GetEnemy(enemyIndex)->GetPushColliders();

        for (int playerPushColliderIndex = 0; playerPushColliderIndex < playerPushColliders.size(); ++playerPushColliderIndex)
        {
            const PushCollider playerPushCollider = playerPushColliders.at(playerPushColliderIndex);

            for (int enemyPushColliderIndex = 0; enemyPushColliderIndex < enemyPushColliders.size(); ++enemyPushColliderIndex)
            {
                const PushCollider enemyPushCollider = enemyPushColliders.at(enemyPushColliderIndex);

                PlayerManager::Instance().UpdatePushColliders();

                // ìñÇΩÇ¡ÇƒÇ¢ÇÈÇ©îªíËÇ∑ÇÈ
                DirectX::XMFLOAT3 resultPosition = {};
                if (IntersectSphereVsSphere(
                    enemyPushCollider.GetPosition(), enemyPushCollider.GetRadius(),
                    playerPushCollider.GetPosition(), playerPushCollider.GetRadius(),
                    resultPosition))
                {
                    const DirectX::XMFLOAT3 playerPosition = PlayerManager::Instance().GetTransform()->GetPosition();
                    resultPosition = playerPosition - resultPosition;
                    PlayerManager::Instance().GetTransform()->SetPosition(resultPosition);
                }
            }
        }
    }


    
}

void CollisionManager::DrawDebug()
{
}

// Sphere VS Sphere
const bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB)
{
    const DirectX::XMFLOAT3 vec = positionB - positionA;
    const float lengthSq = XMFloat3LengthSq(vec);
    const float range = radiusA + radiusB;

    // ìñÇΩÇ¡ÇƒÇ¢Ç»Ç¢
    if (lengthSq > range * range) return false;

    return true;
}

// Sphere VS Sphere ( Yé≤âüÇµèoÇµÇ»Çµ )
const bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB, DirectX::XMFLOAT3& outPositionB)
{
    const DirectX::XMFLOAT3 vec = positionA - positionB;
    const float length = XMFloat3Length(vec);
    const float range = radiusA + radiusB;

    // ìñÇΩÇ¡ÇƒÇ¢Ç»Ç¢
    if (length > range * range) return false;

    const DirectX::XMFLOAT3 horizontalVec = { vec.x, 0.0f, vec.z };
    const float horizontalLength = XMFloat3Length(horizontalVec);
    const float newHorizontalLength = sqrtf(range * range - vec.y * vec.y);
    const float resultLength = newHorizontalLength - horizontalLength;

    outPositionB = XMFloat3Normalize(horizontalVec) * resultLength;

    return true;
}