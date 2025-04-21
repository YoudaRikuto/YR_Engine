#ifndef __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__
#define __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__
#include "ImageBasedLighting.hlsli"

float3 Fschlick(float3 f0, float3 f90, float VoH)
{
    return f0 + (f90 - f0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}

float Vggx(float NoL, float NoV, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float ggxv = NoL * sqrt(NoV * NoV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float ggxl = NoV * sqrt(NoL * NoL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float ggx = ggxv + ggxl;
    return (ggx > 0.0) ? 0.5 / ggx : 0.0;
}

float Dggx(float NoH, float alphaRoughness)
{
    const float PI = 3.14159265358979;
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    float f = (NoH * NoH) * (alphaRoughnessSq - 1.0) + 1.0;
    return alphaRoughnessSq / (PI * f * f);
}

float3 BrdfLambertian(float3 f0, float3 f90, float3 diffuseColor, float VoH)
{
    const float PI = 3.14159265358979;
    return (1.0 - Fschlick(f0, f90, VoH)) * (diffuseColor / PI);
}

float3 BrdfSpecularGgx(float3 f0, float3 f90, float alphaRoughness,
    float VoH, float NoL, float NoV, float NoH)
{
    float F = Fschlick(f0, f90, VoH);
    float Vis = Vggx(NoL, NoV, alphaRoughness);
    float D = Dggx(NoH, alphaRoughness);

    return F * Vis * D;
}

float3 IblRadianceLambertian(float3 N, float3 V, float roughness, float3 diffuseColor, float3 f0)
{
    float NoV = clamp(dot(N, V), 0.0, 1.0);

    float2 brdfSamplePoint = clamp(float2(NoV, roughness), 0.0, 1.0);
    float2 fAb = SampleLutGgx(brdfSamplePoint).rg;

    float3 irradiance = SampleDiffuseIem(N).rgb;

    float3 fr = max(1.0 - roughness, f0) - f0;
    float3 KS = f0 + fr * pow(1.0 - NoV, 5.0);
    float3 fssEss = KS * fAb.x + fAb.y;

    float ems = (1.0 - (fAb.x + fAb.y));
    float3 fAvg = (f0 + (1.0 - f0) / 21.0);
    float3 fmsEms = ems * fssEss * fAvg / (1.0 - fAvg * ems);
    float3 KD = diffuseColor * (1.0 - fssEss + fmsEms);

    return (fmsEms + KD) * irradiance;
}

float3 IblRadianceGgx(float3 N, float3 V, float roughness, float3 f0)
{
    float NoV = clamp(dot(N, V), 0.0, 1.0);

    float2 brdfSamplePoint = clamp(float2(NoV, roughness), 0.0, 1.0);
    float2 fAb = SampleLutGgx(brdfSamplePoint).rg;

    float3 R = normalize(reflect(-V, N));
    float3 specularLight = SampleSpecularPmrem(R, roughness).rgb;

    float3 fr = max(1.0 - roughness, f0) - f0;
    float3 KS = f0 + fr * pow(1.0 - NoV, 5.0);
    float3 fssEss = KS * fAb.x + fAb.y;

    return specularLight * fssEss;
}

#endif