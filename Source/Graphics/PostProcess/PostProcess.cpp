#include "PostProcess.h"
#include "Graphics/Graphics.h"
#include "ImGui/ImGuiCtrl.h"

PostProcess::PostProcess()
    : cascadedShadowMap_(4096, 4096)
{
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/PostProcessPS.cso", postProcessPS_.GetAddressOf());

    renderer_ = std::make_unique<FullscreenQuad>();

    sceneBuffer_ = std::make_unique<FrameBuffer>(SCREEN_WIDTH, SCREEN_HEIGHT);
    postProcess_ = std::make_unique<FrameBuffer>(SCREEN_WIDTH, SCREEN_HEIGHT);

    postProcessConstants_ = std::make_unique<ConstantBuffer<PostProcessConstants>>();
}

// 開始
void PostProcess::Activate()
{
    sceneBuffer_->Clear();
    sceneBuffer_->Activate();
}

// 終了
void PostProcess::Deactivate()
{
    sceneBuffer_->Deactivate();
}

// 描画実行
void PostProcess::Draw()
{
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_OFF_ZW_OFF);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);
    Graphics::Instance().SetBlendState(Shader::BlendState::None);

    postProcessConstants_->Activate(8);

    kawaseBloom_.Execute(sceneBuffer_->GetColorMap().Get());

    ID3D11ShaderResourceView* shaderResourceViews[] =
    {
        sceneBuffer_->GetColorMap().Get(),
        Graphics::Instance().GetDepthMap().Get(),
        kawaseBloom_.GetColorMap().Get(),
        cascadedShadowMap_.GetDepthMap().Get(),
    };

    postProcess_->Clear();
    postProcess_->Activate();

    renderer_->Draw(shaderResourceViews, 0, _countof(shaderResourceViews), postProcessPS_.Get());

    postProcess_->Deactivate();
}

// ImGui用
void PostProcess::DrawDebug()
{
#if 0
    ImGui::DragFloat("Brightness", &postProcessConstants_->GetData()->brightness_, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat("Contrast", &postProcessConstants_->GetData()->contrast_, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat("Hue", &postProcessConstants_->GetData()->hue_, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat("Saturation", &postProcessConstants_->GetData()->saturation_, 0.01f, -1.0f, 1.0f);
#endif

    // カスケードシャドウマップ
    cascadedShadowMap_.DrawDebug();

    // 川瀬式ブルーム
    kawaseBloom_.DrawDebug();

    // ---------- Scene 描画表示 ----------
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const ImVec2 contentSize = ImGui::GetContentRegionAvail();

    // 1280 x 720 のアスペクト比を保つ
    const float targetAspect = 1280.0f / 720.0f;
    const float windowAspect = contentSize.x / contentSize.y;

    ImVec2 drawSize = {};

    if (windowAspect > targetAspect)
    {
        drawSize.y = contentSize.y;
        drawSize.x = drawSize.y * targetAspect;
    }
    else
    {
        drawSize.x = contentSize.x;
        drawSize.y = drawSize.x / targetAspect;
    }

    // 中央配置
    const ImVec2 cursorPos = ImGui::GetCursorPos();
    const ImVec2 setPosition = ImVec2(cursorPos.x + (contentSize.x - drawSize.x) * 0.5f, cursorPos.y + (contentSize.y - drawSize.y) * 0.5f);
    ImGui::SetCursorPos(setPosition);

    ImGui::Image(reinterpret_cast<ImTextureID>(postProcess_->GetColorMap().Get()), drawSize);
    ImGui::End();
}

void PostProcess::MakeCascadedShadowMap(const DirectX::XMFLOAT4& lightDirection, const UINT& cbSlot, std::function<void()> drawcallback)
{
    cascadedShadowMap_.Clear();
    cascadedShadowMap_.Activate(lightDirection, cbSlot);

    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);
    Graphics::Instance().SetBlendState(Shader::BlendState::None);

    drawcallback();

    cascadedShadowMap_.Deactivate();
}
