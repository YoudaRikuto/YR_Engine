#include "DeferredRendering.h"
#include "Graphics/Graphics.h"

DeferredRendering::DeferredRendering()
{
    renderer_ = std::make_unique<FullscreenQuad>();

    Graphics::Instance().CreatePsFromCso("./Resources/Shader/DeferredRenderingPS.cso", psShader_.GetAddressOf());
}

void DeferredRendering::Draw()
{
    Graphics::Instance().SetBlendState(Shader::BlendState::Alpha);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_OFF_ZW_OFF);

    Graphics::Instance().SetGBufferShaderResourceView();
    renderer_->Draw(Graphics::Instance().GetGBufferBaseColorShaderResourceView(), 0, 1, psShader_.Get());
}
