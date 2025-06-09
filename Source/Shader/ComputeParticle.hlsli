#include "SceneConstants.hlsli"

// パーティクルスレッド数
static const int NumParticleThread = 1024;

struct EmitParticleData
{
    int textureType_;
    int isAlive_;
    float duration_;
    float lifeTime_;
    float startDelay_;
    float gravity_;

    float lifeTimeConst_;
    float dummy2_;

    float4 position_;
    float4 velocity_;
    float4 acceleration_;

    float4 rotation_;
    float4 rotationVelocity_;
    float4 rotationAcceleration_;

    float4 scale_;
    float4 scaleVelocity_;
    float4 scaleAcceleration_;
    
    float4 startColor_;
    float4 endColor_;
};

struct ParticleData
{
    int textureType_; 
    int isAlive_; 
    float duration_;
    float lifeTime_;
    float startDelay_;
    float gravity_;

    float lifeTimeConst_;
    float dummy2_;

    float4 position_;
    float4 velocity_;
    float4 acceleration_;

    float4 rotation_;
    float4 rotationVelocity_;
    float4 rotationAcceleration_;

    float4 scale_;
    float4 scaleVelocity_;
    float4 scaleAcceleration_;
    
    float4 startColor_;
    float4 endColor_;
    
    float4 texcoord_;
};

static const uint IndirectArgumentsNumCurrentParticle           = 0;
static const uint IndirectArgumentsNumPraviousParticle          = 4;
static const uint IndirectArgumentsNumDeadParticle              = 8;
static const uint IndirectArgumentsEmitParticleDispatchIndirect = 12;

cbuffer ComputeParticleConstantBuffer : register(b10)
{
    float   deltaTime_;
    uint2   textureSplitCount_;
    uint    systemNumParticles_;
    
    uint totalEmitCount_;
};

struct GSIn
{
    uint vertexId_ : VERTEX_ID;
};

struct PSIn
{
    float4 position_    : SV_POSITION;
    float4 color_       : COLOR;
    float2 texcoord_    : TEXCOORD;
};
