#pragma once
#include "CutSceneCameraInfo.h"
#include <vector>
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

    void PlayCutScene(const std::string& cutSceneName);

    const bool IsCutSceneCameraActive() const { return isCutSceneCameraActive_; }
    const CutSceneCameraInfo GetCutSceneCameraInfo() const { return cutSceneCameraInfo_; }

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
};

