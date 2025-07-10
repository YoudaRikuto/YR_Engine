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

    data.position_ = { jsonData["Position"][0], jsonData["Position"][1], jsonData["Position"][2] };
    data.scale_ = { jsonData["Scale"][0], jsonData["Scale"][1], jsonData["Scale"][2] };
    data.rotation_ = { jsonData["Rotation"][0], jsonData["Rotation"][1], jsonData["Rotation"][2] };

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
            ImGui::OpenPopup("Export Check");
        }

        if (ImGui::BeginPopupModal("Export Check", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Export Particle Asset??");

            if (ImGui::Button("Yes!", ImVec2(150, 150)))
            {
                AssetCreation();

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(150, 150)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Set EmitParameter From JsonData", ImGuiTreeNodeFlags_Framed))
    {
        static char filename[128] = "";
        ImGui::InputText("Asset Name", filename, ARRAYSIZE(filename));

        if(ImGui::Button("Set EmitData")) SetEmitData(filename);

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

    transform_.DrawDebug();

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
            
            transform_.SetPosition(computeParticleEmitData_.position_);
            transform_.SetScale(computeParticleEmitData_.scale_);
            transform_.SetRotation(computeParticleEmitData_.rotation_);
            
            EmitParticle();

            return;
        }
    }

    // 失敗
    return;
}

void ComputeParticleSystem::EmitParticle(const std::string& effectName, const Transform3D& transform)
{
    for (int i = 0; i < computeParticleData_.size(); ++i)
    {
        if (effectName == computeParticleData_.at(i).name_)
        {
            computeParticleEmitData_ = computeParticleData_.at(i);

            transform_.SetPosition(transform.GetPosition());
            transform_.SetScale(transform.GetScale());
            transform_.SetRotation(transform.GetRotation());

            EmitParticle();

            return;
        }
    }

    // 失敗
    return;
}

void ComputeParticleSystem::SetEmitData(const std::string& filename)
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

    data.positionMin_ = { jsonData["PositionMin"][0], jsonData["PositionMin"][1], jsonData["PositionMin"][2], jsonData["PositionMin"][3] };
    data.positionMax_ = { jsonData["PositionMax"][0], jsonData["PositionMax"][1], jsonData["PositionMax"][2], jsonData["PositionMax"][3] };
    data.velocityMin_ = { jsonData["VelocityMin"][0], jsonData["VelocityMin"][1], jsonData["VelocityMin"][2], jsonData["VelocityMin"][3] };
    data.velocityMax_ = { jsonData["VelocityMax"][0], jsonData["VelocityMax"][1], jsonData["VelocityMax"][2], jsonData["VelocityMax"][3] };
    data.accelerationMin_ = { jsonData["AccelerationMin"][0], jsonData["AccelerationMin"][1], jsonData["AccelerationMin"][2], jsonData["AccelerationMin"][3] };
    data.accelerationMax_ = { jsonData["AccelerationMax"][0], jsonData["AccelerationMax"][1], jsonData["AccelerationMax"][2], jsonData["AccelerationMax"][3] };
    data.rotationMin_ = { jsonData["RotationMin"][0], jsonData["RotationMin"][1], jsonData["RotationMin"][2], jsonData["RotationMin"][3] };
    data.rotationMax_ = { jsonData["RotationMax"][0], jsonData["RotationMax"][1], jsonData["RotationMax"][2], jsonData["RotationMax"][3] };
    data.rotationVelocityMin_ = { jsonData["RotationVelocityMin"][0], jsonData["RotationVelocityMin"][1], jsonData["RotationVelocityMin"][2], jsonData["RotationVelocityMin"][3] };
    data.rotationVelocityMax_ = { jsonData["RotationVelocityMax"][0], jsonData["RotationVelocityMax"][1], jsonData["RotationVelocityMax"][2], jsonData["RotationVelocityMax"][3] };
    data.rotationAccelerationMin_ = { jsonData["RotationAccelerationMin"][0], jsonData["RotationAccelerationMin"][1], jsonData["RotationAccelerationMin"][2], jsonData["RotationAccelerationMin"][3] };
    data.rotationAccelerationMax_ = { jsonData["RotationAccelerationMax"][0], jsonData["RotationAccelerationMax"][1], jsonData["RotationAccelerationMax"][2], jsonData["RotationAccelerationMax"][3] };
    data.scaleMin_ = { jsonData["ScaleMin"][0], jsonData["ScaleMin"][1], jsonData["ScaleMin"][2], jsonData["ScaleMin"][3] };
    data.scaleMax_ = { jsonData["ScaleMax"][0], jsonData["ScaleMax"][1], jsonData["ScaleMax"][2], jsonData["ScaleMax"][3] };
    data.scaleVelocityMin_ = { jsonData["ScaleVelocityMin"][0], jsonData["ScaleVelocityMin"][1], jsonData["ScaleVelocityMin"][2], jsonData["ScaleVelocityMin"][3] };
    data.scaleVelocityMax_ = { jsonData["ScaleVelocityMax"][0], jsonData["ScaleVelocityMax"][1], jsonData["ScaleVelocityMax"][2], jsonData["ScaleVelocityMax"][3] };
    data.scaleAccelerationMin_ = { jsonData["ScaleAccelerationMin"][0], jsonData["ScaleAccelerationMin"][1], jsonData["ScaleAccelerationMin"][2], jsonData["ScaleAccelerationMin"][3] };
    data.scaleAccelerationMax_ = { jsonData["ScaleAccelerationMax"][0], jsonData["ScaleAccelerationMax"][1], jsonData["ScaleAccelerationMax"][2], jsonData["ScaleAccelerationMax"][3] };
    data.startColorMin_ = { jsonData["StartColorMin"][0], jsonData["StartColorMin"][1], jsonData["StartColorMin"][2], jsonData["StartColorMin"][3] };
    data.startColorMax_ = { jsonData["StartColorMax"][0], jsonData["StartColorMax"][1], jsonData["StartColorMax"][2], jsonData["StartColorMax"][3] };
    data.endColor_ = { jsonData["EndColor"][0], jsonData["EndColor"][1], jsonData["EndColor"][2], jsonData["EndColor"][3] };

    data.randomBetweenTwoLifeTimes_ = jsonData["RandomBetweenTwoLifeTimes"];
    data.randomBetweenTwoStartDelays_ = jsonData["RandomBetweenTwoStartDelays"];
    data.randomBetweenTwoPositions_ = jsonData["RandomBetweenTwoPositions"];
    data.randomBetweenTwoVelocities_ = jsonData["RandomBetweenTwoVelocities"];
    data.randomBetweenTwoAccelerations_ = jsonData["RandomBetweenTwoAccelerations"];
    data.randomBetweenTwoGravities_ = jsonData["RandomBetweenTwoGravities"];
    data.randomBetweenTwoRotations_ = jsonData["RandomBetweenTwoRotations"];
    data.randomBetweenTwoRotationVelocities_ = jsonData["RandomBetweenTwoRotationVelocities"];
    data.randomBetweenTwoRotationAccelerations_ = jsonData["RandomBetweenTwoRotationAccelerations"];
    data.randomBetweenTwoScales_ = jsonData["RandomBetweenTwoScales"];
    data.randomBetweenTwoScaleVelocities_ = jsonData["RandomBetweenTwoScaleVelocities"];
    data.randomBetweenTwoScaleAccelerations_ = jsonData["RandomBetweenTwoScaleAccelerations"];
    data.randomBetweenTwoColors_ = jsonData["RandomBetweenTwoColors"];

    computeParticleEmitData_ = data;

    data.position_ = { jsonData["Position"][0], jsonData["Position"][1], jsonData["Position"][2] };
    data.scale_ = { jsonData["Scale"][0], jsonData["Scale"][1], jsonData["Scale"][2] };
    data.rotation_ = { jsonData["Rotation"][0], jsonData["Rotation"][1], jsonData["Rotation"][2] };
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
            // 粒子生成位置を決定
            const DirectX::XMFLOAT4 position = XMFloat4RandomRange(computeParticleEmitData_.positionMin_, computeParticleEmitData_.positionMax_);
            
            // 親子関係を付けている
            emitParticleData.position_ =
            {
                position.x + transform_.GetPositionX(),
                position.y + transform_.GetPositionY(),
                position.z + transform_.GetPositionZ(),
                position.w
            };
        }
        else
        {
            // 粒子生成位置を決定
            const DirectX::XMFLOAT4 position = computeParticleEmitData_.positionMin_;

            // 親子関係を付けている
            emitParticleData.position_ =
            {
                position.x + transform_.GetPositionX(),
                position.y + transform_.GetPositionY(),
                position.z + transform_.GetPositionZ(),
                position.w
            };
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

        DirectX::XMStoreFloat4x4(&emitParticleData.world_, transform_.CalcWorldMatrix(1.0f));

        EmitParticle(emitParticleData);
    }
}

// Asset Export (Json Data)
void ComputeParticleSystem::AssetCreation()
{
    nlohmann::json jsonData;
        
    jsonData["EffectName"]      = computeParticleEmitData_.name_;
    jsonData["EmitParticleNum"] = computeParticleEmitData_.emitParticleNum_;
    jsonData["TextureType"]     = computeParticleEmitData_.textureType_;
    jsonData["Duration"]        = computeParticleEmitData_.duration_;
    jsonData["LifeTimeMin"]     = computeParticleEmitData_.lifeTimeMin_;
    jsonData["LifeTimeMax"]     = computeParticleEmitData_.lifeTimeMax_;
    jsonData["StartDelayMin"]   = computeParticleEmitData_.startDelayMin_;
    jsonData["StartDelayMax"]   = computeParticleEmitData_.startDelayMax_;
    jsonData["GravityMin"]      = computeParticleEmitData_.gravityMin_;
    jsonData["GravityMax"]      = computeParticleEmitData_.gravityMax_;
    jsonData["PositionMin"] = 
    {
        computeParticleEmitData_.positionMin_.x,
        computeParticleEmitData_.positionMin_.y,
        computeParticleEmitData_.positionMin_.z,
        computeParticleEmitData_.positionMin_.w 
    };
    jsonData["PositionMax"] =
    {
        computeParticleEmitData_.positionMax_.x,
        computeParticleEmitData_.positionMax_.y,
        computeParticleEmitData_.positionMax_.z,
        computeParticleEmitData_.positionMax_.w
    };
    jsonData["VelocityMin"] =
    {
        computeParticleEmitData_.velocityMin_.x,
        computeParticleEmitData_.velocityMin_.y,
        computeParticleEmitData_.velocityMin_.z,
        computeParticleEmitData_.velocityMin_.w 
    };
    jsonData["VelocityMax"] =
    {
        computeParticleEmitData_.velocityMax_.x, 
        computeParticleEmitData_.velocityMax_.y,
        computeParticleEmitData_.velocityMax_.z, 
        computeParticleEmitData_.velocityMax_.w 
    };
    jsonData["AccelerationMin"] = 
    {
        computeParticleEmitData_.accelerationMin_.x,
        computeParticleEmitData_.accelerationMin_.y,
        computeParticleEmitData_.accelerationMin_.z,
        computeParticleEmitData_.accelerationMin_.w 
    };
    jsonData["AccelerationMax"] =
    {
        computeParticleEmitData_.accelerationMax_.x,
        computeParticleEmitData_.accelerationMax_.y,
        computeParticleEmitData_.accelerationMax_.z, 
        computeParticleEmitData_.accelerationMax_.w
    };
    jsonData["RotationMin"] =
    { 
        computeParticleEmitData_.rotationMin_.x,
        computeParticleEmitData_.rotationMin_.y,
        computeParticleEmitData_.rotationMin_.z,
        computeParticleEmitData_.rotationMin_.w 
    };
    jsonData["RotationMax"] =
    {
        computeParticleEmitData_.rotationMax_.x,
        computeParticleEmitData_.rotationMax_.y,
        computeParticleEmitData_.rotationMax_.z, 
        computeParticleEmitData_.rotationMax_.w 
    };
    jsonData["RotationVelocityMin"] = 
    {
        computeParticleEmitData_.rotationVelocityMin_.x,
        computeParticleEmitData_.rotationVelocityMin_.y,
        computeParticleEmitData_.rotationVelocityMin_.z,
        computeParticleEmitData_.rotationVelocityMin_.w 
    };
    jsonData["RotationVelocityMax"] =
    { 
        computeParticleEmitData_.rotationVelocityMax_.x,
        computeParticleEmitData_.rotationVelocityMax_.y,
        computeParticleEmitData_.rotationVelocityMax_.z,
        computeParticleEmitData_.rotationVelocityMax_.w 
    };
    jsonData["RotationAccelerationMin"] = 
    {
        computeParticleEmitData_.rotationAccelerationMin_.x,
        computeParticleEmitData_.rotationAccelerationMin_.y, 
        computeParticleEmitData_.rotationAccelerationMin_.z,
        computeParticleEmitData_.rotationAccelerationMin_.w 
    };
    jsonData["RotationAccelerationMax"] = 
    { 
        computeParticleEmitData_.rotationAccelerationMax_.x,
        computeParticleEmitData_.rotationAccelerationMax_.y,
        computeParticleEmitData_.rotationAccelerationMax_.z,
        computeParticleEmitData_.rotationAccelerationMax_.w 
    };
    jsonData["ScaleMin"] = 
    { 
        computeParticleEmitData_.scaleMin_.x,
        computeParticleEmitData_.scaleMin_.y,
        computeParticleEmitData_.scaleMin_.z,
        computeParticleEmitData_.scaleMin_.w 
    };
    jsonData["ScaleMax"] = 
    {
        computeParticleEmitData_.scaleMax_.x,
        computeParticleEmitData_.scaleMax_.y,
        computeParticleEmitData_.scaleMax_.z, 
        computeParticleEmitData_.scaleMax_.w 
    };
    jsonData["ScaleVelocityMin"] = 
    { 
        computeParticleEmitData_.scaleVelocityMin_.x,
        computeParticleEmitData_.scaleVelocityMin_.y, 
        computeParticleEmitData_.scaleVelocityMin_.z,
        computeParticleEmitData_.scaleVelocityMin_.w 
    };
    jsonData["ScaleVelocityMax"] = 
    { 
        computeParticleEmitData_.scaleVelocityMax_.x,
        computeParticleEmitData_.scaleVelocityMax_.y, 
        computeParticleEmitData_.scaleVelocityMax_.z, 
        computeParticleEmitData_.scaleVelocityMax_.w 
    };
    jsonData["ScaleAccelerationMin"] = 
    { 
        computeParticleEmitData_.scaleAccelerationMin_.x,
        computeParticleEmitData_.scaleAccelerationMin_.y, 
        computeParticleEmitData_.scaleAccelerationMin_.z,
        computeParticleEmitData_.scaleAccelerationMin_.w 
    };
    jsonData["ScaleAccelerationMax"] =
    { 
        computeParticleEmitData_.scaleAccelerationMax_.x,
        computeParticleEmitData_.scaleAccelerationMax_.y,
        computeParticleEmitData_.scaleAccelerationMax_.z, 
        computeParticleEmitData_.scaleAccelerationMax_.w 
    };
    jsonData["StartColorMin"] = 
    { 
        computeParticleEmitData_.startColorMin_.x,
        computeParticleEmitData_.startColorMin_.y,
        computeParticleEmitData_.startColorMin_.z, 
        computeParticleEmitData_.startColorMin_.w 
    };
    jsonData["StartColorMax"] =
    {
        computeParticleEmitData_.startColorMax_.x, 
        computeParticleEmitData_.startColorMax_.y,
        computeParticleEmitData_.startColorMax_.z,
        computeParticleEmitData_.startColorMax_.w 
    };
    jsonData["EndColor"] = 
    { 
        computeParticleEmitData_.endColor_.x,
        computeParticleEmitData_.endColor_.y, 
        computeParticleEmitData_.endColor_.z, 
        computeParticleEmitData_.endColor_.w
    };
    jsonData["RandomBetweenTwoLifeTimes"]               = computeParticleEmitData_.randomBetweenTwoLifeTimes_;
    jsonData["RandomBetweenTwoStartDelays"]             = computeParticleEmitData_.randomBetweenTwoStartDelays_;    
    jsonData["RandomBetweenTwoPositions"]               = computeParticleEmitData_.randomBetweenTwoPositions_;
    jsonData["RandomBetweenTwoVelocities"]              = computeParticleEmitData_.randomBetweenTwoVelocities_;
    jsonData["RandomBetweenTwoAccelerations"]           = computeParticleEmitData_.randomBetweenTwoAccelerations_;
    jsonData["RandomBetweenTwoGravities"]               = computeParticleEmitData_.randomBetweenTwoGravities_;
    jsonData["RandomBetweenTwoRotations"]               = computeParticleEmitData_.randomBetweenTwoRotations_;
    jsonData["RandomBetweenTwoRotationVelocities"]      = computeParticleEmitData_.randomBetweenTwoRotationVelocities_;
    jsonData["RandomBetweenTwoRotationAccelerations"]   = computeParticleEmitData_.randomBetweenTwoRotationAccelerations_;
    jsonData["RandomBetweenTwoScales"]                  = computeParticleEmitData_.randomBetweenTwoScales_;
    jsonData["RandomBetweenTwoScaleVelocities"]         = computeParticleEmitData_.randomBetweenTwoScaleVelocities_;
    jsonData["RandomBetweenTwoScaleAccelerations"]      = computeParticleEmitData_.randomBetweenTwoScaleAccelerations_;
    jsonData["RandomBetweenTwoColors"]                  = computeParticleEmitData_.randomBetweenTwoColors_;

    jsonData["Position"] =
    {
        transform_.GetPositionX(),
        transform_.GetPositionY(),
        transform_.GetPositionZ()
    };
    jsonData["Scale"] =
    {
        transform_.GetScaleX(),
        transform_.GetScaleY(),
        transform_.GetScaleZ()
    };
    jsonData["Rotation"] =
    {
        transform_.GetRotationX(),
        transform_.GetRotationY(),
        transform_.GetRotationZ()
    };

    std::ofstream writingFile;
    std::string filepath = "./Resources/JsonParameters/ComputeParticle/" + computeParticleEmitData_.name_;
    writingFile.open(filepath, std::ios::out);
    writingFile << jsonData.dump() << std::endl;
    writingFile.close();
}
