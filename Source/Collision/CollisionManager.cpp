#include "CollisionManager.h"
#include "Math/MathHelper.h"
#include "Object/Character/Player/PlayerManager.h"
#include "Object/Character/Enemy/EnemyManager.h"
#include "Object/Character/Enemy/WoodMonster/WoodMonster.h"

// 更新
void CollisionManager::Update(const float& elapsedTime)
{
    // Player(PushCOllider) VS Enemy(PushCollider)
    PlayerPushColliderVsEnemyPushCollider();

    // Player(HitBox) VS Enemy(HurtBox)
    PlayerHitBoxVsEnemyHurtBox();
    
    // Player(HurtBox) VS Enemy(HitBox)
    PlayerHurtBoxVsEnemyHitBox();
}

void CollisionManager::DrawDebug()
{
}

// Player(PushCOllider) VS Enemy(PushCollider)
void CollisionManager::PlayerPushColliderVsEnemyPushCollider()
{
    // 押し出し判定が有効でない
    if (PlayerManager::Instance().GetPlayer()->IsPushColliderActive() == false) return;

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

                // 当たっているか判定する
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

// Player(HitBox) VS Enemy(HurtBox)
void CollisionManager::PlayerHitBoxVsEnemyHurtBox()
{
    // Playerの攻撃判定が有効でない
    if (PlayerManager::Instance().GetPlayer()->IsAttackHitBoxActive() == false) return;

    // Enemyが存在しない
    if (EnemyManager::Instance().GetEnemyCount() <= 0) return;

    const std::vector<HitBox> playerSwordHitBoxes = PlayerManager::Instance().GetSwordHitBoxes();
    
    for (int playerHitBoxIndex = 0; playerHitBoxIndex < playerSwordHitBoxes.size(); ++playerHitBoxIndex)
    {
        const HitBox playerSwordHitBox = playerSwordHitBoxes.at(playerHitBoxIndex);
        const int enemyCount = EnemyManager::Instance().GetEnemyCount();
        for (int enemyIndex = 0; enemyIndex < enemyCount; ++enemyIndex)
        {
            // Enemy情報抽出
            const std::vector<HurtBox> enemyHurtBoxes = EnemyManager::Instance().GetEnemy(enemyIndex)->GetHurtBoxes();
            const EnemyType enemyType = EnemyManager::Instance().GetEnemy(enemyIndex)->GetEnemyType();

            for (int enemyHurtBoxIndex = 0; enemyHurtBoxIndex < enemyHurtBoxes.size(); ++enemyHurtBoxIndex)
            {
                const HurtBox enemyHurtBox = enemyHurtBoxes.at(enemyHurtBoxIndex);

                // 攻撃が当たったかの判定
                if (IntersectSphereVsSphere(
                    playerSwordHitBox.GetPosition(), playerSwordHitBox.GetRadius(),
                    enemyHurtBox.GetPosition(), enemyHurtBox.GetRadius()))
                {
                    // 攻撃判定を無効化する
                    PlayerManager::Instance().GetPlayer()->SetAttackHitBoxActive(false);

                    const Player::STATE playerState = PlayerManager::Instance().GetPlayer()->GetCurrentState();
                    
                    if (enemyType == EnemyType::WoodMonster)
                    {
                        WoodMonster* woodMonster = dynamic_cast<WoodMonster*>(EnemyManager::Instance().GetEnemy(enemyIndex));
                        woodMonster->OnDamage();
                    }

                    // 同じフレームで攻撃は連続して当たらないので終了する
                    return;
                }
            }
        }
    }
}

// Player(HurtBox) VS Enemy(HitBox)
void CollisionManager::PlayerHurtBoxVsEnemyHitBox()
{
    // 敵が存在しない
    const int enemyCount = EnemyManager::Instance().GetEnemyCount();
    if (enemyCount <= 0) return;

    for (int enemyIndex = 0; enemyIndex < enemyCount; ++enemyIndex)
    {
        if (EnemyManager::Instance().GetEnemy(enemyIndex)->GetEnemyType() == EnemyType::WoodMonster)
        {
            WoodMonster* woodMonster = dynamic_cast<WoodMonster*>(EnemyManager::Instance().GetEnemy(enemyIndex));

            // 攻撃判定が有効でない
            if (woodMonster->IsAttackHitBoxActive() == false) continue;

            const std::vector<HitBox> enemyHitBoxes = woodMonster->GetHitBoxes();            
            for (int enemyHitBoxIndex = 0; enemyHitBoxIndex < enemyHitBoxes.size(); ++enemyHitBoxIndex)
            {
                const HitBox enemyHitBox = enemyHitBoxes.at(enemyHitBoxIndex);
                if (enemyHitBox.IsActive() == false) continue;

                const std::vector<HurtBox> playerHurtBoxes = PlayerManager::Instance().GetHurtBoxes();
                for (int playerHurtBoxIndex = 0; playerHurtBoxIndex < playerHurtBoxes.size(); ++playerHurtBoxIndex)
                {
                    const HurtBox playerHurtBox = playerHurtBoxes.at(playerHurtBoxIndex);

                    // 当たったか判定する
                    if (IntersectSphereVsSphere(
                        enemyHitBox.GetPosition(), enemyHitBox.GetRadius(),
                        playerHurtBox.GetPosition(), playerHurtBox.GetRadius()))
                    {
                        // 攻撃判定を無効化
                        woodMonster->SetAttackHitBoxActive(false);

                        // プレイヤー側でダメージを受けたかを判定する
                        if (PlayerManager::Instance().GetPlayer()->OnDamage())
                        {
                            // *** ダメージを受けた ***

                        }
                        else
                        {
                            // *** ガードされた ***

                            woodMonster->ChangeState(WoodMonster::STATE::BlockHitBreak);
                        }

                        // 終了
                        return;
                    }
                }
            }
        }
    }    
}

// Sphere VS Sphere
const bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB)
{
    const DirectX::XMFLOAT3 vec = positionB - positionA;
    const float lengthSq = XMFloat3LengthSq(vec);
    const float range = radiusA + radiusB;

    // 当たっていない
    if (lengthSq > range * range) return false;

    return true;
}

// Sphere VS Sphere ( Y軸押し出しなし )
const bool CollisionManager::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA, const DirectX::XMFLOAT3& positionB, const float& radiusB, DirectX::XMFLOAT3& outPositionB)
{
    const DirectX::XMFLOAT3 vec = positionA - positionB;
    const float length = XMFloat3Length(vec);
    const float range = radiusA + radiusB;

    // 当たっていない
    if (length > range * range) return false;

    const DirectX::XMFLOAT3 horizontalVec = { vec.x, 0.0f, vec.z };
    const float horizontalLength = XMFloat3Length(horizontalVec);
    const float newHorizontalLength = sqrtf(range * range - vec.y * vec.y);
    const float resultLength = newHorizontalLength - horizontalLength;

    outPositionB = XMFloat3Normalize(horizontalVec) * resultLength;

    return true;
}