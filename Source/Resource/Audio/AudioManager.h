#pragma once
#include "Audio.h"
#include <wrl.h>

class AudioManager
{
private:
    AudioManager();
    ~AudioManager() = default;

public:
    static AudioManager& Instance()
    {
        static AudioManager instance;
        return instance;
    }

    void DrawDebug();

private:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;

};

