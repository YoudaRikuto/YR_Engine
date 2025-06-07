#include "ComputeParticle.hlsli"

RWStructuredBuffer<ParticleData> particleDataBuffer : register(u0); //  パーティクル管理バッファ
AppendStructuredBuffer<uint> particleUnusedBuffer : register(u1); //  パーティクル番号管理バッファ(末尾への追加専用)

RWByteAddressBuffer indirectDataBuffer : register(u2); // インダイレクト用バッファー

[numthreads(NumParticleThread, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint index = dispatchThreadId.x;
    
    // 有効フラグが立っているものだけ処理
    if (particleDataBuffer[index].isAlive_ < 0.0f)
        return;
    
    // パーティクル自体の生存時間
    particleDataBuffer[index].duration_ -= deltaTime_;
    if (particleDataBuffer[index].duration_ < 0)
    {
        // 寿命が尽きたら未使用リストに追加
        particleDataBuffer[index].isAlive_ = -1.0f; // 生存フラグを初期化しておく
        particleUnusedBuffer.Append(index);
        
        // 死亡数をカウントする
        uint originalValue;
        indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumDeadParticle, 1, originalValue);
        
        return;
    }
    
    // 開始遅延処理
    particleDataBuffer[index].startDelay_ -= deltaTime_;
    if (particleDataBuffer[index].startDelay_ > 0.0f)
        return;
        
    // 経過時間分減少させる
    particleDataBuffer[index].lifeTime_ -= deltaTime_;
    if (particleDataBuffer[index].lifeTime_ < 0)
    {
        // 寿命が尽きたら未使用リストに追加
        particleDataBuffer[index].isAlive_ = -1.0f; // 生存フラグを初期化しておく
        particleUnusedBuffer.Append(index);
        
        // 死亡数をカウントする
        uint originalValue;
        indirectDataBuffer.InterlockedAdd(IndirectArgumentsNumDeadParticle, 1, originalValue);
        
        return;
    }
    
    // 速度更新
    particleDataBuffer[index].velocity_.xyz += particleDataBuffer[index].acceleration_.xyz * deltaTime_;
    particleDataBuffer[index].velocity_.y -= particleDataBuffer[index].gravity_ * deltaTime_;
    
    // 位置更新
    particleDataBuffer[index].position_.xyz += particleDataBuffer[index].velocity_.xyz * deltaTime_;
    
    
    // 回転速度更新
    particleDataBuffer[index].rotationVelocity_.xyz += particleDataBuffer[index].rotationAcceleration_.xyz * deltaTime_;
    
    // 回転更新
    particleDataBuffer[index].rotation_.xyz += particleDataBuffer[index].rotationVelocity_.xyz * deltaTime_;
    
    // スケール速度変更
    particleDataBuffer[index].scaleVelocity_.xyz += particleDataBuffer[index].scaleAcceleration_.xyz * deltaTime_;
    
    // スケール更新
    particleDataBuffer[index].scale_.xyz += particleDataBuffer[index].scaleVelocity_.xyz * deltaTime_;
    
    // 切り取り座標を算出
    uint type = (uint) (particleDataBuffer[index].textureType_ + 0.5f);
    
    float width = 1.0 / textureSplitCount_.x;
    float height = 1.0 / textureSplitCount_.y;
    
    float2 uv = float2((type % textureSplitCount_.x) * width, (type / textureSplitCount_.x) * height);
    particleDataBuffer[index].texcoord_.xy = uv;
    particleDataBuffer[index].texcoord_.zw = float2(width, height);
    
    // 徐々に透明にしていく
    particleDataBuffer[index].startColor_.a = saturate(particleDataBuffer[index].lifeTime_ * 0.5f);
}