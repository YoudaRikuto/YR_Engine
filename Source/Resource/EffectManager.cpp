#include "EffectManager.h"
#include "Graphics/Graphics.h"
#include "Graphics/Camera/Camera.h"
#include "FrameWork/Common.h"
#include "ImGui/ImGuiCtrl.h"

// 初期化 
void EffectManager::Initialize()
{
    Graphics& graphics = Graphics::Instance();

    // Effekseerレンダラ生成
    effekseerRenderer_ = EffekseerRendererDX11::Renderer::Create(graphics.GetDevice(),
        graphics.GetDeviceContext(), 2048);

    // Effekseerマネージャー生成
    effekseerManager_ = Effekseer::Manager::Create(2048);

    // Effekseerレンダラの各種設定(特別なカスタマイズをしない場合は定型的に以下の設定でOK)
    effekseerManager_->SetSpriteRenderer(effekseerRenderer_->CreateSpriteRenderer());
    effekseerManager_->SetRibbonRenderer(effekseerRenderer_->CreateRibbonRenderer());
    effekseerManager_->SetRingRenderer(effekseerRenderer_->CreateRingRenderer());
    effekseerManager_->SetTrackRenderer(effekseerRenderer_->CreateTrackRenderer());
    effekseerManager_->SetModelRenderer(effekseerRenderer_->CreateModelRenderer());
    // Effekseer内でのローダーの設定(特別なカスタマイズをしない場合は以下の設定でOK)
    effekseerManager_->SetTextureLoader(effekseerRenderer_->CreateTextureLoader());
    effekseerManager_->SetModelLoader(effekseerRenderer_->CreateModelLoader());
    effekseerManager_->SetMaterialLoader(effekseerRenderer_->CreateMaterialLoader());

    // Effekseerを左手亜座標系で計算する
    effekseerManager_->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

// 終了化 
void EffectManager::Finalize()
{
    Clear();
}

// 更新
void EffectManager::Update(const float& elapsedTime)
{
    // -------------------------
    //  生成
    // -------------------------
    for (Effect* effect : generates_)
    {
        effects_.emplace_back(effect);
    }
    generates_.clear();

    // エフェクト更新処理(引数にはフレームの通過時間を渡す)
    effekseerManager_->Update(elapsedTime * 60.0f);

    // -------------------------
    //  破棄
    // -------------------------
    for (Effect* effect : removes_)
    {
        auto it = std::find(effects_.begin(), effects_.end(), effect);

        if (it != effects_.end())
        {
            effects_.erase(it);
        }

        SafeDeletePtr(effect);
    }
    removes_.clear();
}

// 描画 
void EffectManager::Render()
{
    DirectX::XMFLOAT4X4 view        = Camera::Instance().GetView();
    DirectX::XMFLOAT4X4 projection  = Camera::Instance().GetProjection();

    // view,projection Matrixを EffekseerRendererに設定
    effekseerRenderer_->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
    effekseerRenderer_->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

    // Effekseer描画開始
    effekseerRenderer_->BeginRendering();

    // Effekseer描画実行
    // マネージャー単位で描画するので描画順を制御する場合はマネージャーを複数個制作し、
    // Draw()関数を実行する順序で制御できそう
    effekseerManager_->Draw();

    // Effekseer描画終了
    effekseerRenderer_->EndRendering();
}

// ImGui用 
void EffectManager::DrawDebug()
{
    if (ImGui::BeginMenu("EffectManager"))
    {
        for (Effect*& effect : effects_)
        {
            effect->DrawDebug();
        }

        ImGui::EndMenu();
    }
}

// 登録 
void EffectManager::Register(Effect* effect)
{
    generates_.insert(effect);
}

// 削除 
void EffectManager::Remove(Effect* effect)
{
    removes_.insert(effect);
}

// 全削除 
void EffectManager::Clear()
{
    for (Effect*& effect : effects_)
    {
        SafeDeletePtr(effect);
    }
    effects_.clear();
    effects_.shrink_to_fit();
}

// エフェクト取得 
Effect* EffectManager::GetEffect(const std::string& name)
{
    for (Effect*& effect : effects_)
    {
        if (effect->GetName() == name)
        {
            return effect;
        }
    }

    return nullptr;
}

void EffectManager::StopEffect(const Effekseer::Handle& handle)
{
    effekseerManager_->StopEffect(handle);
}

void EffectManager::SetPosition(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& position)
{
    effekseerManager_->SetLocation(handle, position.x, position.y, position.z);
}

void EffectManager::AddPosition(const Effekseer::Handle& handle, const DirectX::XMFLOAT3& addPosition)
{
    effekseerManager_->AddLocation(handle, Effekseer::Vector3D(addPosition.x, addPosition.y, addPosition.z));
}
