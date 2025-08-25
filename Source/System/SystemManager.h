#pragma once
class SystemManager
{
private:
    SystemManager() {}
    ~SystemManager() {}

public:
    static SystemManager& Instance()
    {
        static SystemManager instance;
        return instance;
    }

    void DrawDebug();

    const float GetGlobalTimeScale() const { return globalTimeScale_; }
    void SetGlobalTimeScale(const float timeScale) { globalTimeScale_ = timeScale; }

private:
    float globalTimeScale_ = 1.0f;

};

