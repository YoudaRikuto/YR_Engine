#include "GltfModel.hlsli"
Texture2D<float4> noiseMap : register(t10);
SamplerState samplerState : register(s2);

float4 main(PSIn psIn) : SV_TARGET
{
    float alpha = noiseMap.Sample(samplerState, psIn.texcoord_).r;

    return float4(shaderConstantsColor_.rgb, shaderConstantsColor_.a * alpha);
}