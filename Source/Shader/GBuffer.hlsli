struct PSGBufferOut
{
    float4 baseColor_ : SV_TARGET0;
    float4 emissiveColor_ : SV_TARGET1;
    float4 normal_ : SV_TARGET2;
    float4 parameter_ : SV_TARGET3;
    float depth_ : SV_TARGET4;
};

struct GBufferData
{
    float4 baseColor_; // ベースカラー
    float3 emissiveColor_; // 自己発行色
    float3 worldNormal_; // ワールド法線
    float3 worldPosition_; // ワールド座標
    float depth_; // 深度
    float roughness_; // 粗さ
    float metalness_; // 金属質
    float occlusionFactor_; // 遮蔽値
    float occlusionStrength_; // 遮蔽強度
};

// GBufferテクスチャ受け渡し用構造体
struct PSGBufferTextures
{
    Texture2D<float4> baseColor_;
    Texture2D<float4> emissiveColor_;
    Texture2D<float4> normal_;
    Texture2D<float4> parameter_;
    Texture2D<float> depth_;
    SamplerState state_;
};

PSGBufferOut EncodeGBuffer(in GBufferData data, matrix viewProjection)
{
    PSGBufferOut ret = (PSGBufferOut) 0;
    
    // ベースカラーは baseColor RT0 に出力
    ret.baseColor_ = data.baseColor_;
    
    // 自己発行色は emissiveColor RT1 に出力
    ret.emissiveColor_.rgb = data.emissiveColor_;
    ret.emissiveColor_.a = 1;
    
    // 法線は normal RT2 に出力
    ret.normal_.rgb = data.worldNormal_;
    ret.normal_.a = 1;
    
    // 粗さ/金属質/遮蔽は parameter RT3 に出力
    ret.parameter_.r = data.occlusionFactor_;
    ret.parameter_.g = data.roughness_;
    ret.parameter_.b = data.metalness_;
    ret.parameter_.a = data.occlusionStrength_;
    
    // 深度は depth RT4 に出力
    float4 position = mul(float4(data.worldPosition_, 1.0f), viewProjection);
    ret.depth_ = position.z / position.w;
    return ret;
}

// ピクセルシェーダーの出力用構造体からGBufferData情報に変換
GBufferData DecodeGBuffer(PSGBufferTextures textures, float2 uv, matrix inverseViewProjection)
{
    // 各テクスチャから情報を取得
    float4 baseColor = textures.baseColor_.Sample(textures.state_, uv);
    float4 emissiveColor = textures.emissiveColor_.Sample(textures.state_, uv);
    float4 normal = textures.normal_.Sample(textures.state_, uv);
    float4 parameter = textures.parameter_.Sample(textures.state_, uv);
    float depth = textures.depth_.Sample(textures.state_, uv);
    
    GBufferData ret = (GBufferData) 0;
    
    ret.baseColor_ = baseColor;
    ret.emissiveColor_ = emissiveColor;
    ret.worldNormal_ = normal.rgb;
    ret.occlusionFactor_ = parameter.r;
    ret.roughness_ = parameter.g;
    ret.metalness_ = parameter.b;
    ret.occlusionStrength_ = parameter.a;
    ret.depth_ = depth;
    float4 position = float4(uv.x * 2.0f - 1.0f, uv.y * -2.0f + 1.0f, depth, 1);
    position = mul(position, inverseViewProjection);
    ret.worldPosition_ = position.xyz / position.w;
    return ret;
}