#pragma once
#include "CutSceneCameraInfo.h"
#include "Resource/DebugRenderer/DebugRenderer.h"
#include <unordered_map>

class CutSceneCamera
{
private:
    CutSceneCamera() {}
    ~CutSceneCamera() {}

public:
    static CutSceneCamera& Instance()
    {
        static CutSceneCamera instance;
        return instance;
    }

    void Update(const float& elapsedTime);
    void DrawDebug();
    void DebugRender(DebugRenderer* debugRenderer);

    void PlayCutScene(const std::string& cutSceneName);

    const bool IsCutSceneCameraActive() const { return isCutSceneCameraActive_; }
    const CutSceneCameraInfo GetCutSceneCameraInfo() const { return cutSceneCameraInfo_; }

private:
    void AssetCreation(const std::string& filename);

private:
    std::unordered_map<std::string, std::vector<CutSceneCameraInfo>> cutSceneCameras_;
    std::vector<CutSceneCameraInfo> cutSceneCamera_;
    
    CutSceneCameraInfo cutSceneCameraInfo_;
    CutSceneCameraInfo currentCutSceneCameraInfo_;
    CutSceneCameraInfo oldCutSceneCameraInfo_;


    CutSceneCameraInfo debugCutSceneCameraInfo_;


    int currentDataIndex_ = 0;
    float lerpTimer_ = 0.0f;


    bool isCutSceneCameraActive_ = false;

    // ----- DebugRender -----
    float boxScale_             = 0.1f;
    float forwardSphereLength_  = 0.15f;
    float sphereRadius_         = 0.05f;

    const std::string fileDirectory_ = "./Resources/JsonParameters/CutSceneCamera/";
};

