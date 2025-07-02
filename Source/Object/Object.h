#pragma once
#include "Resource/GltfModel/GltfModel.h"
#include "Collision/CollisionData.h"
#include "Resource/DebugRenderer/DebugRenderer.h"
#include <vector>

class Object
{
public:
    Object(const std::string& filename, const float& scaleFactor, const std::string& objectName);
    ~Object() {}

    void Update(const float& elapsedTime);              // çXêV
    void Render(ID3D11PixelShader* psShader = nullptr);
    void Render(const DirectX::XMFLOAT4X4& world, ID3D11PixelShader* psShader = nullptr);
    void DrawDebug();                                   // ImGui
    void DebugRender(DebugRenderer* debugRenderer);

public:
    // ---------- Transform ----------
    Transform3D* GetTransform() { return gltfModel_.GetTransform(); }
    const float GetScaleFactor() const { return scaleFactor_; }

    // ---------- Animation ----------
    void PlayAnimation(const int& index, const bool& loop = false, const float& speed = 1.0f, const float& startFrame = 0.0f) { gltfModel_.PlayAnimation(index, loop, speed, startFrame); }
    void PlayAnimationBlend(const int& index, const bool& loop = false, const float& speed = 1.0f, const float& blendStartFrame = 0.0f, const float& transitionTime = 1.0f) { gltfModel_.PlayAnimationBlend(index, loop, speed, blendStartFrame, transitionTime); }

    void SetAnimationSpeed(const float& speed) { gltfModel_.SetAnimationSpeed(speed); }
    void SetTransitionTime(const float& transitionTime) { gltfModel_.SetTransitionTime(transitionTime); }

    const int GetAnimationIndex() const { return gltfModel_.GetAnimationIndex(); }
    const float GetAnimationSeconds() const { return gltfModel_.GetAnimationSeconds(); }
    const bool IsAnimationEnd()     const { return gltfModel_.IsAnimationEnd(); }
    const bool IsAnimationBlend()   const { return gltfModel_.IsAnimationBlend(); }

    // ---------- RootMotion ----------
    void UpdateRootMotion(const float& scaleFacter) { gltfModel_.UpdateRootMotion(scaleFacter); }
    void UseRootMotion(const bool& flag) { gltfModel_.UseRootMotion(flag); }
    const bool IsRootMotionActive() const { return gltfModel_.IsRootMotionActive(); }
    void SetRootMotionValue(const DirectX::XMFLOAT3& value) { gltfModel_.SetRootMotionValue(value); }

    // ---------- JointPosition ----------
    const DirectX::XMFLOAT3 GetJointPosition(const size_t& nodeIndex, const DirectX::XMFLOAT3& offsetPosition = {}) { return gltfModel_.GetJointPosition(nodeIndex, scaleFactor_, offsetPosition); }
    const DirectX::XMFLOAT3 GetJointPosition(const std::string& nodeName, const DirectX::XMFLOAT3& offsetPosition = {}) { return gltfModel_.GetJointPosition(nodeName, scaleFactor_, offsetPosition); }

    const int GetNodeIndex(const std::string& nodeName) { return gltfModel_.GetNodeIndex(nodeName); }
    std::vector<GltfModel::Node>* GetNodes() { return gltfModel_.GetNodes(); }

    // ---------- Collision ----------
    void UpdateCollisions(const float& elapsedTime);
    void UpdatePushColliders();
    const std::vector<PushCollider> GetPushColliders() const { return pushColliders_; }
    const std::vector<HitBox> GetHitBoxes() const { return hitBoxes_; }
    const std::vector<HurtBox> GetHurtBoxes() const { return hurtBoxes_; }

private:
    GltfModel   gltfModel_;
    const float scaleFactor_;
    const std::string objectName_;

    // ---------- Collision ----------
    std::vector<PushCollider>   pushColliders_;
    std::vector<HitBox>         hitBoxes_;
    std::vector<HurtBox>        hurtBoxes_;
    PushCollider                pushCollider_;
    HitBox                      hitBox_;
    HurtBox                     hurtBox_;
    bool                        isDebugDrawPushColliders_   = true;
    bool                        isDebugDrawHitBoxes_        = true;
    bool                        isDebugDrawHurtBoxes_       = true;
    bool                        isDebugDrawPushCollider_    = false;
    bool                        isDebugDrawHitBox_          = false;
    bool                        isDebugDrawHurtBox_         = false;
    bool                        isPushColliderListActive_   = false;
    bool                        isHitBoxListActive_         = false;
    bool                        isHurtBoxListActive_        = false;
};