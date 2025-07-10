#pragma once
#include "MathHelperFloat2.h"
#include "MathHelperFloat3.h"
#include "MathHelperFloat4.h"

inline const float XMFloatRandomRange(
    const float& min,
    const float& max)
{
    float value = static_cast<float>(rand()) / RAND_MAX;

    return min + (max - min) * value;
}

inline const float XMFloatLerp(
    const float& start,
    const float& end,
    const float& timer)
{
    return start + timer * (end - start);
}

inline const DirectX::XMFLOAT4 XMFloat4RandomRange(
    const DirectX::XMFLOAT4& min,
    const DirectX::XMFLOAT4& max)
{
    DirectX::XMFLOAT4 result =
    {
        XMFloatRandomRange(min.x, max.x),
        XMFloatRandomRange(min.y, max.y),
        XMFloatRandomRange(min.z, max.z),
        XMFloatRandomRange(min.w, max.w),
    };

    return result;
}