#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include "Graphics/ConstantBuffer.h"

class ComputeParticleSystem
{
public:
    static constexpr UINT numParticleThread_ = 1024;

#if 0
    //struct EmitData
    struct EmitParticleData
    {
        DirectX::XMFLOAT4 position_ = {};
        DirectX::XMFLOAT4 rotation_ = {};
        DirectX::XMFLOAT4 scale_ = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    };
    
    struct ParticleData
    {
        DirectX::XMFLOAT4 position_ = {};
        DirectX::XMFLOAT4 rotation_ = {};
        DirectX::XMFLOAT4 scale_ = {};
        DirectX::XMFLOAT4 color_ = {};
    };
#endif

    // エミット
    //struct karideta
    struct EmitParticleData
    {
        int     emitParticleNum_    = 100;    // 粒子生成数
        int     textureType_        = 0;    // テクスチャタイプ
        int     isAlive_            = 1;    // 生存フラグ
        float   duration_           = 5.0f; // エフェクト再生時間
        float   lifeTimeMin_        = 5.0f; // 生存時間 Min
        float   lifeTimeMax_        = 5.0f; // 生存時間 Max
        float   startDelayMin_      = 0.0f; // 開始遅延 Min
        float   startDelayMax_      = 0.0f; // 開始遅延 Max

        float   gravityMin_ = 0.0f; // 重力 Min
        float   gravityMax_ = 0.0f; // 重力 Max

        // ---------- Min Max 使用フラグ ----------
        int     randomBetweenTwoLifeTimes_              = 0; // 生存時間 
        int     randomBetweenTwostartDelays_            = 0; // 開始遅延
        int     randomBetweenTwoPositions_              = 0; // 位置 
        int     randomBetweenTwovelocities_             = 0; // 初速度 
        int     randomBetweenTwoAccelerations_          = 0; // 加速度 
        int     randomBetweenTwoGravities_              = 0; // 重力 
        int     randomBetweenTwoRotations_              = 0; // 回転値 
        int     randomBetweenTwoRotationVelocities_     = 0; // 回転初速度 
        int     randomBetweenTwoRotationAccelerations_  = 0; // 回転加速度 
        int     randomBetweenTwoScales_                 = 0; // 大きさ
        int     randomBetweenTwoScaleVelocities_        = 0; // 大きさ初速度
        int     randomBetweenTwoScaleAccelerations_     = 0; // 大きさ加速度
        int     randomBetweenTwoColors_                 = 0; // 開始色 
        
        int dummy_ = 0.0f;


        DirectX::XMFLOAT4 positionMin_              = {}; // 生成位置 Min
        DirectX::XMFLOAT4 positionMax_              = {}; // 生成位置 Max
        DirectX::XMFLOAT4 velocityMin_              = {}; // 初速度 Min
        DirectX::XMFLOAT4 velocityMax_              = {}; // 初速度 Max
        DirectX::XMFLOAT4 accelerationMin_          = {}; // 加速度 Min
        DirectX::XMFLOAT4 accelerationMax_          = {}; // 加速度 Max

        DirectX::XMFLOAT4 rotationMin_              = {}; // 回転値 Min
        DirectX::XMFLOAT4 rotationMax_              = {}; // 回転値 Max
        DirectX::XMFLOAT4 rotationVelocityMin_      = {}; // 回転初速度 Min
        DirectX::XMFLOAT4 rotationVelocityMax_      = {}; // 回転初速度 Max
        DirectX::XMFLOAT4 rotationAccelerationMin_  = {}; // 回転加速度 Min
        DirectX::XMFLOAT4 rotationAccelerationMax_  = {}; // 回転加速度 Max

        DirectX::XMFLOAT4 scaleMin_                 = { 10.0f, 10.0f, 10.0f, 10.0f }; // 大きさ Min
        DirectX::XMFLOAT4 scaleMax_                 = { 1.0f, 1.0f, 1.0f, 1.0f }; // 大きさ Max
        DirectX::XMFLOAT4 scaleVelocityMin_         = {}; // 大きさ初速度 Min
        DirectX::XMFLOAT4 scaleVelocityMax_         = {}; // 大きさ初速度 Max
        DirectX::XMFLOAT4 scaleAccelerationMin_     = {}; // 大きさ加速度 Min
        DirectX::XMFLOAT4 scaleAccelerationMax_     = {}; // 大きさ加速度 Max

        DirectX::XMFLOAT4 startColorMin_            = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色 Min
        DirectX::XMFLOAT4 startColorMax_            = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色 Max
        DirectX::XMFLOAT4 endColor_                 = { 1.0f, 1.0f, 1.0f, 1.0f }; // 終了色
    };

    // 保持データ
    //struct karidataA
    struct ParticleData
    {
        int     textureType_;   // テクスチャタイプ
        int     isAlive_;       // 生存しているか
        float   duration_;      // エフェクトの生存時間
        float   lifeTime_;      // 粒子の生存時間
        float   startDelay_;    // 生成ディレイ        
        float   gravity_;

        float dummy1_;
        float dummy2_;

        DirectX::XMFLOAT4 position_;
        DirectX::XMFLOAT4 velocity_;
        DirectX::XMFLOAT4 acceleration_;

        DirectX::XMFLOAT4 rotation_;
        DirectX::XMFLOAT4 rotationVelocity_;
        DirectX::XMFLOAT4 rotationAcceleration_;
        
        DirectX::XMFLOAT4 scale_;
        DirectX::XMFLOAT4 scaleVelocity_;
        DirectX::XMFLOAT4 scaleAcceleration_;

        DirectX::XMFLOAT4 startColor_;
        DirectX::XMFLOAT4 endColor_;

        DirectX::XMFLOAT4 texcoord_;
    };

    struct Constants
    {
        float               deltaTime_;
        DirectX::XMUINT2    textureSplitCount_;
        UINT                systemNumParticles_;

        UINT totalEmitCount_;

        DirectX::XMUINT3 dummy_;
    };

    using DispatchIndirect = DirectX::XMUINT3;

    static constexpr UINT numCurrentParticleOffset_     = 0;
    static constexpr UINT numPreviousParticleOffset_    = numCurrentParticleOffset_ + sizeof(UINT);
    static constexpr UINT numDeadParticleOffset_        = numPreviousParticleOffset_ + sizeof(UINT);
    static constexpr UINT emitDispatchIndirectOffset_   = numDeadParticleOffset_ + sizeof(UINT);
    static constexpr UINT drawIndirectSize_             = emitDispatchIndirectOffset_ + sizeof(DispatchIndirect);

private:
    ComputeParticleSystem(const UINT& particleCount, const DirectX::XMUINT2& splitCount);
    ~ComputeParticleSystem();

public:
    static ComputeParticleSystem& Instance()
    {
        static ComputeParticleSystem instance(1000000, { 4, 4 });
        return instance;
    }

    void Update(const float& elapsedTime);
    void Render();
    void DrawDebug();

    void EmitParticle();

private:
    UINT numParticles_;
    UINT numEmitParticles_;
    bool oneShotInitialize_;
    DirectX::XMUINT2 textureSplitCount_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> perlinNoiseTexture_;

    EmitParticleData emitParticleData_;

    std::vector<EmitParticleData> emitParticles_;    
    std::unique_ptr<ConstantBuffer<Constants>> constants_;

    Microsoft::WRL::ComPtr<ID3D11Buffer>                particleDataBuffer_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    particleDataSRV_;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>   particleDataUAV_;

    Microsoft::WRL::ComPtr<ID3D11Buffer>                particleAppendConsumeBuffer_;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>   particleAppendConsumeUAV_;

    Microsoft::WRL::ComPtr<ID3D11Buffer>                particleEmitBuffer_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    particleEmitSRV_;

    Microsoft::WRL::ComPtr<ID3D11ComputeShader>         initComputeShader_;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader>         emitComputeShader_;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader>         updateComputeShader_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader>        geometryShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           pixelShader_;

    Microsoft::WRL::ComPtr<ID3D11Buffer>                indirectDataBuffer_;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>   indirectDataUAV_;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader>         beginFrameShader_;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader>         endFrameShader_;
};

