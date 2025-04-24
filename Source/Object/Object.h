#pragma once
#include "Resource/GltfModel.h"

class Object
{
public:
    Object(const std::string& filename, const float& scaleFactor);
    ~Object() {}

    void Update(const float& elapsedTime);              // çXêV
    void Render(ID3D11PixelShader* psShader = nullptr); // ï`âÊ
    void DrawDebug();                                   // ImGui

public:
    // ---------- Transform ----------
    Transform3D* GetTransform() { return gltfModel_.GetTransform(); }

    // ---------- Animation ----------
    void PlayAnimation(const int& index, const bool& loop = false, const float& speed = 1.0f, const float& startFrame = 0.0f) { gltfModel_.PlayAnimation(index, loop, speed, startFrame); }
    void PlayAnimationBlend(const int& index, const bool& loop = false, const float& speed = 1.0f, const float& blendStartFrame = 0.0f, const float& transitionTime = 1.0f) { gltfModel_.PlayAnimationBlend(index, loop, speed, blendStartFrame, transitionTime); }

    void SetAnimationSpeed(const float& speed) { gltfModel_.SetAnimationSpeed(speed); }
    void SetTransitionTime(const float& transitionTime) { gltfModel_.SetTransitionTime(transitionTime); }

    // ---------- JointPosition ----------
    const DirectX::XMFLOAT3 GetJointPosition(const size_t& nodeIndex, const DirectX::XMFLOAT3& offsetPosition = {}) { return gltfModel_.GetJointPosition(nodeIndex, scaleFactor_, offsetPosition); }
    const DirectX::XMFLOAT3 GetJointPosition(const std::string& nodeName, const DirectX::XMFLOAT3& offsetPosition = {}) { return gltfModel_.GetJointPosition(nodeName, scaleFactor_, offsetPosition); }

private:
    GltfModel   gltfModel_;
    const float scaleFactor_;
};