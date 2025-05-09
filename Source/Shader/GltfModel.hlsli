struct VSIn
{
    float4 position_    : POSITION;
    float4 normal_      : NORMAL;
    float4 tangent_     : TANGENT;
    float2 texcoord_    : TEXCOORD;
    uint4  joints_[2]   : JOINTS;
    float4 weights_[2]  : WEIGHTS;
};

struct PSIn
{
    float4 position_        : SV_POSITION;
    float4 worldPosition_   : POSITION;
    float4 worldNormal_     : NORMAL;
    float4 worldTangent_    : TANGENT;
    float2 texcoord_        : TEXCOORD;
};

cbuffer SceneConstantBuffer : register(b0)
{
    row_major float4x4 viewProjection_;
    float4 lightDirection_;
    float4 cameraPosition_;
    row_major float4x4 lightViewProjection_;
};

cbuffer PrimitiveConstantBuffer : register(b1)
{
    row_major float4x4 world_;
    int material_;
    bool hasTangent_;
    int skin_;
    int startInstanceLocation_;
};

static const uint PrimitiveMaxJoints = 512;
cbuffer PrimitiveJointConstants : register(b2)
{
    row_major float4x4 jointMatrices_[PrimitiveMaxJoints];
};

struct TextureInfo
{
    int index_;
    int texcoord_;
};

struct NormalTextureInfo
{
    int index_;
    int texcoord_;
    float scale_;
};

struct OcclusionTextureInfo
{
    int index_;
    int texcoord_;
    float strength_;
};

struct PbrMetallicRoughness
{
    float4 baseColorFactor_;
    TextureInfo baseColorTexture_;
    float metallicFactor_;
    float roughnessFactor_;
    TextureInfo metallicRoughnessTexture_;
};

struct MaterialConstants
{
    float3 emissiveFactor_;
    int alphaMode_;
    float alphaCutoff_;
    bool doubleSided_;
    PbrMetallicRoughness pbrMetallicRoughness_;
    NormalTextureInfo normalTexture_;
    OcclusionTextureInfo occlusionTexture_;
    TextureInfo emissiveTexture_;
};
StructuredBuffer<MaterialConstants> materials : register(t0);