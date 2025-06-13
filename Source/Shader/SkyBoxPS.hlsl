#include "SkyMap.hlsli"
#include "SceneConstants.hlsli"

SamplerState samplerState[3] : register(s0);
TextureCube skyBox : register(t0);

float4 main(PSIn psIn) : SV_TARGET
{
    float4 R = mul(float4((psIn.texcoord_.x * 2.0) - 1.0, 1.0 - (psIn.texcoord_.y * 2.0), 1.0, 1.0), sceneData_.inverseViewProjection_);
    R /= R.w;
    
    const float lod = 0;
    
    return skyBox.SampleLevel(samplerState[1], R.xyz, lod);
}