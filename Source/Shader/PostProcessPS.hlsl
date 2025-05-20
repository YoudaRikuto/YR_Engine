#include "PostProcess.hlsli"

Texture2D colorMap : register(t0);
Texture2D depthMap : register(t1);
Texture2D bloomMap : register(t2);
Texture2DArray cascadedShadowMaps : register(t3);

float4 main(PSIn psIn) : SV_TARGET
{
    float4 sampledColor = colorMap.Sample(samplerStates[LINEAR_BORDER_BLACK], psIn.texcoord_);
    float3 color = sampledColor.rgb;
    float alpha = sampledColor.a;
    
    float depthNdc = depthMap.Sample(samplerStates[LINEAR_BORDER_BLACK], psIn.texcoord_).x;
    
    float4 positionNdc;
    // Texture Space -> Ndc
    positionNdc.x = psIn.texcoord_.x * +2 - 1;
    positionNdc.y = psIn.texcoord_.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    
    // Ndc -> View Space
    float4 positionViewSpace = mul(positionNdc, inverseProjection_);
    positionViewSpace = positionViewSpace / positionViewSpace.w;
    
    // Ndx -> World Space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection_);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;
    
    
    
    color = HueSaturation(color, hue_, saturation_);
    color = BrightnessContrast(color, brightness_, contrast_);    
    color += bloomMap.Sample(samplerStates[LINEAR], psIn.texcoord_).rgb;
    
    //color += sampledColor * 0.7;
    
    return float4(color, alpha);
}