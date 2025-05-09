#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define LINEAR_BORDER_BLACK 3
#define LINEAR_BORDER_WHITE 4

SamplerState samplerStates[5] : register(s0);
SamplerComparisonState comparisonSamplerState : register(s5);

struct PSIn
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

cbuffer PostEffectConstants : register(b8)
{
    //float3 colorize_;
    //float exposure_;
    
    float brightness_;
    float contrast_;
    float hue_;
    float saturation_;
}

float3 BrightnessContrast(float3 fragmentColor, float brightness, float contrast)
{
    fragmentColor += brightness;
    if (contrast > 0.0)
    {
        fragmentColor = (fragmentColor - 0.5) / (1.0 - contrast) + 0.5;
    }
    else if (contrast < 0.0)
    {
        fragmentColor = (fragmentColor - 0.5) * (1.0 + contrast) + 0.5;
    }
    return fragmentColor;
}
float3 HueSaturation(float3 fragmentColor, float hue, float saturation)
{
    float angle = hue * 3.14159265;
    float s = sin(angle), c = cos(angle);
    float3 weights = (float3(2.0 * c, -sqrt(3.0) * s - c, sqrt(3.0) * s - c) + 1.0) / 3.0;
    fragmentColor = float3(dot(fragmentColor, weights.xyz), dot(fragmentColor, weights.zxy), dot(fragmentColor, weights.yzx));
    float average = (fragmentColor.r + fragmentColor.g + fragmentColor.b) / 3.0;
    if (saturation > 0.0)
    {
        fragmentColor += (average - fragmentColor) * (1.0 - 1.0 / (1.001 - saturation));
    }
    else
    {
        fragmentColor += (average - fragmentColor) * (-saturation);
    }
    return fragmentColor;
}