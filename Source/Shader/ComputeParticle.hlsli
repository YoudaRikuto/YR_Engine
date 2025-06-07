#include "SceneConstants.hlsli"

// パーティクルスレッド数
static const int NumParticleThread = 1024;

struct EmitParticleData
{
    int emitParticleNum_;
    int textureType_;
    int isAlive_;
    float duration_;
    float lifeTimeMin_;
    float lifeTimeMax_;
    float startDelayMin_;
    float startDelayMax_;

    float gravityMin_;
    float gravityMax_;

    int randomBetweenTwoLifeTimes_;
    int randomBetweenTwostartDelays_;
    int randomBetweenTwoPositions_;
    int randomBetweenTwovelocities_;
    int randomBetweenTwoAccelerations_;
    int randomBetweenTwoGravities_;
    int randomBetweenTwoRotations_;
    int randomBetweenTwoRotationVelocities_;
    int randomBetweenTwoRotationAccelerations_;
    int randomBetweenTwoScales_;
    int randomBetweenTwoScaleVelocities_;
    int randomBetweenTwoScaleAccelerations_;
    int randomBetweenTwoColors_;
        
    int dummy_;

    float4 positionMin_;
    float4 positionMax_;
    float4 velocityMin_;
    float4 velocityMax_;
    float4 accelerationMin_;
    float4 accelerationMax_;

    float4 rotationMin_;
    float4 rotationMax_;
    float4 rotationVelocityMin_;
    float4 rotationVelocityMax_;
    float4 rotationAccelerationMin_;
    float4 rotationAccelerationMax_;

    float4 scaleMin_;
    float4 scaleMax_;
    float4 scaleVelocityMin_;
    float4 scaleVelocityMax_;
    float4 scaleAccelerationMin_;
    float4 scaleAccelerationMax_;

    float4 startColorMin_;
    float4 startColorMax_;
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

    float dummy1_;
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
