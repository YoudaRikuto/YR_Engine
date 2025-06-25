#include "Framework.h"
#include "Math/MathHelper.h"
#include "Scene/SceneManager.h"
#include "Graphics/Camera/Camera.h"
#include "Resource/EffectManager.h"
#include "Graphics/PostProcess/PostProcess.h"
#include "Resource/Audio/AudioManager.h"

Framework::Framework(HWND hwnd)
    : hwnd_(hwnd), graphics_(hwnd), input_(hwnd),
    sceneConstants_()
{
}

// 初期化 
const bool Framework::Initialize()
{
    // Input 初期設定
    input_.GetMouse().SetScreenWidth(SCREEN_WIDTH);
    input_.GetMouse().SetScreenHeight(SCREEN_HEIGHT);

    // Audio 読み込み
    AudioManager::Instance().LoadAudio();
    AudioManager::Instance().StopAllAudio();

    SceneManager::Instance().Initialize();

    EffectManager::Instance().Initialize();

    sceneConstants_.GetData()->lightDirection_ = { 0.0f, -1.0f, 0.0f, 0.0f };

    return true;
}

// 終了化 
const bool Framework::Finalize()
{
    SceneManager::Instance().Finalize();

    EffectManager::Instance().Finalize();

    return false;
}

// 更新 
void Framework::Update(const float& elapsedTime)
{
    // ImGui更新
    IMGUI_CTRL_CLEAR_FRAME();

    // 入力更新
    input_.Update();

    // Scene更新
    SceneManager::Instance().Update(elapsedTime);

    // カメラ更新
    Camera::Instance().Update(elapsedTime);

    EffectManager::Instance().Update(elapsedTime);

    animationEditer_.Update(elapsedTime);

    // ImGui更新
    DrawDebug();
}

#define USE_GBUFFER 1

// 描画 
void Framework::Render()
{
    // 描画初期化
    Graphics::Instance().RenderInitialize();

    Camera::Instance().SetPerspectiveFov();

    const DirectX::XMFLOAT4X4 view = Camera::Instance().GetView();
    const DirectX::XMFLOAT4X4 projection = Camera::Instance().GetProjection();
    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->viewProjection_, DirectX::XMLoadFloat4x4(&view) * DirectX::XMLoadFloat4x4(&projection));

    const DirectX::XMFLOAT3 cameraPosition = Camera::Instance().GetEye();
    sceneConstants_.GetData()->cameraPosition_ = { cameraPosition.x, cameraPosition.y, cameraPosition.z, 0 };

    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->inverseProjection_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&projection)));
    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->inverseViewProjection_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view) * DirectX::XMLoadFloat4x4(&projection)));
    DirectX::XMStoreFloat4x4(&sceneConstants_.GetData()->inverseView_, DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view)));

    sceneConstants_.Activate(0, true, true, true, true);

#if USE_GBUFFER
    // G-Buffer設定
    Graphics::Instance().SetGBuffer();
    Graphics::Instance().SetBlendState(Shader::BlendState::MRT);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::Solid);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_ON_ZW_ON);
    
    // Scene描画
    SceneManager::Instance().DeferredRender();
#endif

    PostProcess::Instance().Activate();
        
    skyMap_.Draw();

    // Scene描画
    //SceneManager::Instance().Render();

#if USE_GBUFFER
    deferredRendering_.Draw();
#endif

    SceneManager::Instance().Render();
    EffectManager::Instance().Render();

    PostProcess::Instance().Deactivate();

    ID3D11RenderTargetView* renderTargetView = graphics_.GetRenderTargetView();
    ID3D11DepthStencilView* depthStencilView = graphics_.GetDepthStencilView();
    FLOAT color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    Graphics::Instance().GetDeviceContext()->ClearRenderTargetView(renderTargetView, color);
    Graphics::Instance().GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    PostProcess::Instance().Draw();

    Graphics::Instance().SetBlendState(Shader::BlendState::Alpha);
    Graphics::Instance().SetRasterizerState(Shader::RasterState::CullNone);
    Graphics::Instance().SetDepthStencileState(Shader::DepthState::ZT_OFF_ZW_OFF);

    // アニメーションエディタ
    animationEditer_.Render();
    animationEditer_.DrawDebug();

    // ImGui描画
    IMGUI_CTRL_DISPLAY();

    // 描画実行
    Graphics::Instance().Draw();
}

// ImGui用 
void Framework::DrawDebug()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    ImGuiID dockSpaceID = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    SceneManager::Instance().DrawDebug();

    PostProcess::Instance().DrawDebug();

    ImGui::Begin("SkyMap");
    skyMap_.DrawDebug();
    ImGui::End();
}

// 実行 
const int Framework::Run()
{
    MSG msg = {};

    if (Initialize() == false) return 0;

    // ImGui初期化(DirectX11の初期化の下に置くこと)
    IMGUI_CTRL_INITIALIZE(hwnd_, graphics_.GetDevice(), graphics_.GetDeviceContext());

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            tictoc_.Tick();
            CalculateFrameStats();
            Update(tictoc_.TimeInterval());
            Render();
        }
    }

    // ImGui終了化
    IMGUI_CTRL_UNINITIALIZE();

    BOOL fullscreen = 0;
    graphics_.GetSwapChain()->GetFullscreenState(&fullscreen, 0);
    if (fullscreen)
    {
        graphics_.GetSwapChain()->SetFullscreenState(FALSE, 0);
    }

    return Finalize() ? static_cast<int>(msg.wParam) : 0;
}

// メッセージハンドラ 
LRESULT Framework::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // ImGui
    IMGUI_CTRL_WND_PRC_HANDLER(hwnd, msg, wparam, lparam);

    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        BeginPaint(hwnd, &ps);

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CREATE:
        break;
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_ENTERSIZEMOVE:
        tictoc_.Stop();
        break;
    case WM_EXITSIZEMOVE:
        tictoc_.Start();
        break;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}

// フレーム計算 
void Framework::CalculateFrameStats()
{
    if (++frames_, (tictoc_.TimeStamp() - elapsedTime_) >= 1.0f)
    {
        float fps = static_cast<float>(frames_);
        std::wostringstream outs;
        outs.precision(6);

        // ゲームタイトル
        outs << APPLICATION_NAME;

        // FPS
        outs << L" FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";

        SetWindowTextW(hwnd_, outs.str().c_str());

        frames_ = 0;
        elapsedTime_ += 1.0f;
    }
}