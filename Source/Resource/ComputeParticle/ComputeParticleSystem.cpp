#include "ComputeParticleSystem.h"
#include "Graphics/Graphics.h"
#include "FrameWork/Misc.h"
#include "Resource/Texture.h"
#include "ImGui/ImGuiCtrl.h"
#include "Math/MathHelper.h"

ComputeParticleSystem::ComputeParticleSystem(const UINT& particleCount, const DirectX::XMUINT2& splitCount)
{
    HRESULT result = S_OK;
    ID3D11Device* device = Graphics::Instance().GetDevice();

    numParticles_ = ((particleCount + (numParticleThread_ - 1)) / numParticleThread_) * numParticleThread_;
    numEmitParticles_ = min(numParticles_, 10000); // 1フレームの生成制限数※要調整

    textureSplitCount_ = splitCount;
    oneShotInitialize_ = false;

    constants_ = std::make_unique<ConstantBuffer<Constants>>();

    // パーティクルバッファ生成
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.ByteWidth            = sizeof(ParticleData) * numParticles_;
        bufferDesc.MiscFlags            = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride  = sizeof(ParticleData);
        bufferDesc.Usage                = D3D11_USAGE_DEFAULT;
        
        result = device->CreateBuffer(&bufferDesc, nullptr, particleDataBuffer_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        result = device->CreateShaderResourceView(particleDataBuffer_.Get(), nullptr, particleDataSRV_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        result = device->CreateUnorderedAccessView(particleDataBuffer_.Get(), nullptr, particleDataUAV_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    // パーティクルの生成 / 破棄番号をため込むバッファ生成
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags            = D3D11_BIND_UNORDERED_ACCESS;
        bufferDesc.ByteWidth            = sizeof(UINT) * numParticles_;
        bufferDesc.MiscFlags            = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride  = sizeof(UINT);
        bufferDesc.Usage                = D3D11_USAGE_DEFAULT;

        result = device->CreateBuffer(&bufferDesc, nullptr, particleAppendConsumeBuffer_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements  = numParticles_;
        uavDesc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_APPEND;

        result = device->CreateUnorderedAccessView(particleAppendConsumeBuffer_.Get(), &uavDesc, particleAppendConsumeUAV_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    // パーティクルエミット用バッファ作成
    {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.ByteWidth            = sizeof(EmitParticleData) * numEmitParticles_;
        bufferDesc.MiscFlags            = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride  = sizeof(EmitParticleData);
        bufferDesc.Usage                = D3D11_USAGE_DEFAULT;

        result = device->CreateBuffer(&bufferDesc, nullptr, particleEmitBuffer_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        result = device->CreateShaderResourceView(particleEmitBuffer_.Get(), nullptr, particleEmitSRV_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    // パーティクルの更新・描画数の削減のためのバッファ
    {
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags  = D3D11_BIND_UNORDERED_ACCESS;
        desc.ByteWidth  = drawIndirectSize_;
        desc.MiscFlags  = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        desc.Usage      = D3D11_USAGE_DEFAULT;
        
        result = device->CreateBuffer(&desc, nullptr, indirectDataBuffer_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Format              = DXGI_FORMAT_R32_TYPELESS;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements  = desc.ByteWidth / sizeof(UINT);
        uavDesc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_RAW;
        
        result = device->CreateUnorderedAccessView(indirectDataBuffer_.Get(), &uavDesc, indirectDataUAV_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    // コンピュートシェーダ読み込み
    Graphics::Instance().CreateCsFromCso("./Resources/Shader/ComputeParticleInitCS.cso", initComputeShader_.GetAddressOf());
    Graphics::Instance().CreateCsFromCso("./Resources/Shader/ComputeParticleEmitCS.cso", emitComputeShader_.GetAddressOf());
    Graphics::Instance().CreateCsFromCso("./Resources/Shader/ComputeParticleUpdateCS.cso", updateComputeShader_.GetAddressOf());
    Graphics::Instance().CreateCsFromCso("./Resources/Shader/ComputeParticleBeginFrameCS.cso", beginFrameShader_.GetAddressOf());
    Graphics::Instance().CreateCsFromCso("./Resources/Shader/ComputeParticleEndFrameCS.cso", endFrameShader_.GetAddressOf());

    // 描画用情報生成
    shaderResourceView_ = Texture::Instance().LoadTexture(L"./Resources/Image/Particle/Particle.png").shaderResourceView_;
    perlinNoiseTexture_ = Texture::Instance().LoadTexture(L"./Resources/Image/Noise/PerlinNoise.png").shaderResourceView_;
    Graphics::Instance().CreateVsFromCso("./Resources/Shader/ComputeParticleRenderVS.cso", vertexShader_.GetAddressOf(), nullptr, nullptr, 0);
    Graphics::Instance().CreateGsFromCso("./Resources/Shader/ComputeParticleRenderGS.cso", geometryShader_.GetAddressOf());
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/ComputeParticleRenderPS.cso", pixelShader_.GetAddressOf());
}

ComputeParticleSystem::~ComputeParticleSystem()
{
    initComputeShader_.Reset();
    updateComputeShader_.Reset();
    emitComputeShader_.Reset();
    vertexShader_.Reset();
    geometryShader_.Reset();
    pixelShader_.Reset();
    shaderResourceView_.Reset();
    perlinNoiseTexture_.Reset();
}

// 更新
void ComputeParticleSystem::Update(const float& elapsedTime)
{
    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

    constants_->GetData()->deltaTime_ = elapsedTime;
    constants_->GetData()->textureSplitCount_ = textureSplitCount_;
    constants_->GetData()->systemNumParticles_ = numParticles_;
    constants_->GetData()->totalEmitCount_ = static_cast<UINT>(emitParticles_.size());;
    constants_->Activate(10, false, true, true);


    deviceContext->CSSetShaderResources(0, 1, particleEmitSRV_.GetAddressOf());
    deviceContext->CSSetShaderResources(1, 1, perlinNoiseTexture_.GetAddressOf());

    ID3D11UnorderedAccessView* uavs[] =
    {
        particleDataUAV_.Get(),
        particleAppendConsumeUAV_.Get(),
        indirectDataUAV_.Get(),
    };
    deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);

    // 初期化処理
    if (!oneShotInitialize_)
    {
        oneShotInitialize_ = true;
        deviceContext->CSSetShader(initComputeShader_.Get(), nullptr, 0);
        deviceContext->Dispatch(numParticles_ / numParticleThread_, 1, 1);
    }

    // フレーム開始時の処理
    {
        // 現在フレームでのパーティクル総数を算出
        // それに合わせて各種設定を行う
        deviceContext->CSSetShader(beginFrameShader_.Get(), nullptr, 0);
        deviceContext->Dispatch(1, 1, 1);
    }

    // エミット処理
    if (!emitParticles_.empty())
    {
        // エミットバッファ更新
        D3D11_BOX writeBox = {};
        writeBox.left = 0;
        writeBox.right = static_cast<UINT>(emitParticles_.size() * sizeof(EmitParticleData));
        writeBox.top = 0;
        writeBox.bottom = 1;
        writeBox.front = 0;
        writeBox.back = 1;
        deviceContext->UpdateSubresource(
            particleEmitBuffer_.Get(),
            0,
            &writeBox,
            emitParticles_.data(),
            static_cast<UINT>(emitParticles_.size() * sizeof(EmitParticleData)),
            0);
        deviceContext->CSSetShader(emitComputeShader_.Get(), nullptr, 0);
        deviceContext->DispatchIndirect(indirectDataBuffer_.Get(), emitDispatchIndirectOffset_);
        emitParticles_.clear();
    }

    // 更新処理
    {
        deviceContext->CSSetShader(updateComputeShader_.Get(), nullptr, 0);
        deviceContext->Dispatch(numParticles_ / numParticleThread_, 1, 1);
    }

    // フレーム終了時の処理
    {
        // 総パーティクル数を変動させる
        deviceContext->CSSetShader(endFrameShader_.Get(), nullptr, 0);
        deviceContext->Dispatch(1, 1, 1);
    }

    // 後処理
    {
        ID3D11UnorderedAccessView* uavs[] = { nullptr, nullptr, nullptr, nullptr };
        deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);
    }
}

// 描画
void ComputeParticleSystem::Render()
{
    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    deviceContext->IASetInputLayout(nullptr);

    deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
    deviceContext->GSSetShader(geometryShader_.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);

    deviceContext->PSSetShaderResources(0, 1, shaderResourceView_.GetAddressOf());
    deviceContext->GSSetShaderResources(0, 1, particleDataSRV_.GetAddressOf());

    // バッファクリア
    ID3D11Buffer* clearBuffer[] = { nullptr };
    UINT strides[] = { 0 };
    UINT offsets[] = { 0 };
    deviceContext->IASetVertexBuffers(0, 1, clearBuffer, strides, offsets);
    deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

    // パーティクル情報分描画コール
    deviceContext->Draw(numParticles_, 0);

    // 後始末
    deviceContext->VSSetShader(nullptr, nullptr, 0);
    deviceContext->GSSetShader(nullptr, nullptr, 0);
    deviceContext->PSSetShader(nullptr, nullptr, 0);

    ID3D11ShaderResourceView* clearShaderResourceViews[] = { nullptr };
    deviceContext->PSSetShaderResources(0, 1, clearShaderResourceViews);
    deviceContext->GSSetShaderResources(0, 1, clearShaderResourceViews);
}

// ImGui
void ComputeParticleSystem::DrawDebug()
{
    ImGui::Begin("Effect Editer");

    if (ImGui::Button("Emit Particle"))
    {
        EmitParticle();
    }

    ImGui::DragInt("Emit Particle Num", &computeParticleEmitData_.emitParticleNum_, 1, 0, 10000);
    ImGui::SliderInt("Texture Type", &computeParticleEmitData_.textureType_, 0, 16);
    ImGui::DragFloat("Duration", &computeParticleEmitData_.duration_, 0.1f, 0.0f, 10.0f);

    if (ImGui::TreeNodeEx("LifeTime", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoLifeTimes_);

        if (computeParticleEmitData_.randomBetweenTwoLifeTimes_)
        {
            ImGui::PushItemWidth(150);
            ImGui::DragFloat("## LifeTime Min", &computeParticleEmitData_.lifeTimeMin_, 0.1f, 0.0f, 10.0f);
            ImGui::SameLine();
            ImGui::DragFloat("## LifeTime Max", &computeParticleEmitData_.lifeTimeMax_, 0.1f, 0.0f, 10.0f);
            ImGui::SameLine();
            ImGui::Text("LifeTime");
            ImGui::PopItemWidth();
        }
        else
        {
            ImGui::DragFloat("LifeTime", &computeParticleEmitData_.lifeTimeMin_, 0.1f, 0.0f, 10.0f);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("StartDelay", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwostartDelays_);

        if (computeParticleEmitData_.randomBetweenTwostartDelays_)
        {
            ImGui::PushItemWidth(150);
            ImGui::DragFloat("## StartDelay Min", &computeParticleEmitData_.startDelayMin_, 0.1f, 0.0f, 10.0f);
            ImGui::SameLine();
            ImGui::DragFloat("## StartDelay Max", &computeParticleEmitData_.startDelayMax_, 0.1f, 0.0f, 10.0f);
            ImGui::SameLine();
            ImGui::Text("StartDelay");
            ImGui::PopItemWidth();
        }
        else
        {
            ImGui::DragFloat("StartDelay", &computeParticleEmitData_.startDelayMin_, 0.1f, 0.0f, 10.0f);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Gravity", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoGravities_);

        if (computeParticleEmitData_.randomBetweenTwoGravities_)
        {
            ImGui::PushItemWidth(150);
            ImGui::DragFloat("## Gravity Min", &computeParticleEmitData_.gravityMin_, 0.1f, 0.0f, 50.0f);
            ImGui::SameLine();
            ImGui::DragFloat("## Gravity Max", &computeParticleEmitData_.gravityMax_, 0.1f, 0.0f, 50.0f);
            ImGui::SameLine();
            ImGui::Text("Gravity");
            ImGui::PopItemWidth();
        }
        else
        {
            ImGui::DragFloat("Gravity", &computeParticleEmitData_.gravityMin_, 0.1f, 0.0f, 50.0f);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Position", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoPositions_);

        if (computeParticleEmitData_.randomBetweenTwoPositions_)
        {
            ImGui::DragFloat4("Position Min", &computeParticleEmitData_.positionMin_.x);
            ImGui::DragFloat4("Position Max", &computeParticleEmitData_.positionMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Position", &computeParticleEmitData_.positionMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Velocity", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoVelocities_);

        if (computeParticleEmitData_.randomBetweenTwoVelocities_)
        {
            ImGui::DragFloat4("Velocity Min", &computeParticleEmitData_.velocityMin_.x);
            ImGui::DragFloat4("Velocity Max", &computeParticleEmitData_.velocityMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Velocity", &computeParticleEmitData_.velocityMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Acceleration", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoAccelerations_);

        if (computeParticleEmitData_.randomBetweenTwoAccelerations_)
        {
            ImGui::DragFloat4("Acceleration Min", &computeParticleEmitData_.accelerationMin_.x);
            ImGui::DragFloat4("Acceleration Max", &computeParticleEmitData_.accelerationMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Acceleration", &computeParticleEmitData_.accelerationMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Rotation", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoRotations_);

        if (computeParticleEmitData_.randomBetweenTwoRotations_)
        {
            ImGui::DragFloat4("Rotation Min", &computeParticleEmitData_.rotationMin_.x);
            ImGui::DragFloat4("Rotation Max", &computeParticleEmitData_.rotationMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Rotation", &computeParticleEmitData_.rotationMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Rotation Velocity", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoRotationVelocities_);

        if (computeParticleEmitData_.randomBetweenTwoRotationVelocities_)
        {
            ImGui::DragFloat4("Rotation Velocity Min", &computeParticleEmitData_.rotationVelocityMin_.x);
            ImGui::DragFloat4("Rotation Velocity Max", &computeParticleEmitData_.rotationVelocityMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Rotation Velocity", &computeParticleEmitData_.rotationVelocityMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Rotation Acceleration", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoRotationAccelerations_);

        if (computeParticleEmitData_.randomBetweenTwoRotationAccelerations_)
        {
            ImGui::DragFloat4("Rotation Acceleration Min", &computeParticleEmitData_.rotationAccelerationMin_.x);
            ImGui::DragFloat4("Rotation Acceleration Max", &computeParticleEmitData_.rotationAccelerationMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Rotation Acceleration", &computeParticleEmitData_.rotationAccelerationMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Scale", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoScales_);

        if (computeParticleEmitData_.randomBetweenTwoScales_)
        {
            ImGui::DragFloat4("Scale Min", &computeParticleEmitData_.scaleMin_.x);
            ImGui::DragFloat4("Scale Max", &computeParticleEmitData_.scaleMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Scale", &computeParticleEmitData_.scaleMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Scale Velocity", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoScaleVelocities_);

        if (computeParticleEmitData_.randomBetweenTwoScaleVelocities_)
        {
            ImGui::DragFloat4("Scale Velocity Min", &computeParticleEmitData_.scaleVelocityMin_.x);
            ImGui::DragFloat4("Scale Velocity Max", &computeParticleEmitData_.scaleVelocityMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Scale Velocity", &computeParticleEmitData_.scaleVelocityMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Scale Acceleration", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoScaleAccelerations_);

        if (computeParticleEmitData_.randomBetweenTwoScaleAccelerations_)
        {
            ImGui::DragFloat4("Scale Acceleration Min", &computeParticleEmitData_.scaleAccelerationMin_.x);
            ImGui::DragFloat4("Scale Acceleration Max", &computeParticleEmitData_.scaleAccelerationMax_.x);
        }
        else
        {
            ImGui::DragFloat4("Scale Acceleration", &computeParticleEmitData_.scaleAccelerationMin_.x);
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoColors_);

        if (computeParticleEmitData_.randomBetweenTwoColors_)
        {
            ImGui::ColorEdit4("Start Color Min", &computeParticleEmitData_.startColorMin_.x);
            ImGui::ColorEdit4("Start Color Max", &computeParticleEmitData_.startColorMax_.x);
        }
        else
        {
            ImGui::ColorEdit4("Start Color", &computeParticleEmitData_.startColorMin_.x);
        }

        ImGui::ColorEdit4("End Color", &computeParticleEmitData_.endColor_.x);

        ImGui::TreePop();
    }


    ImGui::End();
}

void ComputeParticleSystem::EmitParticle(const EmitParticleData& emitParticleData)
{
    if (emitParticles_.size() >= numEmitParticles_) return;

    emitParticles_.emplace_back(emitParticleData);
}

void ComputeParticleSystem::EmitParticle()
{
    EmitParticleData emitParticleData = {};
    float weight_ = 0.0f;

    for (int i = 0; i < computeParticleEmitData_.emitParticleNum_; ++i)
    {
        emitParticleData.textureType_ = computeParticleEmitData_.textureType_;
        emitParticleData.duration_ = computeParticleEmitData_.duration_;

        if (computeParticleEmitData_.randomBetweenTwoLifeTimes_)
        {
            emitParticleData.lifeTime_ = XMFloatRandomRange(computeParticleEmitData_.lifeTimeMin_, computeParticleEmitData_.lifeTimeMax_);
        }
        else
        {
            emitParticleData.lifeTime_ = computeParticleEmitData_.lifeTimeMin_;
        }
        emitParticleData.lifeTimeConst_ = emitParticleData.lifeTime_;

        if (computeParticleEmitData_.randomBetweenTwostartDelays_)
        {
            emitParticleData.startDelay_ = XMFloatRandomRange(computeParticleEmitData_.startDelayMin_, computeParticleEmitData_.startDelayMax_);
        }
        else
        {
            emitParticleData.startDelay_ = computeParticleEmitData_.startDelayMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoGravities_)
        {
            emitParticleData.gravity_ = XMFloatRandomRange(computeParticleEmitData_.gravityMin_, computeParticleEmitData_.gravityMax_);
        }
        else
        {
            emitParticleData.gravity_ = computeParticleEmitData_.gravityMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoPositions_)
        {
            emitParticleData.position_ = XMFloat4RandomRange(computeParticleEmitData_.positionMin_, computeParticleEmitData_.positionMax_);
        }
        else
        {
            emitParticleData.position_ = computeParticleEmitData_.positionMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoVelocities_)
        {
            emitParticleData.velocity_ = XMFloat4RandomRange(computeParticleEmitData_.velocityMin_, computeParticleEmitData_.velocityMax_);
        }
        else
        {
            emitParticleData.velocity_ = computeParticleEmitData_.velocityMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoAccelerations_)
        {
            emitParticleData.acceleration_ = XMFloat4RandomRange(computeParticleEmitData_.accelerationMin_, computeParticleEmitData_.accelerationMax_);
        }
        else
        {
            emitParticleData.acceleration_ = computeParticleEmitData_.accelerationMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoRotations_)
        {
            emitParticleData.rotation_ = XMFloat4RandomRange(computeParticleEmitData_.rotationMin_, computeParticleEmitData_.rotationMax_);
        }
        else
        {
            emitParticleData.rotation_ = computeParticleEmitData_.rotationMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoRotationVelocities_)
        {
            emitParticleData.rotationVelocity_ = XMFloat4RandomRange(computeParticleEmitData_.rotationVelocityMin_, computeParticleEmitData_.rotationVelocityMax_);
        }
        else
        {
            emitParticleData.rotationVelocity_ = computeParticleEmitData_.rotationVelocityMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoRotationAccelerations_)
        {
            emitParticleData.rotationAcceleration_ = XMFloat4RandomRange(computeParticleEmitData_.rotationAccelerationMin_, computeParticleEmitData_.rotationAccelerationMax_);
        }
        else
        {
            emitParticleData.rotationAcceleration_ = computeParticleEmitData_.rotationAccelerationMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoScales_)
        {
            emitParticleData.scale_ = XMFloat4RandomRange(computeParticleEmitData_.scaleMin_, computeParticleEmitData_.scaleMax_);
        }
        else
        {
            emitParticleData.scale_ = computeParticleEmitData_.scaleMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoScaleVelocities_)
        {
            emitParticleData.scaleVelocity_ = XMFloat4RandomRange(computeParticleEmitData_.scaleVelocityMin_, computeParticleEmitData_.scaleVelocityMax_);
        }
        else
        {
            emitParticleData.scaleVelocity_ = computeParticleEmitData_.scaleVelocityMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoScaleAccelerations_)
        {
            emitParticleData.scaleAcceleration_ = XMFloat4RandomRange(computeParticleEmitData_.scaleAccelerationMin_, computeParticleEmitData_.scaleAccelerationMax_);
        }
        else
        {
            emitParticleData.scaleAcceleration_ = computeParticleEmitData_.scaleAccelerationMin_;
        }

        if (computeParticleEmitData_.randomBetweenTwoColors_)
        {
            emitParticleData.startColor_ = XMFloat4RandomRange(computeParticleEmitData_.startColorMin_, computeParticleEmitData_.startColorMax_);
        }
        else
        {
            emitParticleData.startColor_ = computeParticleEmitData_.startColorMin_;
        }

        emitParticleData.endColor_ = computeParticleEmitData_.endColor_;

        EmitParticle(emitParticleData);
    }
}
