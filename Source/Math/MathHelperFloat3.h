#pragma once
#include <DirectXMath.h>
#include <algorithm>

// =============================================
//                  éZèpââéZéq
// =============================================

#pragma region ---------- éZèpââéZéq ----------
// ----- float3 + float3 -----
inline const DirectX::XMFLOAT3 operator+(
    const DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    const DirectX::XMFLOAT3 result =
    {
        float3_1.x + float3_2.x,
        float3_1.y + float3_2.y,
        float3_1.z + float3_2.z,
    };

    return result;
}

// ----- float3 += float3 -----
inline const DirectX::XMFLOAT3 operator+=(
    DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    float3_1.x += float3_2.x;
    float3_1.y += float3_2.y;
    float3_1.z += float3_2.z;

    return float3_1;
}

// ----- float3 + float -----
inline const DirectX::XMFLOAT3 operator+(
    const DirectX::XMFLOAT3& float3,
    const float& f)
{
    const DirectX::XMFLOAT3 result =
    {
        float3.x + f,
        float3.y + f,
        float3.z + f,
    };

    return result;
}

// ----- float3 += float -----
inline const DirectX::XMFLOAT3 operator+=(
    DirectX::XMFLOAT3& float3,
    const float& f)
{
    float3.x += f;
    float3.y += f;
    float3.z += f;

    return float3;
}

// ----- float3 - float3 -----
inline const DirectX::XMFLOAT3 operator-(
    const DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    const DirectX::XMFLOAT3 result =
    {
        float3_1.x - float3_2.x,
        float3_1.y - float3_2.y,
        float3_1.z - float3_2.z,
    };

    return result;
}

// ----- float3 -= float3 -----
inline const DirectX::XMFLOAT3 operator-=(
    DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    float3_1.x -= float3_2.x;
    float3_1.y -= float3_2.y;
    float3_1.z -= float3_2.z;

    return float3_1;
}

// ----- float3 - float -----
inline const DirectX::XMFLOAT3 operator-(
    const DirectX::XMFLOAT3& float3,
    const float& f)
{
    const DirectX::XMFLOAT3 result =
    {
        float3.x - f,
        float3.y - f,
        float3.z - f,
    };

    return result;
}

// ----- float3 -= float -----
inline const DirectX::XMFLOAT3 operator-=(
    DirectX::XMFLOAT3& float3,
    const float& f)
{
    float3.x -= f;
    float3.y -= f;
    float3.z -= f;

    return float3;
}

// ----- float3 * float3 -----
inline const DirectX::XMFLOAT3 operator*(
    const DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    const DirectX::XMFLOAT3 result =
    {
        float3_1.x * float3_2.x,
        float3_1.y * float3_2.y,
        float3_1.z * float3_2.z,
    };

    return result;
}

// ----- float3 *= float3 -----
inline const DirectX::XMFLOAT3 operator*=(
    DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    float3_1.x *= float3_2.x;
    float3_1.y *= float3_2.y;
    float3_1.z *= float3_2.z;

    return float3_1;
}

// ----- float3 * float -----
inline const DirectX::XMFLOAT3 operator*(
    const DirectX::XMFLOAT3& float3,
    const float& f)
{
    const DirectX::XMFLOAT3 result =
    {
        float3.x * f,
        float3.y * f,
        float3.z * f,
    };

    return result;
}

// ----- float3 *= float -----
inline const DirectX::XMFLOAT3 operator*=(
    DirectX::XMFLOAT3& float3,
    const float& f)
{
    float3.x *= f;
    float3.y *= f;
    float3.z *= f;

    return float3;
}

// ----- float3 / float3 -----
inline const DirectX::XMFLOAT3 operator/(
    const DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    const DirectX::XMFLOAT3 result =
    {
        float3_1.x / float3_2.x,
        float3_1.y / float3_2.y,
        float3_1.z / float3_2.z,
    };

    return result;
}

// ----- float3 /= float3 -----
inline const DirectX::XMFLOAT3 operator/=(
    DirectX::XMFLOAT3& float3_1,
    const DirectX::XMFLOAT3& float3_2)
{
    float3_1.x /= float3_2.x;
    float3_1.y /= float3_2.y;
    float3_1.z /= float3_2.z;

    return float3_1;
}

// ----- float3 / float -----
inline const DirectX::XMFLOAT3 operator/(
    const DirectX::XMFLOAT3& float3,
    const float& f)
{
    const DirectX::XMFLOAT3 result =
    {
        float3.x / f,
        float3.y / f,
        float3.z / f,
    };

    return result;
}

// ----- float3 /= float -----
inline const DirectX::XMFLOAT3 operator/=(
    DirectX::XMFLOAT3& float3,
    const float& f)
{
    float3.x /= f;
    float3.y /= f;
    float3.z /= f;

    return float3;
}

#pragma endregion ---------- éZèpââéZéq ----------

// =============================================
//                  éZèpä÷êî
// =============================================

#pragma region ---------- éZèpä÷êî ----------
// ----- XMFLOAT3ìØémÇÃì‡êœÇåvéZ -----
inline const float XMFloat3Dot(
    const DirectX::XMFLOAT3& v1,
    const DirectX::XMFLOAT3& v2)
{
    return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
}

// ----- XMFLOAT3ìØémÇÃãóó£ÇåvéZ -----
inline const float XMFloat3Length(const DirectX::XMFLOAT3& v)
{
    return ::sqrtf(XMFloat3Dot(v, v));
}

// ----- XMFLOAT3ìØémÇÃãóó£ÇÃìÒèÊÇåvéZ -----
inline const float XMFloat3LengthSq(const DirectX::XMFLOAT3& v)
{
    return XMFloat3Dot(v, v);
}

// ----- XMFLOAT3ÇÃê≥ãKâª ( íPà ÉxÉNÉgÉãâª ) -----
inline const DirectX::XMFLOAT3 XMFloat3Normalize(const DirectX::XMFLOAT3& v)
{
    const float length = XMFloat3Length(v);

    if (length <= 0.0f) return DirectX::XMFLOAT3(0, 0, 0);

    return (v / length);
}

// ----- XMFLOAT3ìØémÇÃäOêœÇåvéZ ( èáî‘Ç…íçà” ) -----
inline const DirectX::XMFLOAT3 XMFloat3Cross(
    const DirectX::XMFLOAT3& v1,
    const DirectX::XMFLOAT3& v2)
{
    const DirectX::XMFLOAT3 cross =
    {
        (v1.y * v2.z) - (v1.z * v2.y),
        (v1.z * v2.x) - (v1.x * v2.z),
        (v1.x * v2.y) - (v1.y * v2.x),
    };

    return cross;
}

#pragma endregion ---------- éZèpä÷êî ----------