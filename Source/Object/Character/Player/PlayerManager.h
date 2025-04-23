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
    void DebugRender();

    Transform3D* GetTransform() { return player_->GetTransform(); }
    std::unique_ptr<Player>& GetPlayer() { return player_; }

private:
    std::unique_ptr<Player> player_;
};

