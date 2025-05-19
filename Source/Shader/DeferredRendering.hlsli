struct PSIn
{
    float4 position_ : SV_POSITION;
    float2 texcoord_ : TEXCOORD;
};

cbuffer SceneConstantBuffer : register(b0)
{
    row_major float4x4 viewProjection_;
    float4 lightDirection_;
    float4 cameraPosition_;
    row_major float4x4 lightViewProjection_;
    row_major float4x4 inverseViewProjection_;
}

#include "GBuffer.hlsli"