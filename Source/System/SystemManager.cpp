#include "SystemManager.h"
#include "ImGui/ImGuiCtrl.h"

void SystemManager::DrawDebug()
{
    ImGui::Begin("SystemManager");

    ImGui::DragFloat("GlobalTimeScale", &globalTimeScale_, 0.01f, 0.0f, 2.0f);

    ImGui::End();
}
