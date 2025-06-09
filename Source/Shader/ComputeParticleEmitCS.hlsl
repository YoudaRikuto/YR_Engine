#include "ComputeParticle.hlsli"
#include "ComputeParticleEmit.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); // パーティクル管理バッファ
ConsumeStructuredBuffer<uint> particlePoolBuffer : register(u1); // パーティクル番号管理バッファ (末尾から取出専用)
StructuredBuffer<EmitParticleData> emitParticleBuffer : register(t0); // パーティクル生成情報バッファ

Texture2D perlinNoiseTexture : register(t1);

SamplerState samplerstate : register(s0);

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 未使用リストの末尾から未使用パーティクルのインデックスを取得
    uint particleIndex = particlePoolBuffer.Consume();
    uint emitIndex = dispatchThreadId.x;
    
    particleDataBuffer[particleIndex].textureType_ = emitParticleBuffer[emitIndex].textureType_;
    particleDataBuffer[particleIndex].isAlive_ = 1;
    particleDataBuffer[particleIndex].duration_ = emitParticleBuffer[emitIndex].duration_;
    
    particleDataBuffer[particleIndex].lifeTime_ = emitParticleBuffer[emitIndex].lifeTime_;
    particleDataBuffer[particleIndex].lifeTimeConst_ = emitParticleBuffer[emitIndex].lifeTimeConst_;
    particleDataBuffer[particleIndex].startDelay_ = emitParticleBuffer[emitIndex].startDelay_;
    particleDataBuffer[particleIndex].gravity_ = emitParticleBuffer[emitIndex].gravity_;
    
    particleDataBuffer[particleIndex].position_ = emitParticleBuffer[emitIndex].position_;
    particleDataBuffer[particleIndex].velocity_ = emitParticleBuffer[emitIndex].velocity_;
    particleDataBuffer[particleIndex].acceleration_ = emitParticleBuffer[emitIndex].acceleration_;
    particleDataBuffer[particleIndex].rotation_ = emitParticleBuffer[emitIndex].rotation_;
    particleDataBuffer[particleIndex].rotationVelocity_ = emitParticleBuffer[emitIndex].rotationVelocity_;
    particleDataBuffer[particleIndex].rotationAcceleration_ = emitParticleBuffer[emitIndex].rotationAcceleration_;
    particleDataBuffer[particleIndex].scale_ = emitParticleBuffer[emitIndex].scale_;
    particleDataBuffer[particleIndex].scaleVelocity_ = emitParticleBuffer[emitIndex].scaleVelocity_;
    particleDataBuffer[particleIndex].scaleAcceleration_ = emitParticleBuffer[emitIndex].scaleAcceleration_;
    particleDataBuffer[particleIndex].startColor_ = emitParticleBuffer[emitIndex].startColor_;
    particleDataBuffer[particleIndex].endColor_ = emitParticleBuffer[emitIndex].endColor_;
}