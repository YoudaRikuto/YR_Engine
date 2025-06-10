#include "ComputeParticleSystem.h"
#include "Graphics/Graphics.h"
#include "FrameWork/Misc.h"
#include "Resource/Texture.h"
#include "ImGui/ImGuiCtrl.h"
#include "Math/MathHelper.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

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

// Jsonファイルから EmitData を読み取る
void ComputeParticleSystem::LoadEmitDataFromJsonFile(const std::string& filename)
{
    std::string filepath = "./Resources/JsonParameters/ComputeParticle/" + filename;
    std::ifstream ifs(filepath);
    nlohmann::json jsonData;
    if (ifs.good()) ifs >> jsonData;
    else _ASSERT_EXPR(false, L"Asset not found");

    ComputeParticleEmitData data;
    data.name_              = jsonData["EffectName"];
    data.emitParticleNum_   = jsonData["EmitParticleNum"];
    data.textureType_       = jsonData["TextureType"];
    data.duration_          = jsonData["Duration"];
    data.lifeTimeMin_       = jsonData["LifeTimeMin"];
    data.lifeTimeMax_       = jsonData["LifeTimeMax"];
    data.startDelayMin_     = jsonData["StartDelayMin"];
    data.startDelayMax_     = jsonData["StartDelayMax"];
    data.gravityMin_        = jsonData["GravityMin"];
    data.gravityMax_        = jsonData["GravityMax"];

    data.positionMin_               = { jsonData["PositionMin"][0], jsonData["PositionMin"][1], jsonData["PositionMin"][2], jsonData["PositionMin"][3] };
    data.positionMax_               = { jsonData["PositionMax"][0], jsonData["PositionMax"][1], jsonData["PositionMax"][2], jsonData["PositionMax"][3] };
    data.velocityMin_               = { jsonData["VelocityMin"][0], jsonData["VelocityMin"][1], jsonData["VelocityMin"][2], jsonData["VelocityMin"][3] };
    data.velocityMax_               = { jsonData["VelocityMax"][0], jsonData["VelocityMax"][1], jsonData["VelocityMax"][2], jsonData["VelocityMax"][3] };
    data.accelerationMin_           = { jsonData["AccelerationMin"][0], jsonData["AccelerationMin"][1], jsonData["AccelerationMin"][2], jsonData["AccelerationMin"][3] };
    data.accelerationMax_           = { jsonData["AccelerationMax"][0], jsonData["AccelerationMax"][1], jsonData["AccelerationMax"][2], jsonData["AccelerationMax"][3] };
    data.rotationMin_               = { jsonData["RotationMin"][0], jsonData["RotationMin"][1], jsonData["RotationMin"][2], jsonData["RotationMin"][3] };
    data.rotationMax_               = { jsonData["RotationMax"][0], jsonData["RotationMax"][1], jsonData["RotationMax"][2], jsonData["RotationMax"][3] };
    data.rotationVelocityMin_       = { jsonData["RotationVelocityMin"][0], jsonData["RotationVelocityMin"][1], jsonData["RotationVelocityMin"][2], jsonData["RotationVelocityMin"][3] };
    data.rotationVelocityMax_       = { jsonData["RotationVelocityMax"][0], jsonData["RotationVelocityMax"][1], jsonData["RotationVelocityMax"][2], jsonData["RotationVelocityMax"][3] };
    data.rotationAccelerationMin_   = { jsonData["RotationAccelerationMin"][0], jsonData["RotationAccelerationMin"][1], jsonData["RotationAccelerationMin"][2], jsonData["RotationAccelerationMin"][3] };
    data.rotationAccelerationMax_   = { jsonData["RotationAccelerationMax"][0], jsonData["RotationAccelerationMax"][1], jsonData["RotationAccelerationMax"][2], jsonData["RotationAccelerationMax"][3] };
    data.scaleMin_                  = { jsonData["ScaleMin"][0], jsonData["ScaleMin"][1], jsonData["ScaleMin"][2], jsonData["ScaleMin"][3] };
    data.scaleMax_                  = { jsonData["ScaleMax"][0], jsonData["ScaleMax"][1], jsonData["ScaleMax"][2], jsonData["ScaleMax"][3] };
    data.scaleVelocityMin_          = { jsonData["ScaleVelocityMin"][0], jsonData["ScaleVelocityMin"][1], jsonData["ScaleVelocityMin"][2], jsonData["ScaleVelocityMin"][3] };
    data.scaleVelocityMax_          = { jsonData["ScaleVelocityMax"][0], jsonData["ScaleVelocityMax"][1], jsonData["ScaleVelocityMax"][2], jsonData["ScaleVelocityMax"][3] };
    data.scaleAccelerationMin_      = { jsonData["ScaleAccelerationMin"][0], jsonData["ScaleAccelerationMin"][1], jsonData["ScaleAccelerationMin"][2], jsonData["ScaleAccelerationMin"][3] };
    data.scaleAccelerationMax_      = { jsonData["ScaleAccelerationMax"][0], jsonData["ScaleAccelerationMax"][1], jsonData["ScaleAccelerationMax"][2], jsonData["ScaleAccelerationMax"][3] };
    data.startColorMin_             = { jsonData["StartColorMin"][0], jsonData["StartColorMin"][1], jsonData["StartColorMin"][2], jsonData["StartColorMin"][3] };
    data.startColorMax_             = { jsonData["StartColorMax"][0], jsonData["StartColorMax"][1], jsonData["StartColorMax"][2], jsonData["StartColorMax"][3] };
    data.endColor_                  = { jsonData["EndColor"][0], jsonData["EndColor"][1], jsonData["EndColor"][2], jsonData["EndColor"][3] };

    data.randomBetweenTwoLifeTimes_             = jsonData["RandomBetweenTwoLifeTimes"];
    data.randomBetweenTwoStartDelays_           = jsonData["RandomBetweenTwoStartDelays"];
    data.randomBetweenTwoPositions_             = jsonData["RandomBetweenTwoPositions"];
    data.randomBetweenTwoVelocities_            = jsonData["RandomBetweenTwoVelocities"];
    data.randomBetweenTwoAccelerations_         = jsonData["RandomBetweenTwoAccelerations"];
    data.randomBetweenTwoGravities_             = jsonData["RandomBetweenTwoGravities"];
    data.randomBetweenTwoRotations_             = jsonData["RandomBetweenTwoRotations"];
    data.randomBetweenTwoRotationVelocities_    = jsonData["RandomBetweenTwoRotationVelocities"];
    data.randomBetweenTwoRotationAccelerations_ = jsonData["RandomBetweenTwoRotationAccelerations"];
    data.randomBetweenTwoScales_                = jsonData["RandomBetweenTwoScales"];
    data.randomBetweenTwoScaleVelocities_       = jsonData["RandomBetweenTwoScaleVelocities"];
    data.randomBetweenTwoScaleAccelerations_    = jsonData["RandomBetweenTwoScaleAccelerations"];
    data.randomBetweenTwoColors_                = jsonData["RandomBetweenTwoColors"];

    computeParticleData_.emplace_back(data);
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

    if (ImGui::TreeNodeEx("Asset Action", ImGuiTreeNodeFlags_Framed))
    {
        static char filename[128] = "";
        ImGui::InputText("Asset Name", filename, ARRAYSIZE(filename));
        computeParticleEmitData_.name_ = filename;

        if (ImGui::Button("Export Particle Asset"))
        {
            AssetCreation(computeParticleEmitData_, computeParticleEmitData_.name_);
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Emit Particle From JsonData", ImGuiTreeNodeFlags_Framed))
    {
        static char filename[128] = "";
        ImGui::InputText("Asset Name", filename, ARRAYSIZE(filename));

        if (ImGui::Button("Emit Particle"))
        {
            EmitParticle(filename);
        }

        ImGui::TreePop();
    }

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
        ImGui::Checkbox("Random Between Two Constants", &computeParticleEmitData_.randomBetweenTwoStartDelays_);

        if (computeParticleEmitData_.randomBetweenTwoStartDelays_)
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

void ComputeParticleSystem::EmitParticle(const std::string& effectName)
{
    for (int i = 0; i < computeParticleData_.size(); ++i)
    {
        if (effectName == computeParticleData_.at(i).name_)
        {
            computeParticleEmitData_ = computeParticleData_.at(i);
            EmitParticle();

            return;
        }
    }

    // 失敗
    return;
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

        if (computeParticleEmitData_.randomBetweenTwoStartDelays_)
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

void ComputeParticleSystem::AssetCreation(const ComputeParticleEmitData& data, const std::string& filename)
{
    nlohmann::json jsonData;

    jsonData["EffectName"]      = data.name_;
    jsonData["EmitParticleNum"] = data.emitParticleNum_;
    jsonData["TextureType"]     = data.textureType_;
    jsonData["Duration"]        = data.duration_;
    jsonData["LifeTimeMin"]     = data.lifeTimeMin_;
    jsonData["LifeTimeMax"]     = data.lifeTimeMax_;
    jsonData["StartDelayMin"]   = data.startDelayMin_;
    jsonData["StartDelayMax"]   = data.startDelayMax_;
    jsonData["GravityMin"]      = data.gravityMin_;
    jsonData["GravityMax"]      = data.gravityMax_;

    jsonData["PositionMin"]             = { data.positionMin_.x, data.positionMin_.y, data.positionMin_.z, data.positionMin_.w };
    jsonData["PositionMax"]             = { data.positionMax_.x, data.positionMax_.y, data.positionMax_.z, data.positionMax_.w };
    jsonData["VelocityMin"]             = { data.velocityMin_.x, data.velocityMin_.y, data.velocityMin_.z, data.velocityMin_.w };
    jsonData["VelocityMax"]             = { data.velocityMax_.x, data.velocityMax_.y, data.velocityMax_.z, data.velocityMax_.w };
    jsonData["AccelerationMin"]         = { data.accelerationMin_.x, data.accelerationMin_.y,data.accelerationMin_.z, data.accelerationMin_.w };
    jsonData["AccelerationMax"]         = { data.accelerationMax_.x, data.accelerationMax_.y,data.accelerationMax_.z, data.accelerationMax_.w };
    jsonData["RotationMin"]             = { data.rotationMin_.x, data.rotationMin_.y, data.rotationMin_.z, data.rotationMin_.w };
    jsonData["RotationMax"]             = { data.rotationMax_.x, data.rotationMax_.y, data.rotationMax_.z, data.rotationMax_.w };
    jsonData["RotationVelocityMin"]     = { data.rotationVelocityMin_.x, data.rotationVelocityMin_.y, data.rotationVelocityMin_.z, data.rotationVelocityMin_.w };
    jsonData["RotationVelocityMax"]     = { data.rotationVelocityMax_.x, data.rotationVelocityMax_.y, data.rotationVelocityMax_.z, data.rotationVelocityMax_.w };
    jsonData["RotationAccelerationMin"] = { data.rotationAccelerationMin_.x, data.rotationAccelerationMin_.y, data.rotationAccelerationMin_.z, data.rotationAccelerationMin_.w };
    jsonData["RotationAccelerationMax"] = { data.rotationAccelerationMax_.x, data.rotationAccelerationMax_.y, data.rotationAccelerationMax_.z, data.rotationAccelerationMax_.w };
    jsonData["ScaleMin"]                = { data.scaleMin_.x, data.scaleMin_.y, data.scaleMin_.z, data.scaleMin_.w };
    jsonData["ScaleMax"]                = { data.scaleMax_.x, data.scaleMax_.y, data.scaleMax_.z, data.scaleMax_.w };
    jsonData["ScaleVelocityMin"]        = { data.scaleVelocityMin_.x, data.scaleVelocityMin_.y, data.scaleVelocityMin_.z, data.scaleVelocityMin_.w };
    jsonData["ScaleVelocityMax"]        = { data.scaleVelocityMax_.x, data.scaleVelocityMax_.y, data.scaleVelocityMax_.z, data.scaleVelocityMax_.w };
    jsonData["ScaleAccelerationMin"]    = { data.scaleAccelerationMin_.x, data.scaleAccelerationMin_.y, data.scaleAccelerationMin_.z, data.scaleAccelerationMin_.w };
    jsonData["ScaleAccelerationMax"]    = { data.scaleAccelerationMax_.x, data.scaleAccelerationMax_.y, data.scaleAccelerationMax_.z, data.scaleAccelerationMax_.w };
    jsonData["StartColorMin"]           = { data.startColorMin_.x, data.startColorMin_.y, data.startColorMin_.z, data.startColorMin_.w };
    jsonData["StartColorMax"]           = { data.startColorMax_.x, data.startColorMax_.y, data.startColorMax_.z, data.startColorMax_.w };
    jsonData["EndColor"]                = { data.endColor_.x, data.endColor_.y, data.endColor_.z, data.endColor_.w };

    jsonData["RandomBetweenTwoLifeTimes"]               = data.randomBetweenTwoLifeTimes_;
    jsonData["RandomBetweenTwoStartDelays"]             = data.randomBetweenTwoStartDelays_;    
    jsonData["RandomBetweenTwoPositions"]               = data.randomBetweenTwoPositions_;
    jsonData["RandomBetweenTwoVelocities"]              = data.randomBetweenTwoVelocities_;
    jsonData["RandomBetweenTwoAccelerations"]           = data.randomBetweenTwoAccelerations_;
    jsonData["RandomBetweenTwoGravities"]               = data.randomBetweenTwoGravities_;
    jsonData["RandomBetweenTwoRotations"]               = data.randomBetweenTwoRotations_;
    jsonData["RandomBetweenTwoRotationVelocities"]      = data.randomBetweenTwoRotationVelocities_;
    jsonData["RandomBetweenTwoRotationAccelerations"]   = data.randomBetweenTwoRotationAccelerations_;
    jsonData["RandomBetweenTwoScales"]                  = data.randomBetweenTwoScales_;
    jsonData["RandomBetweenTwoScaleVelocities"]         = data.randomBetweenTwoScaleVelocities_;
    jsonData["RandomBetweenTwoScaleAccelerations"]      = data.randomBetweenTwoScaleAccelerations_;
    jsonData["RandomBetweenTwoColors"]                  = data.randomBetweenTwoColors_;

    std::ofstream writingFile;
    std::string filepath = "./Resources/JsonParameters/ComputeParticle/" + filename;
    writingFile.open(filepath, std::ios::out);
    writingFile << jsonData.dump() << std::endl;
    writingFile.close();
}
