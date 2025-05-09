#include "KawaseBloom.h"
#include "Graphics/Graphics.h"
#include "ImGui/ImGuiCtrl.h"

KawaseBloom::KawaseBloom()
{
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/LuminanceExtractionPS.cso", luminancePS_.GetAddressOf());
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/GaussianBlurHorizontalPS.cso", gaussianBlurHorizontalPS_.GetAddressOf());
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/GaussianBlurVerticalPS.cso", gaussianBlurVerticalPS_.GetAddressOf());
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/KawaseBloomPS.cso", kawaseBloomPS_.GetAddressOf());

    renderer_ = std::make_unique<FullscreenQuad>();

    const uint32_t width  = SCREEN_WIDTH;
    const uint32_t height = SCREEN_HEIGHT;

    luminanceExtraction_    = std::make_unique<FrameBuffer>(width, height);
    kawaseBloom_            = std::make_unique<FrameBuffer>(width, height);

    int shiftNum = 1;
    for (int downSamplingIndex = 0; downSamplingIndex < maxBlurBufferCount_; downSamplingIndex += 2)
    {
        // 横ダウンサンプリング
        gaussianBlur_[downSamplingIndex] = std::make_unique<FrameBuffer>(width >> shiftNum, height >> max(0, shiftNum - 1));

        // 縦ダウンサンプリング
        gaussianBlur_[downSamplingIndex + 1] = std::make_unique<FrameBuffer>(width >> shiftNum, height >> shiftNum);

        ++shiftNum;
    }

    bloomConstants_ = std::make_unique<ConstantBuffer<BloomConstants>>();
}

// 実行
void KawaseBloom::Execute(ID3D11ShaderResourceView* colorMap)
{
    // 定数バッファー更新
    bloomConstants_->Activate(1);

    // 輝度抽出
    luminanceExtraction_->Clear();
    luminanceExtraction_->Activate();
    renderer_->Draw(&colorMap, 0, 1, luminancePS_.Get());
    luminanceExtraction_->Deactivate();

    // ダウンサンプリング
    for (int downSamplingIndex = 0; downSamplingIndex < maxBlurBufferCount_; downSamplingIndex += 2)
    {
        // 横方向 ダウンサンプリング
        gaussianBlur_[downSamplingIndex]->Clear();
        gaussianBlur_[downSamplingIndex]->Activate();
        renderer_->Draw(luminanceExtraction_->GetColorMap().GetAddressOf(), 0, 1, gaussianBlurHorizontalPS_.Get());
        gaussianBlur_[downSamplingIndex]->Deactivate();

        // 縦方向 ダウンサンプリング
        gaussianBlur_[downSamplingIndex + 1]->Clear();
        gaussianBlur_[downSamplingIndex + 1]->Activate();
        renderer_->Draw(gaussianBlur_[downSamplingIndex]->GetColorMap().GetAddressOf(), 0, 1, gaussianBlurVerticalPS_.Get());
        gaussianBlur_[downSamplingIndex + 1]->Deactivate();
    }

    // 川瀬式ブルーム実行
    ID3D11ShaderResourceView* shaderResourceViews[] =
    {
        colorMap,
        gaussianBlur_[1]->GetColorMap().Get(),
        gaussianBlur_[3]->GetColorMap().Get(),
        gaussianBlur_[5]->GetColorMap().Get(),
        gaussianBlur_[7]->GetColorMap().Get(),
    };
    kawaseBloom_->Clear();
    kawaseBloom_->Activate();
    renderer_->Draw(shaderResourceViews, 0, 5, kawaseBloomPS_.Get());
    kawaseBloom_->Deactivate();
}

// ImGui用
void KawaseBloom::DrawDebug()
{
    ImGui::Begin("KawaseBloom");
    
    if (ImGui::TreeNodeEx("Constants", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::DragFloat("ExtractionThreshold", &bloomConstants_->GetData()->bloomExtractionThreshold_, 0.01f, 0.0f, 3.0f);
        ImGui::DragFloat("Intencity", &bloomConstants_->GetData()->bloomIntensity_, 0.01f, 0.0f, 1.0f);

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(luminanceExtraction_->GetColorMap().Get()), ImVec2(512.0f, 512.0f));
        ImGui::Image(reinterpret_cast<ImTextureID>(gaussianBlur_[1]->GetColorMap().Get()), ImVec2(512.0f, 512.0f));
        ImGui::Image(reinterpret_cast<ImTextureID>(gaussianBlur_[3]->GetColorMap().Get()), ImVec2(512.0f, 512.0f));
        ImGui::Image(reinterpret_cast<ImTextureID>(gaussianBlur_[5]->GetColorMap().Get()), ImVec2(512.0f, 512.0f));
        ImGui::Image(reinterpret_cast<ImTextureID>(gaussianBlur_[7]->GetColorMap().Get()), ImVec2(512.0f, 512.0f));
        ImGui::Image(reinterpret_cast<ImTextureID>(kawaseBloom_->GetColorMap().Get()), ImVec2(512.0f, 512.0f));

        ImGui::TreePop();
    }
    
    ImGui::End();
}
