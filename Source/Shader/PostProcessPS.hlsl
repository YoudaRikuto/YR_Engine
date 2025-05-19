#include "PostProcess.hlsli"

Texture2D colorMap : register(t0);
//Texture2D depthMap : register(t1);
Texture2D bloomMap : register(t1);
Texture2DArray cascadedShadowMaps : register(t2);

float4 main(PSIn psIn) : SV_TARGET
{
    float4 sampledColor = colorMap.Sample(samplerStates[LINEAR_BORDER_BLACK], psIn.texcoord_);
    float3 color = sampledColor.rgb;
    float alpha = sampledColor.a;
	
    color = HueSaturation(color, hue_, saturation_);
    color = BrightnessContrast(color, brightness_, contrast_);    
    color += bloomMap.Sample(samplerStates[LINEAR], psIn.texcoord_).rgb;
    
    //color += sampledColor * 0.7;
    
    return float4(color, alpha);
}