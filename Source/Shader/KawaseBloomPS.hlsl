#include "KawaseBloom.hlsli"

#define LINEAR_BORDER_BLACK 3

Texture2D colorMap : register(t0);
static const int downsampledCount = 4;
Texture2D downSampleTextures[downsampledCount] : register(t1);
SamplerState samplerStates[5] : register(s0);

float4 main(PSIn psIn) : SV_TARGET
{
    float4 baseColor = colorMap.Sample(samplerStates[LINEAR_BORDER_BLACK], psIn.texcoord_);
    float4 color = { 0, 0, 0, 1 };
    for (int downSampledIndex = 0; downSampledIndex < downsampledCount; ++downSampledIndex)
    {
        color.rgb += downSampleTextures[downSampledIndex].Sample(samplerStates[LINEAR_BORDER_BLACK], psIn.texcoord_).rgb;
    }
    
    return float4(color.rgb * bloomIntencity_, 1.0f);
}