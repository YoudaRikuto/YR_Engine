#pragma once
#include <dxgi1_6.h>
#include <sstream>
#include "HighResolutionTimer.h"
#include "ImGui/ImGuiCtrl.h"
#include "Graphics/Graphics.h"
#include "Graphics/ConstantBuffer.h"
#include "Input/Input.h"
#include "Resource/GltfModel/AnimationEditer.h"
#include "Graphics/DeferredRendering/DeferredRendering.h"
#include "Resource/SkyMap/SkyMap.h"

class Framework
{
public:
    Framework(HWND hwnd);
    ~Framework() {}
    Framework(const Framework&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework(Framework&&) noexcept = delete;
    Framework& operator=(Framework&&) noexcept = delete;

    const int Run();
    LRESULT CALLBACK HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 viewProjection_;
        DirectX::XMFLOAT4   lightDirection_;
        DirectX::XMFLOAT4   cameraPosition_;
        DirectX::XMFLOAT4X4 inverseProjection_;
        DirectX::XMFLOAT4X4 inverseViewProjection_;
        DirectX::XMFLOAT4X4 inverseView_;
    };

private:
    const bool Initialize();
    const bool Finalize();
    void Update(const float& elapsedTime);
    void Render();
    void DrawDebug();
    void CalculateFrameStats();

private:
    CONST HWND  hwnd_;
    Graphics    graphics_;
    Input       input_;

    SkyMap skyMap_;

    HighResolutionTimer tictoc_;
    uint32_t            frames_ = 0;
    float               elapsedTime_ = 0.0f; // Œo‰ßŽžŠÔ

    ConstantBuffer<SceneConstants> sceneConstants_;

    DeferredRendering deferredRendering_;

    AnimationEditer animationEditer_;
};

