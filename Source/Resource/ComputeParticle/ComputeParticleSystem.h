#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include "Graphics/ConstantBuffer.h"
#include <string>

class ComputeParticleSystem
{
public:
    static constexpr UINT numParticleThread_ = 1024;

    struct ComputeParticleEmitData
    {
        std::string name_ = "";

        int     emitParticleNum_    = 100;  // 粒子生成数
        int     textureType_        = 0;    // テクスチャタイプ
        float   duration_           = 1.0f; // エフェクト再生時間

        float   lifeTimeMin_    = 5.0f; // 生存時間 Min
        float   lifeTimeMax_    = 5.0f; // 生存時間 Max
        float   startDelayMin_  = 0.0f; // 開始遅延 Min
        float   startDelayMax_  = 0.0f; // 開始遅延 Max

        float   gravityMin_ = 0.0f; // 重力 Min
        float   gravityMax_ = 0.0f; // 重力 Max

        DirectX::XMFLOAT4 positionMin_ = {}; // 生成位置 Min
        DirectX::XMFLOAT4 positionMax_ = {}; // 生成位置 Max
        DirectX::XMFLOAT4 velocityMin_ = {}; // 初速度 Min
        DirectX::XMFLOAT4 velocityMax_ = {}; // 初速度 Max
        DirectX::XMFLOAT4 accelerationMin_ = {}; // 加速度 Min
        DirectX::XMFLOAT4 accelerationMax_ = {}; // 加速度 Max

        DirectX::XMFLOAT4 rotationMin_ = {}; // 回転値 Min
        DirectX::XMFLOAT4 rotationMax_ = {}; // 回転値 Max
        DirectX::XMFLOAT4 rotationVelocityMin_ = {}; // 回転初速度 Min
        DirectX::XMFLOAT4 rotationVelocityMax_ = {}; // 回転初速度 Max
        DirectX::XMFLOAT4 rotationAccelerationMin_ = {}; // 回転加速度 Min
        DirectX::XMFLOAT4 rotationAccelerationMax_ = {}; // 回転加速度 Max

        DirectX::XMFLOAT4 scaleMin_             = { 1.0f, 1.0f, 1.0f, 1.0f }; // 大きさ Min
        DirectX::XMFLOAT4 scaleMax_             = { 1.0f, 1.0f, 1.0f, 1.0f }; // 大きさ Max
        DirectX::XMFLOAT4 scaleVelocityMin_     = {}; // 大きさ初速度 Min
        DirectX::XMFLOAT4 scaleVelocityMax_     = {}; // 大きさ初速度 Max
        DirectX::XMFLOAT4 scaleAccelerationMin_ = {}; // 大きさ加速度 Min
        DirectX::XMFLOAT4 scaleAccelerationMax_ = {}; // 大きさ加速度 Max

        DirectX::XMFLOAT4 startColorMin_    = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色 Min
        DirectX::XMFLOAT4 startColorMax_    = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色 Max
        DirectX::XMFLOAT4 endColor_         = { 1.0f, 1.0f, 1.0f, 1.0f }; // 終了色


        // ---------- Min Max 使用フラグ ----------
        bool    randomBetweenTwoLifeTimes_ = false;
        bool    randomBetweenTwoStartDelays_ = false;
        bool    randomBetweenTwoPositions_ = false;
        bool    randomBetweenTwoVelocities_ = false;
        bool    randomBetweenTwoAccelerations_ = false;
        bool    randomBetweenTwoGravities_ = false;
        bool    randomBetweenTwoRotations_ = false;
        bool    randomBetweenTwoRotationVelocities_ = false;
        bool    randomBetweenTwoRotationAccelerations_ = false;
        bool    randomBetweenTwoScales_ = false;
        bool    randomBetweenTwoScaleVelocities_ = false;
        bool    randomBetweenTwoScaleAccelerations_ = false;
        bool    randomBetweenTwoColors_ = false;
    };

    struct EmitParticleData
    {
        int     textureType_    = 0;
        int     isAlive_        = 0;
        float   duration_       = 0;
        float   lifeTime_       = 0;
        float   startDelay_     = 0;
        float   gravity_        = 0.0f;

        float lifeTimeConst_ = 0.0f;
        float dummy2_ = 0.0f;

        DirectX::XMFLOAT4 position_             = {};
        DirectX::XMFLOAT4 velocity_             = {};
        DirectX::XMFLOAT4 acceleration_         = {};
        DirectX::XMFLOAT4 rotation_             = {};
        DirectX::XMFLOAT4 rotationVelocity_     = {};
        DirectX::XMFLOAT4 rotationAcceleration_ = {};
        DirectX::XMFLOAT4 scale_                = {};
        DirectX::XMFLOAT4 scaleVelocity_        = {};
        DirectX::XMFLOAT4 scaleAcceleration_    = {};
        DirectX::XMFLOAT4 startColor_           = {};
        DirectX::XMFLOAT4 endColor_             = {};
    };

    // 保持データ
    struct ParticleData
    {
        int     textureType_;   // テクスチャタイプ
        int     isAlive_;       // 生存しているか
        float   duration_;      // エフェクトの生存時間
        float   lifeTime_;      // 粒子の生存時間
        float   startDelay_;    // 生成ディレイ        
        float   gravity_;

        float lifeTimeConst_;
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

    void LoadEmitDataFromJsonFile(const std::string& filename);
    void ClearResource() { computeParticleData_.clear(); }

    void Update(const float& elapsedTime);
    void Render();
    void DrawDebug();

    
    void EmitParticle(const std::string& effectName);

private:
    void EmitParticle(const EmitParticleData& emitParticleData);
    void EmitParticle();
    void AssetCreation(const ComputeParticleEmitData& data, const std::string& filename);

private:
    UINT numParticles_;
    UINT numEmitParticles_;
    bool oneShotInitialize_;
    DirectX::XMUINT2 textureSplitCount_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> perlinNoiseTexture_;

    ComputeParticleEmitData computeParticleEmitData_ = {};
    std::vector<ComputeParticleEmitData> computeParticleData_;
    

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

