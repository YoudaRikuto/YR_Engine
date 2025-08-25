#pragma once
#include "Object/Character/Character.h"

enum class EnemyType
{
    WoodMonster,
};

class Enemy : public Character
{
public:
    Enemy(const std::string& filename, const float& scaleFactor, const std::string& objectName)
        : Character(filename, scaleFactor, objectName) {}
    ~Enemy() override {}

    virtual const EnemyType GetEnemyType() const = 0;

    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
    virtual void Update(const float& elapsedTime) = 0;
    virtual void Render(ID3D11PixelShader* psShader = nullptr) = 0;
    virtual void RenderUniqueModel() = 0;
    virtual void DrawDebug() = 0;
    virtual void DebugRender(DebugRenderer* debugRenderer) = 0;
};

