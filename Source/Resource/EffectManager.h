#pragma once
#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include "Effect.h"
#include <vector>
#include <set>

class EffectManager
{
private:
    EffectManager() {}
    ~EffectManager() {}

public:
    static EffectManager& Instance()
    {
        static EffectManager instance;
        return instance;
    }

    void Initialize();
    void Finalize();
    void Update(const float& elapsedTime);
    void Render();
    void DrawDebug();

    Effekseer::ManagerRef GetEffekseerManager() { return effekseerManager_; }

    void Register(Effect* effect);  // ìoò^
    void Remove(Effect* effect);    // çÌèú
    void Clear();                   // ëSçÌèú

    [[nodiscard]] Effect* GetEffect(const std::string& name);

    void StopEffect(const Effekseer::Handle& handle);
    void SetPosition(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& position);
    void AddPosition(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& addPosition);

private:
    Effekseer::ManagerRef           effekseerManager_;
    EffekseerRenderer::RendererRef  effekseerRenderer_;

    std::vector<Effect*>    effects_;
    std::set<Effect*>       generates_;
    std::set<Effect*>       removes_;
};

