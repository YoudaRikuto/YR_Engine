#pragma once
#include "Player.h"
#include <memory>

class PlayerManager
{
private:
    PlayerManager() {}
    ~PlayerManager() {}

public:
    static PlayerManager& Instance()
    {
        static PlayerManager instance;
        return instance;
    }
    
    void Initialize();
    void Finalize();
    void Update(const float& elapsedTime);
    void Render(ID3D11PixelShader* psShader = nullptr);
    void DrawDebug();
    void DebugRender(DebugRenderer* debugRenderer);

    Transform3D* GetTransform() { return player_->GetTransform(); }
    std::unique_ptr<Player>& GetPlayer() { return player_; }

    // ---------- Collision ----------
    void UpdatePushColliders() { player_->UpdatePushColliders(); }
    const std::vector<PushCollider> GetPushColliders() const { return player_->GetPushColliders(); }
    const std::vector<HitBox> GetHitBoxes() const { return player_->GetHitBoxes(); }
    const std::vector<HurtBox> GetHurtBoxes() const { return player_->GetHurtBoxes(); }

private:
    std::unique_ptr<Player> player_;
};

