#pragma once
#define NOMINMAX
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include "tinygltf/tiny_gltf.h"
#include "Graphics/ConstantBuffer.h"
#include "Math/Serialize.h"
#include "Math/Transform.h"

class GltfModel
{
public:
    struct Scene
    {
        std::string         name_;
        std::vector<int>    nodes_;

        template<class T>
        void serialize(T& archive)
        {
            archive(name_, nodes_);
        }
    };

    struct Node
    {
        std::string         name_;
        int                 skin_ = -1;
        int                 mesh_ = -1;
        std::vector<int>    children_;
        DirectX::XMFLOAT4   rotation_ = { 0, 0, 0, 1 };
        DirectX::XMFLOAT3   scale_ = { 1, 1, 1 };
        DirectX::XMFLOAT3   translation_ = { 0, 0, 0 };
        DirectX::XMFLOAT4X4 globalTransform_ = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        bool                isRootNode_ = false;

        template<class T>
        void serialize(T& archive)
        {
            archive(name_, skin_, mesh_, children_, rotation_, scale_, translation_, globalTransform_, isRootNode_);
        }
    };

    struct BufferView
    {
        DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
        size_t strideInBytes_ = 0;
        size_t sizeInBytes_ = 0;
        std::vector<UINT8> verticesBinary_;

        const size_t Count() const { return sizeInBytes_ / strideInBytes_; }

        template<class T>
        void serialize(T& archive)
        {
            archive(format_, strideInBytes_, sizeInBytes_, verticesBinary_);
        }
    };

    struct Mesh
    {
        std::string name_;

        struct Primitive
        {
            int material_;
            std::map<std::string, BufferView> vertexBufferViews_;
            BufferView indexBufferView_;

            template<class T>
            void serialize(T& archive)
            {
                archive(material_, vertexBufferViews_, indexBufferView_);
            }
        };
        std::vector<Primitive> primitives_;

        template<class T>
        void serialize(T& archive)
        {
            archive(name_, primitives_);
        }
    };

    struct TextureInfo
    {
        int index_ = -1;
        int texcoord_ = 0;

        template<class T>
        void serialize(T& archive)
        {
            archive(index_, texcoord_);
        }
    };

    struct NormalTextureInfo
    {
        int     index_ = -1;
        int     texcoord_ = 0;
        float   scale_ = 1;

        template<class T>
        void serialize(T& archive)
        {
            archive(index_, texcoord_, scale_);
        }
    };

    struct OcclusionTextureInfo
    {
        int     index_ = -1;
        int     texcoord_ = 0;
        float   strength_ = 1;

        template<class T>
        void serialize(T& archive)
        {
            archive(index_, texcoord_, strength_);
        }
    };

    struct PbrMetaricRoughness
    {
        float       baseColorFactor_[4] = { 1, 1, 1, 1 };
        TextureInfo baseColorTexture_;
        float       metallicFactor_ = 1;
        float       roughnessFactor_ = 1;
        TextureInfo metallicRoughnessTexture_;

        template<class T>
        void serialize(T& archive)
        {
            archive(baseColorFactor_, baseColorTexture_, metallicFactor_, roughnessFactor_, metallicRoughnessTexture_);
        }
    };

    struct Material
    {
        std::string name_;

        struct Cbuffer
        {
            float   emissiveFactor_[3] = { 1, 1, 1 };
            int     alphaMode_ = 0; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
            float   alphaCutoff_ = 0.5f;
            bool    doubleSided_ = false;

            PbrMetaricRoughness pbrMetallicRoughness_;

            NormalTextureInfo       normalTexture_;
            OcclusionTextureInfo    occlusionTexture_;
            TextureInfo             emissiveTexture_;

            template<class T>
            void serialize(T& archive)
            {
                archive(emissiveFactor_, alphaMode_, alphaCutoff_, doubleSided_, pbrMetallicRoughness_,
                    normalTexture_, occlusionTexture_, emissiveTexture_);
            }
        };
        Cbuffer data_;

        template<class T>
        void serialize(T& archive)
        {
            archive(name_, data_);
        }
    };

    struct TextureData
    {
        std::string name_;
        int         source_ = -1;

        template<class T>
        void serialize(T& archive)
        {
            archive(name_, source_);
        }
    };

    struct Image
    {
        std::string uri_;
        std::wstring filename_;
        std::string name_;
        int         width_ = -1;
        int         height_ = -1;
        int         component_ = -1;
        int         bits_ = -1;
        int         pixelType_ = -1;
        int         bufferView_ = 0;
        std::string mimeType_;
        bool        asIs_ = false;


        template<class T>
        void serialize(T& archive)
        {
            archive(name_, width_, height_, component_, bits_, pixelType_, bufferView_,
                mimeType_, uri_, asIs_, filename_);
        }
    };

    struct Skin
    {
        std::vector<DirectX::XMFLOAT4X4> inverseBindMatrices_;
        std::vector<int> joints_;

        template<class T>
        void serialize(T& archive)
        {
            archive(inverseBindMatrices_, joints_);
        }
    };

    struct Animation
    {
        std::string name_;
        float       duration_ = 0.0f;

        struct Channel
        {
            int         sampler_ = -1;
            int         targetNode_ = -1;
            std::string targetPath_;

            template<class T>
            void serialize(T& archive)
            {
                archive(sampler_, targetNode_, targetPath_);
            }
        };
        std::vector<Channel> channels_;

        struct Sampler
        {
            int         input_ = -1;
            int         output_ = -1;
            std::string interpolation_;  // 補間

            template<class T>
            void serialize(T& archive)
            {
                archive(input_, output_, interpolation_);
            }
        };
        std::vector<Sampler> samplers_;

        std::unordered_map<int, std::vector<float>> timelines_;
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> scales_;
        std::unordered_map<int, std::vector<DirectX::XMFLOAT4>> rotations_;
        std::unordered_map<int, std::vector<DirectX::XMFLOAT3>> translations_;

        template<class T>
        void serialize(T& archive)
        {
            archive(name_, duration_, channels_, samplers_, timelines_, scales_, rotations_, translations_);
        }
    };

public:
    GltfModel(const std::string& filename, const std::string& rootNodeName = "root");
    virtual ~GltfModel() = default;

    void Render(const float& scaleFactor, ID3D11PixelShader* psShader = nullptr);
    void DrawDebug(); // ImGui
    
public:
    // ---------- Transform ----------
    Transform3D* GetTransform() { return &transform_; }

    // ---------- Animation ----------
    void PlayAnimation(const int& index, const bool& loop, const float& speed, const float& startFrame);
    void PlayAnimationBlend(const int& index, const bool& loop, const float& speed, const float& blendStartFrame, const float& transitionTime);
    void UpdateAnimation(const float& elapsedTime); // アニメーション更新
    void SetAnimationSpeed(const float& speed) { animationSpeed_ = speed; }
    void SetTransitionTime(const float& time) { transitionTime_ = time; }

    // ---------- RootMotion ----------
    void UpdateRootMotion(const float& scaleFacter);
    void UseRootMotion(const bool& flag);

    // ---------- JointPosition ----------
    const DirectX::XMFLOAT3 GetJointPosition(const size_t& nodeIndex, const float& scaleFactor, const DirectX::XMFLOAT3& offsetPosition = {});
    const DirectX::XMFLOAT3 GetJointPosition(const std::string& nodeName, const float& scaleFactor, const DirectX::XMFLOAT3& offsetPosition = {});


private:
    // ---------- Animation ----------
    const bool UpdateAnimationBlend(const float& elapsedTime);  // アニメーションブレンド
    void BlendAnimations(const std::vector<Node>* nodes[2], const float& factor, std::vector<Node>& node);
    void Animate(const int& animationIndex, const float& time, std::vector<Node>& animatedNodes);



private:
    // ---------- Model情報読み取り ----------
    void LoadGltfModelFromFilename();   // FilenameからModel情報読み込み
    void LoadGltfModelFromCereal();     // CerealデータからModel情報読み込み
    void FetchNodes(const tinygltf::Model& gltfModel);
    void FetchMeshes(const tinygltf::Model& gltfModel);
    void FetchMaterials(const tinygltf::Model& gltfModel);
    void FetchTexture(const tinygltf::Model& gltfModel);
    void FetchAnimation(const tinygltf::Model& gltfModel);
    void CumulateTransforms(std::vector<Node>& nodes);
    BufferView MakeBufferView(const tinygltf::Accessor& accessor);

private:
    std::vector<Scene>          scenes_;
    std::vector<Mesh>           meshes_;
    std::vector<Material>       materials_;
    std::vector<TextureData>    textures_;
    std::vector<Image>          images_;
    std::vector<Animation>      animations_;
    std::vector<Skin>           skins_;
    std::vector<Node>           nodes_;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    materialResourceView_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>          vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>           pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>           inputLayout_;



public:
    // ---------- Model情報 ----------
    const std::string           filename_;
    const std::string           rootNodeName_;
    int                         rootJointIndex_ = 0;



    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>   textureResourceViews_;
    // ---------- Shader ----------
    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world_;
        int                 material_ = -1;
        int                 hasTangent_ = 0;
        int                 skin_ = -1;
        int                 startInstanceLocation_ = 0;
    };
    static const size_t maxJoints_ = 512;
    struct JointConstants
    {
        DirectX::XMFLOAT4X4 matrices_[maxJoints_];
    };
    std::unique_ptr<ConstantBuffer<PrimitiveConstants>> primitiveConstants_;
    std::unique_ptr<ConstantBuffer<JointConstants>>     jointConstants_;

private:
    Transform3D transform_;

    // ---------- Animation ----------
    std::vector<Node>   animatedNodes_[2];
    int                 animationIndex_         = -1;
    float               animationSeconds_       = 0.0f;
    float               animationSpeed_         = 0.0f;
    float               animationBlendSeconds_  = 0.0f;
    float               transitionTime_         = 1.0f;
    bool                isAnimationLoop_        = false;
    bool                isAnimationEnd_         = false;
    bool                isAnimationBlend_       = false;
    // ---------- RootMotion ----------
    std::vector<Node>   zeroAnimatedNodes_;
    DirectX::XMFLOAT3   previousPosition_       = {};
    DirectX::XMFLOAT3   rootMotionValue_        = { 1.0f, 1.0f, 1.0f };
    bool                isRootMotionActive_     = false;

};