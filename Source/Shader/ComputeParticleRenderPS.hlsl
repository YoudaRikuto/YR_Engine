#include "ComputeParticle.hlsli"

Texture2D colorMap : register(t0);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);

float4 main(PSIn psIn) : SV_TARGET0
{
    float4 color = colorMap.Sample(samplerStates[ANISOTROPIC], psIn.texcoord_) * psIn.color_;
    //color.rgb *= colorIntensity_;
    
    return color;
}