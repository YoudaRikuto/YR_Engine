#pragma once
#include "Math/Transform.h"
#include "CutSceneCamera.h"

class Camera
{
private:
    Camera() {}
    ~Camera() {}

public:
    static Camera& Instance()
    {
        static Camera instance;
        return instance;
    }

    void Update(const float& elapsedTime);  // 更新
    void DrawDebug();                       // ImGui

    void SetPerspectiveFov();

    // CutSceneCameraInfoによる [位置],[前ベクトル]を算出
    void CalcCameraVectorFromCutSceneCameraInfo(const CutSceneCameraInfo& cutSceneCameraInfo, DirectX::XMFLOAT3& position, DirectX::XMFLOAT3& forwardVec);

    // カメラから見たベクトルに変換する
    const DirectX::XMFLOAT2 ConvertTo2DVectorFromCamera(const DirectX::XMFLOAT2& v);

    // ---------- Camera Vector ----------
    const DirectX::XMFLOAT3 CalcForward();  // 前
    const DirectX::XMFLOAT3 CalcRight();    // 右


    [[nodiscard]] const DirectX::XMFLOAT4X4 GetView()       const { return view_; }
    [[nodiscard]] const DirectX::XMFLOAT4X4 GetProjection() const { return projection_; }
    [[nodiscard]] const DirectX::XMFLOAT3   GetEye()        const { return eye_; }

private:
    void Rotate(const float& elapsedTime);

private:
    // ---------- Camera Information ----------
    Transform3D         transform_  = {};
    DirectX::XMFLOAT4X4 view_       = {};
    DirectX::XMFLOAT4X4 projection_ = {};
    DirectX::XMFLOAT3   eye_        = {};
    DirectX::XMFLOAT3   focus_      = {};
    DirectX::XMFLOAT3   up_         = { 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT3   target_     = {};
    DirectX::XMFLOAT3   offset_     = { 0.0f, 1.7f, 0.0f };
    float               nearZ_      = 0.1f;
    float               farZ_       = 150.0f;
    float               fov_        = 45.0f;
    float               length_     = 6.0f;

    DirectX::XMFLOAT3   cutSceneCameraPosition = {};

    // ---------- Rotation ----------
    float verticalRotationSpeed_    = 1.7f; // 垂直回転速度
    float horizontalRotationSpeed_  = 4.0f; // 水平回転速度
    float minRotationX_             = DirectX::XMConvertToRadians(-15.0f);
    float maxRotationX_             = DirectX::XMConvertToRadians(35.0f);
};

