#ifndef __SceneConstants__
#define __SceneConstants__

struct SceneConstants
{
    row_major float4x4  viewProjection_;
    float4              lightDirection_;
    float4              eyePosition;
    row_major float4x4  inverseProjection_;
    row_major float4x4  inverseViewProjection_;
    row_major float4x4  inverseView_;
};
cbuffer sceneConstants : register(b0)
{
    SceneConstants sceneData_;
}

#endif // __SceneConstants__