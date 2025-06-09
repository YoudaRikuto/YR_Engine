#include "ComputeParticleSystem.h"
#include "Graphics/Graphics.h"
#include "FrameWork/Misc.h"
#include "Resource/Texture.h"
#include "ImGui/ImGuiCtrl.h"

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
        for (int i = 0; i < emitParticleData_.emitParticleNum_; ++i)
        {
            EmitParticle();
        }
    }


    ImGui::DragInt("ParticleNum", &emitParticleData_.emitParticleNum_, 1, 0, 10000);
    ImGui::SliderInt("TextureType", &emitParticleData_.textureType_, 0, 16);
    ImGui::DragFloat("Duration", &emitParticleData_.duration_, 0.1f, 0.0f, 50.0f);

    // ---------- LifeTime ----------
    ImGui::Separator();
    ImGui::Text("LifeTime");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##LifeTime Random", &emitParticleData_.randomBetweenTwoLifeTimes_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwoLifeTimes_ == 0)
    {
        ImGui::DragFloat("LifeTime", &emitParticleData_.lifeTimeMin_, 0.1f, 0.0f, 50.0f);
    }
    else
    {
        ImGui::PushItemWidth(150);
        ImGui::DragFloat("##LifeTimeMin", &emitParticleData_.lifeTimeMin_, 0.1f, 0.0f, 50.0f);
        ImGui::SameLine();
        ImGui::DragFloat("##LifeTimeMax", &emitParticleData_.lifeTimeMax_, 0.1f, 0.0f, 50.0f);
        ImGui::SameLine();
        ImGui::Text("LifeTime");
        ImGui::PopItemWidth();
    }

    // ---------- StartDelay ----------
    ImGui::Separator();
    ImGui::Text("StartDelay");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##StartDelay Random", &emitParticleData_.randomBetweenTwostartDelays_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwostartDelays_ == 0)
    {
        ImGui::DragFloat("StartDelay", &emitParticleData_.startDelayMin_, 0.1f, 0.0f, 50.0f);
    }
    else
    {
        ImGui::PushItemWidth(150);
        ImGui::DragFloat("##StartDelayMin", &emitParticleData_.startDelayMin_, 0.1f, 0.0f, 50.0f);
        ImGui::SameLine();
        ImGui::DragFloat("##StartDelayMax", &emitParticleData_.startDelayMax_, 0.1f, 0.0f, 50.0f);
        ImGui::SameLine();
        ImGui::Text("StartDelay");
        ImGui::PopItemWidth();
    }

    // ---------- Gravity ----------
    ImGui::Separator();
    ImGui::Text("Gravity");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##Gravity Random", &emitParticleData_.randomBetweenTwoGravities_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwoGravities_ == 0)
    {
        ImGui::DragFloat("Gravity", &emitParticleData_.gravityMin_, 0.1f);
    }
    else
    {
        ImGui::PushItemWidth(150);
        ImGui::DragFloat("##GravityMin", &emitParticleData_.gravityMin_, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("##GravityMax", &emitParticleData_.gravityMax_, 0.1f);
        ImGui::SameLine();
        ImGui::Text("Gravity");
        ImGui::PopItemWidth();
    }

    // ---------- Position ----------
    ImGui::Separator();
    ImGui::Text("Position");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##Position Random", &emitParticleData_.randomBetweenTwoPositions_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwoPositions_ == 0)
    {
        ImGui::DragFloat4("Position", &emitParticleData_.positionMin_.x);
    }
    else
    {
        ImGui::DragFloat4("Position Min", &emitParticleData_.positionMin_.x);
        ImGui::DragFloat4("Position Max", &emitParticleData_.positionMax_.x);
    }

    // ---------- Velocity ----------
    ImGui::Separator();
    ImGui::Text("Velocity");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##Velocity Random", &emitParticleData_.randomBetweenTwovelocities_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwovelocities_ == 0)
    {
        ImGui::DragFloat4("Velocity", &emitParticleData_.velocityMin_.x);
    }
    else
    {
        ImGui::DragFloat4("Velocity Min", &emitParticleData_.velocityMin_.x);
        ImGui::DragFloat4("Velocity Max", &emitParticleData_.velocityMax_.x);
    }

    // ---------- Acceleration ----------
    ImGui::Separator();
    ImGui::Text("Acceleration");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##Acceleration Random", &emitParticleData_.randomBetweenTwoAccelerations_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwoAccelerations_ == 0)
    {
        ImGui::DragFloat4("Acceleration", &emitParticleData_.accelerationMin_.x);
    }
    else
    {
        ImGui::DragFloat4("Acceleration Min", &emitParticleData_.accelerationMin_.x);
        ImGui::DragFloat4("Acceleration Max", &emitParticleData_.accelerationMax_.x);
    }

    // ---------- Scale ----------
    ImGui::Separator();
    ImGui::Text("Scale");
    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("##Scale Random", &emitParticleData_.randomBetweenTwoScales_, 0, 1);
    ImGui::SameLine();
    ImGui::Text("Random Between Two Constants");
    if (emitParticleData_.randomBetweenTwoScales_ == 0)
    {
        ImGui::DragFloat4("Scale", &emitParticleData_.scaleMin_.x);
    }
    else
    {
        ImGui::DragFloat4("Scale Min", &emitParticleData_.scaleMin_.x);
        ImGui::DragFloat4("Scale Max", &emitParticleData_.scaleMax_.x);
    }

    ImGui::End();
}

void ComputeParticleSystem::EmitParticle()
{
    if (emitParticles_.size() >= numEmitParticles_) return;

    emitParticles_.emplace_back(emitParticleData_);
}
