#pragma once
#include "Math/Transform.h"
#include <string>

struct CutSceneCameraInfo
{
    DirectX::XMFLOAT3   rotation_           = {};
    DirectX::XMFLOAT3   target_             = {};
    DirectX::XMFLOAT3   offset_             = { 0.0f, 1.7f, 0.0f };
    float               length_             = 6.0f;
    float               lerpTime_           = 0.0f;
    std::string         targetName_         = "";
    std::string         targetJointName_    = "";
};