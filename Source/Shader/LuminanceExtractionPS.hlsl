#include "KawaseBloom.hlsli"

Texture2D colorMap : register(t0);
SamplerState samplerState : register(s0);

float4 main(PSIn psIn) : SV_TARGET
{
    float4 color = colorMap.Sample(samplerState, psIn.texcoord_);
    
    return float4(step(bloomExtractionThreshold_, max(color.r, max(color.g, color.b))) * color.rgb, color.a);
}