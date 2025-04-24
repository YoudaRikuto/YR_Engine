#include "GltfModel.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERANL_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <filesystem>
#include <fstream>
#include "tinygltf/tiny_gltf.h"
#include "Resource/Texture.h"   

// ----- コンストラクタ -----
GltfModel::GltfModel(const std::string& filename, const std::string& rootNodeName)
    : filename_(filename),
    rootNodeName_(rootNodeName)
{
    std::filesystem::path cerealFilename(filename);
    cerealFilename.replace_extension("cereal");
    if (std::filesystem::exists(cerealFilename.c_str()))
    {
        // CerealデータからModel情報読み込み
        LoadGltfModelFromCereal();
    }
    else
    {
        // FilenameからModel情報読み込み
        LoadGltfModelFromFilename();
    }

    const std::map<std::string, BufferView>& vertexBufferViews = meshes_.at(0).primitives_.at(0).vertexBufferViews_;
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION", 0, vertexBufferViews.at("POSITION").format_,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, vertexBufferViews.at("NORMAL").format_,     1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, vertexBufferViews.at("TANGENT").format_,    2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, vertexBufferViews.at("TEXCOORD_0").format_, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "JOINTS",   0, vertexBufferViews.at("JOINTS_0").format_,   4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WEIGHTS",  0, vertexBufferViews.at("WEIGHTS_0").format_,  5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "JOINTS",   1, vertexBufferViews.at("JOINTS_1").format_,   6, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WEIGHTS",  1, vertexBufferViews.at("WEIGHTS_1").format_,  7, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    Graphics::Instance().CreateVsFromCso("./Resources/Shader/GltfModelVS.cso", vertexShader_.ReleaseAndGetAddressOf(), inputLayout_.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    Graphics::Instance().CreatePsFromCso("./Resources/Shader/GltfModelPS.cso", pixelShader_.ReleaseAndGetAddressOf());

    primitiveConstants_ = std::make_unique<ConstantBuffer<PrimitiveConstants>>();
    jointConstants_ = std::make_unique<ConstantBuffer<JointConstants>>();

    animatedNodes_[0] = nodes_;
    animatedNodes_[1] = nodes_;
    zeroAnimatedNodes_ = nodes_;
}

// ----- 描画 -----
void GltfModel::Render(const float& scaleFactor, ID3D11PixelShader* psShader)
{
    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

    const DirectX::XMMATRIX W = transform_.CalcWorldMatrix(scaleFactor);
    DirectX::XMFLOAT4X4 world = {};
    DirectX::XMStoreFloat4x4(&world, W);

    deviceContext->PSSetShaderResources(0, 1, materialResourceView_.GetAddressOf());
    deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
    psShader ? deviceContext->PSSetShader(psShader, nullptr, 0) : deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);
    deviceContext->IASetInputLayout(inputLayout_.Get());
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::function<void(int)> traverse{ [&](int nodeIndex)->void {
        const Node& node = nodes_.at(nodeIndex);
        if (node.mesh_ > -1)
        {
            const Mesh& mesh = meshes_.at(node.mesh_);
            for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives_)
            {
                ID3D11Buffer* vertexBuffers[] =
                {
                    primitive.vertexBufferViews_.at("POSITION").buffer_.Get(),
                    primitive.vertexBufferViews_.at("NORMAL").buffer_.Get(),
                    primitive.vertexBufferViews_.at("TANGENT").buffer_.Get(),
                    primitive.vertexBufferViews_.at("TEXCOORD_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("JOINTS_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("WEIGHTS_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("JOINTS_1").buffer_.Get(),
                    primitive.vertexBufferViews_.at("WEIGHTS_1").buffer_.Get(),
                };
                UINT strides[] =
                {
                    static_cast<UINT>(primitive.vertexBufferViews_.at("POSITION").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("NORMAL").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("TANGENT").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("TEXCOORD_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_1").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_1").strideInBytes_),
                };
                UINT offsets[_countof(vertexBuffers)]{ 0 };
                deviceContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
                deviceContext->IASetIndexBuffer(primitive.indexBufferView_.buffer_.Get(), primitive.indexBufferView_.format_, 0);

                primitiveConstants_->GetData()->material_ = primitive.material_;
                primitiveConstants_->GetData()->hasTangent_ = primitive.vertexBufferViews_.at("TANGENT").buffer_ != NULL;
                primitiveConstants_->GetData()->skin_ = node.skin_;
                DirectX::XMStoreFloat4x4(&primitiveConstants_->GetData()->world_, DirectX::XMLoadFloat4x4(&node.globalTransform_) * DirectX::XMLoadFloat4x4(&world));
                primitiveConstants_->Activate(1, true, true, false, true);


                const Material& material = materials_.at(primitive.material_);
                const int textureIndices[] =
                {
                    material.data_.pbrMetallicRoughness_.baseColorTexture_.index_,
                    material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.index_,
                    material.data_.normalTexture_.index_,
                    material.data_.emissiveTexture_.index_,
                    material.data_.occlusionTexture_.index_,
                };
                ID3D11ShaderResourceView* nullShaderResourceView = {};
                std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
                for (size_t textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
                {
                    shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ?
                        textureResourceViews_.at(textures_.at(textureIndices[textureIndex]).source_).Get() : nullShaderResourceView;
                }
                deviceContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());


                if (node.skin_ > -1)
                {
                    const Skin& skin = skins_.at(node.skin_);
                    for (size_t jointIndex = 0; jointIndex < skin.joints_.size(); ++jointIndex)
                    {
                        DirectX::XMStoreFloat4x4(&jointConstants_->GetData()->matrices_[jointIndex],
                            DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices_.at(jointIndex)) *
                            DirectX::XMLoadFloat4x4(&nodes_.at(skin.joints_.at(jointIndex)).globalTransform_) *
                            DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform_)));
                    }
                    jointConstants_->Activate(2);
                }

                deviceContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView_.Count()), 0, 0);
            }
        }
        for (std::vector<int>::value_type childIndex : node.children_)
        {
            traverse(childIndex);
        }
    } };
    for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
    {
        traverse(nodeIndex);
    }
}

// ----- ImGui -----
void GltfModel::DrawDebug()
{
    transform_.DrawDebug();
}

// ----- アニメーション再生 -----
void GltfModel::PlayAnimation(const int& index, const bool& loop, const float& speed, const float& startFrame)
{
    animationIndex_     = index;
    animationSeconds_   = startFrame;
    animationSpeed_     = speed;
    isAnimationLoop_    = loop;
    isAnimationEnd_     = false;
    isAnimationBlend_   = false;
}

// ----- アニメーションブレンド再生 -----
void GltfModel::PlayAnimationBlend(const int& index, const bool& loop, const float& speed, const float& blendStartFrame, const float& transitionTime)
{
    if (isAnimationBlend_)
    {
        animatedNodes_[0] = nodes_;
    }
    else
    {
        Animate(animationIndex_, animationSeconds_, animatedNodes_[0]);
    }
    Animate(index, blendStartFrame, animatedNodes_[1]);

    animationIndex_     = index;
    animationSeconds_   = blendStartFrame;
    animationSpeed_     = speed;
    transitionTime_     = transitionTime_;
    isAnimationLoop_    = loop;
    isAnimationEnd_     = false;
    isAnimationBlend_   = true;
}


// ----- アニメーション更新 -----
void GltfModel::UpdateAnimation(const float& elapsedTime)
{
    // アニメーションブレンド
    if (UpdateAnimationBlend(elapsedTime)) return;

    animationSeconds_ += animationSpeed_ * elapsedTime;

    const float animationEndFrame = animations_.at(animationIndex_).duration_;

    if (animationSeconds_ > animationEndFrame)
    {
        if (isAnimationLoop_)
        {
            animationSeconds_ = 0.0f;
            
            return;
        }
        else
        {
            isAnimationEnd_ = true;

            return;
        }
    }

    Animate(animationIndex_, animationSeconds_, nodes_);
}

// ----- ルートモーション更新 -----
void GltfModel::UpdateRootMotion(const float& scaleFacter)
{
    if (isRootMotionActive_ == false) return;

    Node& node = nodes_.at(rootJointIndex_);
    
    DirectX::XMFLOAT3 position      = { node.globalTransform_._41, node.globalTransform_._42, node.globalTransform_._43 };
    DirectX::XMFLOAT3 displacement  = { position.x - previousPosition_.x, position.y - previousPosition_.y, position.z - previousPosition_.z };

    DirectX::XMFLOAT4X4 coordinateSystem = transform_.GetCoordinateSystemTransforms(Transform3D::CoordinateSystem::cRightYup);
    DirectX::XMMATRIX C = DirectX::XMLoadFloat4x4(&coordinateSystem) * DirectX::XMMatrixScaling(scaleFacter, scaleFacter, scaleFacter);
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(transform_.GetScale().x, transform_.GetScale().y, transform_.GetScale().z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(transform_.GetRotationX(), transform_.GetRotationY(), transform_.GetRotationZ());
    DirectX::XMStoreFloat3(&displacement, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&displacement), C * S * R));

    DirectX::XMFLOAT3 translation = transform_.GetPosition();
    translation = translation + displacement * rootMotionValue_;
    transform_.SetPosition(translation);

    node.globalTransform_._41 = zeroAnimatedNodes_.at(rootJointIndex_).globalTransform_._41;
    node.globalTransform_._42 = zeroAnimatedNodes_.at(rootJointIndex_).globalTransform_._42;
    node.globalTransform_._43 = zeroAnimatedNodes_.at(rootJointIndex_).globalTransform_._43;

    std::function<void(int, int)> traverse = [&](int parentIndex, int nodeIndex)
    {
        Node& node = nodes_.at(nodeIndex);
        if (parentIndex > -1)
        {
            DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale_.x, node.scale_.y, node.scale_.z);
            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotation_.x, node.rotation_.y, node.rotation_.z, node.rotation_.w));
            DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z);
            DirectX::XMStoreFloat4x4(&node.globalTransform_, S * R * T * DirectX::XMLoadFloat4x4(&nodes_.at(parentIndex).globalTransform_));
        }
        for (int childIndex : node.children_)
        {
            traverse(nodeIndex, childIndex);
        }
    };
    traverse(-1, rootJointIndex_);

    previousPosition_ = position;
}

// ----- ルートモーション使用設定 -----
void GltfModel::UseRootMotion(const bool& flag)
{
    isRootMotionActive_ = flag;

    if (flag)
    {
        Animate(animationIndex_, animationSeconds_, nodes_);
               
        Node& node = nodes_.at(rootJointIndex_);

        previousPosition_ = { node.globalTransform_._41, node.globalTransform_._42, node.globalTransform_._43 };
    }
}

// ----- 指定したジョイントの位置を取得 -----
const DirectX::XMFLOAT3 GltfModel::GetJointPosition(const size_t& nodeIndex, const float& scaleFactor, const DirectX::XMFLOAT3& offsetPosition)
{
    DirectX::XMFLOAT3 position = offsetPosition;

    const Node& node = nodes_.at(nodeIndex);
    DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform_) * GetTransform()->CalcWorldMatrix(scaleFactor);
    DirectX::XMStoreFloat3(&position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&position), M));

    return position;
}

// ----- 指定したジョイントの位置を取得 -----
const DirectX::XMFLOAT3 GltfModel::GetJointPosition(const std::string& nodeName, const float& scaleFactor, const DirectX::XMFLOAT3& offsetPosition)
{
    DirectX::XMFLOAT3 position = offsetPosition;

    // ノードを名前検索する
    for (Node& node : nodes_)
    {
        // 名前が一致しなかったら continue
        if (node.name_ != nodeName) continue;

        DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&node.globalTransform_) * GetTransform()->CalcWorldMatrix(scaleFactor);
        DirectX::XMStoreFloat3(&position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&position), M));

        return position;
    }

    // 見つからなかった。
    return DirectX::XMFLOAT3(0, 0, 0);
}

// ----- アニメーションブレンド -----
const bool GltfModel::UpdateAnimationBlend(const float& elapsedTime)
{
    // アニメーションブレンドを行わない
    if (isAnimationBlend_ == false) return false;

    const float weight = animationBlendSeconds_ / transitionTime_;

    const std::vector<Node>* nodes[2] = { &animatedNodes_[0], &animatedNodes_[1] };
    BlendAnimations(nodes, weight, nodes_);
    
    animationBlendSeconds_ += animationSpeed_ * elapsedTime;

    // 終了チェック
    if (weight > 1.0f)
    {
        animationBlendSeconds_  = 0.0f;
        isAnimationBlend_       = false;
    }

    return true;
}

// ----- ブレンド計算 -----
void GltfModel::BlendAnimations(const std::vector<Node>* nodes[2], const float& factor, std::vector<Node>& node)
{
    const size_t nodeCount = nodes[0]->size();
    node.resize(nodeCount);

    for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        DirectX::XMVECTOR S[2] =
        {
            DirectX::XMLoadFloat3(&nodes[0]->at(nodeIndex).scale_),
            DirectX::XMLoadFloat3(&nodes[1]->at(nodeIndex).scale_)
        };
        DirectX::XMStoreFloat3(&node.at(nodeIndex).scale_, DirectX::XMVectorLerp(S[0], S[1], factor));

        DirectX::XMVECTOR R[2] =
        {
            DirectX::XMLoadFloat4(&nodes[0]->at(nodeIndex).rotation_),
            DirectX::XMLoadFloat4(&nodes[1]->at(nodeIndex).rotation_)
        };
        DirectX::XMStoreFloat4(&node.at(nodeIndex).rotation_, DirectX::XMQuaternionSlerp(R[0], R[1], factor));

        DirectX::XMVECTOR T[2] =
        {
            DirectX::XMLoadFloat3(&nodes[0]->at(nodeIndex).translation_),
            DirectX::XMLoadFloat3(&nodes[1]->at(nodeIndex).translation_),
        };
        DirectX::XMStoreFloat3(&node.at(nodeIndex).translation_, DirectX::XMVectorLerp(T[0], T[1], factor));
    }

    CumulateTransforms(node);
}

void GltfModel::Animate(const int& animationIndex, const float& time, std::vector<Node>& animatedNodes)
{
    std::function<size_t(const std::vector<float>&, float, float&)> indexof
    {
        [](const std::vector<float>& timelines, float time, float& interpolationFactor)->size_t
        {
            const size_t keyframeCount = timelines.size();

            if (time > timelines.at(keyframeCount - 1))
            {
                interpolationFactor = 1.0f;
                return keyframeCount - 2;
            }
            else if (time < timelines.at(0))
            {
                interpolationFactor = 0.0f;
                return 0;
            }
            size_t keyframeIndex = 0;
            for (size_t timeIndex = 1; timeIndex < keyframeCount; ++timeIndex)
            {
                if (time < timelines.at(timeIndex))
                {
                    keyframeIndex = std::max<size_t>(0LL, timeIndex - 1);
                    break;
                }
            }
            interpolationFactor = (time - timelines.at(keyframeIndex + 0)) / (timelines.at(keyframeIndex + 1) - timelines.at(keyframeIndex + 0));
            return keyframeIndex;
        }
    };

    if (animations_.size() > 0)
    {
        const Animation& animation = animations_.at(animationIndex);
        for (std::vector<Animation::Channel>::const_reference channel : animation.channels_)
        {
            const Animation::Sampler& sampler = animation.samplers_.at(channel.sampler_);
            const std::vector<float>& timeline = animation.timelines_.at(sampler.input_);

            if (timeline.size() == 0) continue;

            float interpolationFactor = 0.0f;
            size_t keyframeIndex = indexof(timeline, time, interpolationFactor);

            if (channel.targetPath_ == "scale")
            {
                const std::vector<DirectX::XMFLOAT3>& scales = animation.scales_.at(sampler.output_);
                DirectX::XMStoreFloat3(&animatedNodes.at(channel.targetNode_).scale_,
                    DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&scales.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat3(&scales.at(keyframeIndex + 1)), interpolationFactor));
            }
            else if (channel.targetPath_ == "rotation")
            {
                const std::vector<DirectX::XMFLOAT4>& rotations = animation.rotations_.at(sampler.output_);
                DirectX::XMStoreFloat4(&animatedNodes.at(channel.targetNode_).rotation_,
                    DirectX::XMQuaternionNormalize(DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rotations.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat4(&rotations.at(keyframeIndex + 1)), interpolationFactor)));
            }
            else if (channel.targetPath_ == "translation")
            {
                const std::vector<DirectX::XMFLOAT3>& translations = animation.translations_.at(sampler.output_);
                DirectX::XMStoreFloat3(&animatedNodes.at(channel.targetNode_).translation_,
                    DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&translations.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat3(&translations.at(keyframeIndex + 1)), interpolationFactor));
            }
        }
        CumulateTransforms(animatedNodes);
    }
    else
    {
        animatedNodes = nodes_;
    }
}

// ----- FilenameからModel情報読み込み -----
void GltfModel::LoadGltfModelFromFilename()
{
    tinygltf::Model     gltfModel;
    tinygltf::TinyGLTF  tinyGltf;
    std::string         error, warning;
    bool                succeeded = false;

    if (filename_.find(".glb") != std::string::npos)
    {
        succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename_.c_str());
    }
    else if (filename_.find(".gltf") != std::string::npos)
    {
        succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename_.c_str());
    }
    _ASSERT_EXPR_A(warning.empty(), warning.c_str());
    _ASSERT_EXPR_A(error.empty(), warning.c_str());
    _ASSERT_EXPR_A(succeeded, L"Failed to load gltf file");

    for (std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
    {
        Scene& scene = scenes_.emplace_back();
        scene.name_ = gltfScene.name;
        scene.nodes_ = gltfScene.nodes;
    }

    FetchNodes(gltfModel);      // Node情報抽出
    FetchMeshes(gltfModel);     // Mesh情報抽出
    FetchMaterials(gltfModel);  // Material情報抽出
    FetchTexture(gltfModel);    // Texture情報抽出
    FetchAnimation(gltfModel);  // Animaiton情報抽出

    // Cerealデータに書き出し
    std::filesystem::path cerealFilename(filename_);
    cerealFilename.replace_extension("cereal");
    std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
    cereal::BinaryOutputArchive serialization(ofs);
    serialization(scenes_, nodes_, meshes_, materials_, textures_, images_, skins_, animations_, rootJointIndex_);
}

// ----- CerealデータからModel情報読み込み -----
void GltfModel::LoadGltfModelFromCereal()
{
    std::filesystem::path cerealFilename(filename_);
    cerealFilename.replace_extension("cereal");

    std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
    cereal::BinaryInputArchive deserialization(ifs);
    deserialization(scenes_, nodes_, meshes_, materials_, textures_, images_, skins_, animations_, rootJointIndex_);

    // Load Texture
    for (size_t imageIndex = 0; imageIndex < images_.size(); ++imageIndex)
    {
        const Texture::TextureData textureData = Texture::Instance().LoadTexture(images_.at(imageIndex).filename_.c_str());

        textureResourceViews_.emplace_back(textureData.shaderResourceView_);
    }

    for (size_t meshIndex = 0; meshIndex < meshes_.size(); ++meshIndex)
    {
        for (size_t primitiveIndex = 0; primitiveIndex < meshes_.at(meshIndex).primitives_.size(); ++primitiveIndex)
        {
            const BufferView& indexBufferView = meshes_.at(meshIndex).primitives_.at(primitiveIndex).indexBufferView_;
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = static_cast<UINT>(indexBufferView.sizeInBytes_);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pSysMem = indexBufferView.verticesBinary_.data();

            HRESULT result = S_OK;
            result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData,
                meshes_.at(meshIndex).primitives_.at(primitiveIndex).indexBufferView_.buffer_.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));


            for (auto& vertexBufferView : meshes_.at(meshIndex).primitives_.at(primitiveIndex).vertexBufferViews_)
            {
                if (static_cast<UINT>(vertexBufferView.second.sizeInBytes_) == 0) continue;

                bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.second.sizeInBytes_);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                subresourceData.pSysMem = vertexBufferView.second.verticesBinary_.data();

                result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData,
                    vertexBufferView.second.buffer_.ReleaseAndGetAddressOf());
                if (FAILED(result)) _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
            }
        }
    }

    std::vector<Material::Cbuffer> materialData;
    for (std::vector<Material>::const_reference material : materials_)
    {
        materialData.emplace_back(material.data_);
    }
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
    bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = materialData.data();

    HRESULT result = S_OK;
    result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, materialBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());

    result = Graphics::Instance().GetDevice()->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// ----- Node情報抽出 -----
void GltfModel::FetchNodes(const tinygltf::Model& gltfModel)
{
    int counter = 0;

    for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)
    {
        Node& node = nodes_.emplace_back();
        node.name_ = gltfNode.name;
        node.skin_ = gltfNode.skin;
        node.mesh_ = gltfNode.mesh;
        node.children_ = gltfNode.children;
        node.isRootNode_ = (gltfNode.name == rootNodeName_.c_str());

        // RootNodeを見つけ出す
        if (node.isRootNode_)
        {
            rootJointIndex_ = counter;
        }
        ++counter;

        if (gltfNode.matrix.empty() == false)
        {
            DirectX::XMFLOAT4X4 matrix = {};
            for (size_t row = 0; row < 4; ++row)
            {
                for (size_t column = 0; column < 4; ++column)
                {
                    matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
                }
            }

            DirectX::XMVECTOR S, R, T;
            const bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
            _ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

            DirectX::XMStoreFloat3(&node.scale_, S);
            DirectX::XMStoreFloat4(&node.rotation_, R);
            DirectX::XMStoreFloat3(&node.translation_, T);
        }
        else
        {
            if (gltfNode.scale.size() > 0)
            {
                node.scale_.x = static_cast<float>(gltfNode.scale.at(0));
                node.scale_.y = static_cast<float>(gltfNode.scale.at(1));
                node.scale_.z = static_cast<float>(gltfNode.scale.at(2));
            }
            if (gltfNode.translation.size() > 0)
            {
                node.translation_.x = static_cast<float>(gltfNode.translation.at(0));
                node.translation_.y = static_cast<float>(gltfNode.translation.at(1));
                node.translation_.z = static_cast<float>(gltfNode.translation.at(2));
            }
            if (gltfNode.rotation.size() > 0)
            {
                node.rotation_.x = static_cast<float>(gltfNode.rotation.at(0));
                node.rotation_.y = static_cast<float>(gltfNode.rotation.at(1));
                node.rotation_.z = static_cast<float>(gltfNode.rotation.at(2));
                node.rotation_.w = static_cast<float>(gltfNode.rotation.at(3));
            }
        }
    }
    CumulateTransforms(nodes_);
}

// ----- Mesh情報抽出 -----
void GltfModel::FetchMeshes(const tinygltf::Model& gltfModel)
{
    HRESULT result = S_OK;

    for (std::vector<tinygltf::Mesh>::const_reference gltfMesh : gltfModel.meshes)
    {
        Mesh& mesh = meshes_.emplace_back();
        mesh.name_ = gltfMesh.name;

        for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
        {
            Mesh::Primitive& primitive = mesh.primitives_.emplace_back();
            primitive.material_ = gltfPrimitive.material;

            const tinygltf::Accessor& gltfAccessorIndexBuffer = gltfModel.accessors.at(gltfPrimitive.indices);
            const tinygltf::BufferView& gltfBufferViewIndexBuffer = gltfModel.bufferViews.at(gltfAccessorIndexBuffer.bufferView);

            primitive.indexBufferView_ = MakeBufferView(gltfAccessorIndexBuffer);

            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = static_cast<UINT>(primitive.indexBufferView_.sizeInBytes_);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferViewIndexBuffer.buffer).data.data() + gltfBufferViewIndexBuffer.byteOffset + gltfAccessorIndexBuffer.byteOffset;

            primitive.indexBufferView_.verticesBinary_.resize(bufferDesc.ByteWidth);
            memcpy(primitive.indexBufferView_.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);

            result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, primitive.indexBufferView_.buffer_.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

            for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes)
            {
                tinygltf::Accessor          gltfAccessor = gltfModel.accessors.at(gltfAttribute.second);
                const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

                const void* buffer = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                std::vector<USHORT> joints0;
                std::vector<FLOAT>  weights0;
                if (gltfAttribute.first == "JOINTS_0")
                {
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count * 4; ++accessorIndex)
                        {
                            joints0.emplace_back(static_cast<USHORT>(data[accessorIndex]));
                        }
                        buffer = joints0.data();
                        gltfAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                    }
                }
                else if (gltfAttribute.first == "JOINTS_1")
                {
                    continue;
                }
                else if (gltfAttribute.first == "WEIGHTS_0")
                {
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count * 4; ++accessorIndex)
                        {
                            weights0.emplace_back(static_cast<FLOAT>(data[accessorIndex]) / 0xFF);
                        }
                        buffer = weights0.data();
                        gltfAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    }
                }
                else if (gltfAttribute.first == "WEIGHTS_1")
                {
                    continue;
                }

                BufferView vertexBufferView = MakeBufferView(gltfAccessor);

                bufferDesc = {};
                bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                subresourceData = {};
                subresourceData.pSysMem = buffer;

                vertexBufferView.verticesBinary_.resize(bufferDesc.ByteWidth);
                memcpy(vertexBufferView.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);

                result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

                primitive.vertexBufferViews_.emplace(std::make_pair(gltfAttribute.first, vertexBufferView));
            }

            const std::unordered_map<std::string, BufferView> attributes{
                { "TANGENT",    { DXGI_FORMAT_R32G32B32A32_FLOAT } },
                { "TEXCOORD_0", { DXGI_FORMAT_R32G32_FLOAT } },
                { "JOINTS_0",   { DXGI_FORMAT_R16G16B16A16_UINT } },
                { "WEIGHTS_0",  { DXGI_FORMAT_R32G32B32A32_FLOAT } },
                { "JOINTS_1",   { DXGI_FORMAT_R16G16B16A16_UINT } },
                { "WEIGHTS_1",  { DXGI_FORMAT_R32G32B32A32_FLOAT } },
            };

            for (std::unordered_map<std::string, BufferView>::const_reference attribute : attributes)
            {
                if (primitive.vertexBufferViews_.find(attribute.first) == primitive.vertexBufferViews_.end())
                {
                    primitive.vertexBufferViews_.insert(std::make_pair(attribute.first, attribute.second));
                }
            }
        }
    }
}

// ----- Material情報抽出 -----
void GltfModel::FetchMaterials(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Material>::const_reference gltfMaterial : gltfModel.materials)
    {
        std::vector<Material>::reference material = materials_.emplace_back();

        material.name_ = gltfMaterial.name;

        material.data_.emissiveFactor_[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
        material.data_.emissiveFactor_[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
        material.data_.emissiveFactor_[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

        material.data_.alphaMode_ = gltfMaterial.alphaMode == "OPAQUE" ? 0 : gltfMaterial.alphaMode == "MASK" ? 1 : gltfMaterial.alphaMode == "BLEND" ? 2 : 0;
        material.data_.alphaCutoff_ = static_cast<float>(gltfMaterial.alphaCutoff);
        material.data_.doubleSided_ = gltfMaterial.doubleSided ? 1 : 0;

        material.data_.pbrMetallicRoughness_.baseColorFactor_[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data_.pbrMetallicRoughness_.baseColorFactor_[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data_.pbrMetallicRoughness_.baseColorFactor_[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data_.pbrMetallicRoughness_.baseColorFactor_[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
        material.data_.pbrMetallicRoughness_.baseColorTexture_.index_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.data_.pbrMetallicRoughness_.baseColorTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
        material.data_.pbrMetallicRoughness_.metallicFactor_ = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.data_.pbrMetallicRoughness_.roughnessFactor_ = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
        material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.index_ = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        material.data_.normalTexture_.index_ = gltfMaterial.normalTexture.index;
        material.data_.normalTexture_.texcoord_ = gltfMaterial.normalTexture.texCoord;
        material.data_.normalTexture_.scale_ = static_cast<float>(gltfMaterial.normalTexture.scale);

        material.data_.occlusionTexture_.index_ = gltfMaterial.occlusionTexture.index;
        material.data_.occlusionTexture_.texcoord_ = gltfMaterial.occlusionTexture.texCoord;
        material.data_.occlusionTexture_.strength_ = static_cast<float>(gltfMaterial.occlusionTexture.strength);

        material.data_.emissiveTexture_.index_ = gltfMaterial.emissiveTexture.index;
        material.data_.emissiveTexture_.texcoord_ = gltfMaterial.emissiveTexture.texCoord;
    }

    std::vector<Material::Cbuffer> materialData;
    for (std::vector<Material>::const_reference material : materials_)
    {
        materialData.emplace_back(material.data_);
    }

    HRESULT result = S_OK;
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
    bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = materialData.data();

    result = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, materialBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());

    result = Graphics::Instance().GetDevice()->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

// ----- Texture情報抽出 -----
void GltfModel::FetchTexture(const tinygltf::Model& gltfModel)
{
    for (const tinygltf::Texture& gltfTexture : gltfModel.textures)
    {
        TextureData& texture = textures_.emplace_back();
        texture.name_ = gltfTexture.name;
        texture.source_ = gltfTexture.source;
    }

    for (const tinygltf::Image& gltfImage : gltfModel.images)
    {
        Image& image = images_.emplace_back();
        image.name_ = gltfImage.name;
        image.width_ = gltfImage.width;
        image.height_ = gltfImage.height;
        image.component_ = gltfImage.component;
        image.bits_ = gltfImage.bits;
        image.pixelType_ = gltfImage.pixel_type;
        image.bufferView_ = gltfImage.bufferView;
        image.mimeType_ = gltfImage.mimeType;
        image.uri_ = gltfImage.uri;
        image.asIs_ = gltfImage.as_is;

        if (gltfImage.bufferView > -1)
        {
            const tinygltf::BufferView& bufferView = gltfModel.bufferViews.at(gltfImage.bufferView);
            const tinygltf::Buffer& buffer = gltfModel.buffers.at(bufferView.buffer);
            const byte* data = buffer.data.data() + bufferView.byteOffset;

            const Texture::TextureData textureData = Texture::Instance().LoadTexture(data, bufferView.byteLength);
            textureResourceViews_.emplace_back().Attach(textureData.shaderResourceView_.Get());
        }
        else
        {
            const std::filesystem::path path(filename_);
            std::wstring filename = path.parent_path().concat(L"/").wstring() + std::wstring(gltfImage.uri.begin(), gltfImage.uri.end());
            image.filename_ = filename;

            const Texture::TextureData textureData = Texture::Instance().LoadTexture(filename.c_str());
            textureResourceViews_.emplace_back().Attach(textureData.shaderResourceView_.Get());
        }
    }
}

// ----- Animation情報抽出 -----
void GltfModel::FetchAnimation(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Skin>::const_reference transmissionSkin : gltfModel.skins)
    {
        Skin& skin = skins_.emplace_back();
        const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(transmissionSkin.inverseBindMatrices);
        const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);
        skin.inverseBindMatrices_.resize(gltfAccessor.count);
        std::memcpy(skin.inverseBindMatrices_.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4X4));
        skin.joints_ = transmissionSkin.joints;
    }

    for (std::vector<tinygltf::Animation>::const_reference gltfAnimation : gltfModel.animations)
    {
        Animation& animation = animations_.emplace_back();
        animation.name_ = gltfAnimation.name;

        for (std::vector<tinygltf::AnimationSampler>::const_reference gltfSampler : gltfAnimation.samplers)
        {
            Animation::Sampler& sampler = animation.samplers_.emplace_back();
            sampler.input_ = gltfSampler.input;
            sampler.output_ = gltfSampler.output;
            sampler.interpolation_ = gltfSampler.interpolation;

            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfSampler.input);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

            const std::pair<std::unordered_map<int, std::vector<float>>::iterator, bool>& timelines = animation.timelines_.emplace(gltfSampler.input, gltfAccessor.count);

            if (timelines.second)
            {
                std::memcpy(timelines.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(FLOAT));
            }
        }

        for (std::vector<tinygltf::AnimationChannel>::const_reference gltfChannel : gltfAnimation.channels)
        {
            Animation::Channel& channel = animation.channels_.emplace_back();
            channel.sampler_ = gltfChannel.sampler;
            channel.targetNode_ = gltfChannel.target_node;
            channel.targetPath_ = gltfChannel.target_path;

            const tinygltf::AnimationSampler& gltfSampler = gltfAnimation.samplers.at(gltfChannel.sampler);
            const tinygltf::Accessor& gltfAccessor = gltfModel.accessors.at(gltfSampler.output);
            const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

            if (gltfChannel.target_path == "scale")
            {
                const std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT3>>::iterator, bool>& scales = animation.scales_.emplace(gltfSampler.output, gltfAccessor.count);

                if (scales.second)
                {
                    std::memcpy(scales.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "rotation")
            {
                const std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT4>>::iterator, bool>& rotations = animation.rotations_.emplace(gltfSampler.output, gltfAccessor.count);

                if (rotations.second)
                {
                    std::memcpy(rotations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4));
                }
            }
            else if (gltfChannel.target_path == "translation")
            {
                const std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT3>>::iterator, bool>& translations = animation.translations_.emplace(gltfSampler.output, gltfAccessor.count);

                if (translations.second)
                {
                    std::memcpy(translations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
        }
    }

    for (decltype(animations_)::reference animation : animations_)
    {
        for (decltype(animation.timelines_)::reference timelines : animation.timelines_)
        {
            animation.duration_ = std::max<float>(animation.duration_, timelines.second.back());
        }
    }
}

// ----- 累積変換 -----
void GltfModel::CumulateTransforms(std::vector<Node>& nodes)
{
    std::stack<DirectX::XMFLOAT4X4> parentGlobalTransforms;
    std::function<void(int)> traverse{ [&](int nodeIndex)->void
    {
        Node& node = nodes.at(nodeIndex);
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale_.x, node.scale_.y, node.scale_.z);
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotation_.x, node.rotation_.y, node.rotation_.z, node.rotation_.w));
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z);
        DirectX::XMStoreFloat4x4(&node.globalTransform_, S * R * T * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));

        // RootMotionが有効でないときにはRootの移動値を無くす
        //if (node.isRootNode_ && useRootMotion_ == false)
        //{
        //    node.globalTransform_._41 = 0;
        //    node.globalTransform_._42 = 0;
        //    node.globalTransform_._43 = 0;
        //}

        for (int childIndex : node.children_)
        {
            parentGlobalTransforms.push(node.globalTransform_);
            traverse(childIndex);
            parentGlobalTransforms.pop();
        }
    } };
    for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
    {
        parentGlobalTransforms.push({ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 });
        traverse(nodeIndex);
        parentGlobalTransforms.pop();
    }
}

// ----- BufferView作成 -----
GltfModel::BufferView GltfModel::MakeBufferView(const tinygltf::Accessor& accessor)
{
    BufferView bufferView = {};
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format_ = DXGI_FORMAT_R16_UINT;
            bufferView.strideInBytes_ = sizeof(USHORT);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format_ = DXGI_FORMAT_R32_UINT;
            bufferView.strideInBytes_ = sizeof(UINT);
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format_ = DXGI_FORMAT_R32G32_FLOAT;
            bufferView.strideInBytes_ = sizeof(FLOAT) * 2;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format_ = DXGI_FORMAT_R32G32B32_FLOAT;
            bufferView.strideInBytes_ = sizeof(FLOAT) * 3;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            bufferView.format_ = DXGI_FORMAT_R8G8B8A8_UINT;
            bufferView.strideInBytes_ = sizeof(BYTE) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format_ = DXGI_FORMAT_R16G16B16A16_UINT;
            bufferView.strideInBytes_ = sizeof(USHORT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format_ = DXGI_FORMAT_R32G32B32A32_UINT;
            bufferView.strideInBytes_ = sizeof(UINT) * 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
            bufferView.strideInBytes_ = sizeof(FLOAT) * 4;
            break;
        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not suppoeted.");
            break;
        }
        break;
    default:
        _ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
        break;
    }
    bufferView.sizeInBytes_ = static_cast<UINT>(accessor.count * bufferView.strideInBytes_);
    return bufferView;
}
