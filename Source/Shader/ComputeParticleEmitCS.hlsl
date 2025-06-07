#include "ComputeParticle.hlsli"
#include "ComputeParticleEmit.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); // パーティクル管理バッファ
ConsumeStructuredBuffer<uint> particlePoolBuffer : register(u1); // パーティクル番号管理バッファ (末尾から取出専用)
StructuredBuffer<EmitParticleData> emitParticleBuffer : register(t0); //パーティクル生成情報バッファ

Texture2D perlinNoiseTexture : register(t1);

SamplerState samplerstate : register(s0);

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // 未使用リストの末尾から未使用パーティクルのインデックスを取得
    uint particleIndex = particlePoolBuffer.Consume();
    uint emitIndex = dispatchThreadId.x;
    
    // パーリンノイズを用いてランダムな値を取得する
    uint textureWidth = 512;    
    float2 uv = float2((float) dispatchThreadId.x / textureWidth, 0.5);
    float random0 = perlinNoiseTexture.SampleLevel(samplerstate, uv, 0).r;
    
    // --------------- 詳細設定 ---------------
    
    // エフェクト生存時間
    particleDataBuffer[particleIndex].duration_ = emitParticleBuffer[emitIndex].duration_;
    
    // 粒子生存時間
    particleDataBuffer[particleIndex].lifeTime_ = emitParticleBuffer[emitIndex].lifeTimeMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoLifeTimes_ > 0)
    {
        particleDataBuffer[particleIndex].lifeTime_ = lerp(emitParticleBuffer[emitIndex].lifeTimeMin_, emitParticleBuffer[emitIndex].lifeTimeMax_, random0);
    }
    
    // 開始遅延
    particleDataBuffer[particleIndex].startDelay_ = emitParticleBuffer[emitIndex].startDelayMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwostartDelays_ > 0)
    {
        particleDataBuffer[particleIndex].startDelay_ = lerp(emitParticleBuffer[emitIndex].startDelayMin_, emitParticleBuffer[emitIndex].startDelayMax_, random0);
    }
    
    // 生存フラグ
    particleDataBuffer[particleIndex].isAlive_ = 1.0f;
    
    // 位置
    particleDataBuffer[particleIndex].position_ = emitParticleBuffer[emitIndex].positionMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoPositions_ > 0)
    {
        particleDataBuffer[particleIndex].position_ = lerp(emitParticleBuffer[emitIndex].positionMin_, emitParticleBuffer[emitIndex].positionMax_, random0);
    }
    // 速度
    particleDataBuffer[particleIndex].velocity_ = emitParticleBuffer[emitIndex].velocityMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwovelocities_ > 0)
    {
        particleDataBuffer[particleIndex].velocity_ =
        emitParticleBuffer[emitIndex].velocityMin_ + (emitParticleBuffer[emitIndex].velocityMax_ - emitParticleBuffer[emitIndex].velocityMin_) * random0;
        
        //particleDataBuffer[particleIndex].velocity_ = lerp(emitParticleBuffer[emitIndex].velocityMin_, emitParticleBuffer[emitIndex].velocityMax_, random0);
    }
    particleDataBuffer[particleIndex].acceleration_ = emitParticleBuffer[emitIndex].accelerationMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoAccelerations_ > 0)
    {
        particleDataBuffer[particleIndex].acceleration_ = lerp(emitParticleBuffer[emitIndex].accelerationMin_, emitParticleBuffer[emitIndex].accelerationMax_, random0);
    }
    
    // 重力
    particleDataBuffer[particleIndex].gravity_ = emitParticleBuffer[emitIndex].gravityMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoGravities_ > 0)
    {
        particleDataBuffer[particleIndex].gravity_ = lerp(emitParticleBuffer[emitIndex].gravityMin_, emitParticleBuffer[emitIndex].gravityMax_, random0);
    }
    
    // 回転
    particleDataBuffer[particleIndex].rotation_ = emitParticleBuffer[emitIndex].rotationMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoRotations_ > 0)
    {
        particleDataBuffer[particleIndex].rotation_ = lerp(emitParticleBuffer[emitIndex].rotationMin_, emitParticleBuffer[emitIndex].rotationMax_, random0);
    }
    // 回転速度
    particleDataBuffer[particleIndex].rotationVelocity_ = emitParticleBuffer[emitIndex].rotationVelocityMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoRotationVelocities_ > 0)
    {
        particleDataBuffer[particleIndex].rotationVelocity_ = lerp(emitParticleBuffer[emitIndex].rotationVelocityMin_, emitParticleBuffer[emitIndex].rotationVelocityMax_, random0);
    }
    particleDataBuffer[particleIndex].rotationAcceleration_ = emitParticleBuffer[emitIndex].rotationAccelerationMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoRotationAccelerations_ > 0)
    {
        particleDataBuffer[particleIndex].rotationAcceleration_ = lerp(emitParticleBuffer[emitIndex].rotationAccelerationMin_, emitParticleBuffer[emitIndex].rotationAccelerationMax_, random0);
    }
    
    // 大きさ
    particleDataBuffer[particleIndex].scale_ = emitParticleBuffer[emitIndex].scaleMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoScales_ > 0)
    {
        particleDataBuffer[particleIndex].scale_ = lerp(emitParticleBuffer[emitIndex].scaleMin_, emitParticleBuffer[emitIndex].scaleMax_, random0);
    }
    // 大きさ速度
    particleDataBuffer[particleIndex].scaleVelocity_ = emitParticleBuffer[emitIndex].scaleVelocityMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoScaleVelocities_ > 0)
    {
        particleDataBuffer[particleIndex].scaleVelocity_ = lerp(emitParticleBuffer[emitIndex].scaleVelocityMin_, emitParticleBuffer[emitIndex].scaleVelocityMax_, random0);
    }
    particleDataBuffer[particleIndex].scaleAcceleration_ = emitParticleBuffer[emitIndex].scaleAccelerationMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoScaleAccelerations_ > 0)
    {
        particleDataBuffer[particleIndex].scaleAcceleration_ = lerp(emitParticleBuffer[emitIndex].scaleAccelerationMin_, emitParticleBuffer[emitIndex].scaleAccelerationMax_, random0);
    }
    
    // 色
    particleDataBuffer[particleIndex].startColor_ = emitParticleBuffer[emitIndex].startColorMin_;
    if (emitParticleBuffer[emitIndex].randomBetweenTwoColors_ > 0)
    {
        particleDataBuffer[particleIndex].startColor_ = lerp(emitParticleBuffer[emitIndex].startColorMin_, emitParticleBuffer[emitIndex].startColorMax_, random0);
    }
    particleDataBuffer[particleIndex].endColor_ = emitParticleBuffer[emitIndex].endColor_;
}