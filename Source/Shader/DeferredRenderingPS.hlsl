#include "DeferredRendering.hlsli"
#include "ShadingFunctions.hlsli"

Texture2D<float4> gBufferBaseColor      : register(t0);
Texture2D<float4> gBufferEmissiveColor  : register(t1);
Texture2D<float4> gBufferNormal         : register(t2);
Texture2D<float4> gBufferParameter      : register(t3);
Texture2D<float>  gBufferDepth          : register(t4);

//  IBL用テクスチャ
Texture2D skybox : register(t32);
TextureCube diffuseIem : register(t33);
TextureCube specularPmrem : register(t34);
Texture2D lutGgx : register(t35);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);

float4 main(PSIn psIn) : SV_TARGET
{
    // GBufferテクスチャから情報をデコードする
    PSGBufferTextures gBufferTextures;
    gBufferTextures.baseColor_ = gBufferBaseColor;
    gBufferTextures.emissiveColor_ = gBufferEmissiveColor;
    gBufferTextures.normal_ = gBufferNormal;
    gBufferTextures.parameter_ = gBufferParameter;
    gBufferTextures.depth_ = gBufferDepth;
    gBufferTextures.state_ = samplerStates[POINT];
    GBufferData data;
    data = DecodeGBuffer(gBufferTextures, psIn.texcoord_, inverseViewProjection_);
    
    float4 albedo = data.baseColor_;
    float3 emissiveColor = data.emissiveColor_;
    float3 N = normalize(data.worldNormal_);
    float3 V = normalize(data.worldPosition_.xyz - cameraPosition_.xyz);
    
    // 入射光農地拡散反射になる割合
    float3 diffuseReflectance = lerp(albedo.rgb, 0.0f, data.metalness_);
    
    // 垂直反射時のフレネル反射率 (非金属でも最低4%は鏡面反射する)
    float3 F0 = lerp(0.04f, albedo.rgb, data.metalness_);
    
    float3 diffuse = 0;
    float3 specular = 0;
    float3 L = normalize(lightDirection_.xyz);
    DirectBDRF(diffuseReflectance, F0, N, V, L, float3(1, 1, 1), data.roughness_, diffuse, specular);
    
    // IBL処理
    diffuse += DiffuseIBL(N, V, data.roughness_, diffuseReflectance, F0, diffuseIem, samplerStates[LINEAR]);
    specular += SpecularIBL(N, V, data.roughness_, F0, lutGgx, specularPmrem, samplerStates[LINEAR]);
    
    // 自己遮蔽
    diffuse = lerp(diffuse, diffuse * data.occlusionFactor_, data.occlusionStrength_);
    specular = lerp(specular, specular * data.occlusionFactor_, data.occlusionStrength_);
        
    // 自己発行色加算
    float3 color = diffuse + specular + emissiveColor;
    
    //  ガンマ係数
    static const float GammaFactor = 2.2f;
    color = pow(color, 1.0f / GammaFactor);
    
    return float4(color, data.baseColor_.a);
    
    //  深度値も出力する
    //PS_OUT output = (PS_OUT) 0;
    //output.color = float4(color, data.baseColor_.a);
    ////output.depth = data.depth_;
    
    //return output;
}