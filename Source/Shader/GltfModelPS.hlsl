#include "GltfModel.hlsli"
#include "BidirectionalReflectanceDistributionFunction.hlsli"

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4

Texture2D<float4> materialTextures[5] : register(t1);
Texture2D<float4> shadowMap : register(t10);
SamplerComparisonState comparisonSamplerState : register(s5);

float4 main(PSIn psIn) : SV_TARGET
{
    const float GAMMA = 2.2;
	
    const MaterialConstants m = materials[material_];
	
    float4 baseColorFactor = m.pbrMetallicRoughness_.baseColorFactor_;
    const int baseColorTexture = m.pbrMetallicRoughness_.baseColorTexture_.index_;
    if (baseColorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], psIn.texcoord_);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColorFactor *= sampled;
    }
	
    float3 emissiveFactor = m.emissiveFactor_;
    const int emissiveTexture = m.emissiveTexture_.index_;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], psIn.texcoord_);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emissiveFactor *= sampled.rgb;
    }
	
    float roughnessFactor = m.pbrMetallicRoughness_.roughnessFactor_;
    float metallicFactor = m.pbrMetallicRoughness_.metallicFactor_;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness_.metallicRoughnessTexture_.index_;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], psIn.texcoord_);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }

    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture_.index_;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], psIn.texcoord_);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture_.strength_;
	
    const float3 f0 = lerp(0.04, baseColorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(baseColorFactor.rgb, 0.0, metallicFactor);

    const float3 P = psIn.worldPosition_.xyz;
    const float3 V = normalize(cameraPosition_.xyz - psIn.worldPosition_.xyz);

    float3 N = normalize(psIn.worldNormal_.xyz);
    float3 T = hasTangent_ ? normalize(psIn.worldTangent_.xyz) : float3(1, 0, 0);
    float sigma = hasTangent_ ? psIn.worldTangent_.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);

    const int normalTexture = m.normalTexture_.index_;
    if (normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], psIn.texcoord_);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0) - 1.0;
        normalFactor = normalize(normalFactor * float3(m.normalTexture_.scale_, m.normalTexture_.scale_, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }

    float3 diffuse = 0;
    float3 specular = 0;
    
    
    float3 L = normalize(-lightDirection_.xyz);
    float3 Li = float3(1.0, 1.0, 1.0); // Radiance of the light
    const float NoL = max(0.0, dot(N, L));
    const float NoV = max(0.0, dot(N, V));
    if (NoL > 0.0 || NoV > 0.0)
    {
        const float3 R = reflect(-L, N);
        const float3 H = normalize(V + L);

        const float NoH = max(0.0, dot(N, H));
        const float HoV = max(0.0, dot(H, V));

        diffuse += Li * NoL * BrdfLambertian(f0, f90, cDiff, HoV);
        specular += Li * NoL * BrdfSpecularGgx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
    }

    diffuse += IblRadianceLambertian(N, V, roughnessFactor, cDiff, f0);
    specular += IblRadianceGgx(N, V, roughnessFactor, f0);

    float3 emissive = emissiveFactor;
    diffuse = lerp(diffuse, diffuse * occlusionFactor, occlusionStrength);
    specular = lerp(specular, specular * occlusionFactor, occlusionStrength);

    const float emissiveIntencity = 1.0f;
    float3 Lo = diffuse + specular + emissive * emissiveIntencity;
    
    return float4(Lo, baseColorFactor.a);
}