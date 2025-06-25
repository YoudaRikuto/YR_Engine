#pragma once
#include "Audio.h"
#include <wrl.h>
#include <memory>
#include <xaudio2.h>
#include <vector>

enum class BGM
{
    //Title,
    Game,

    Max,
};

enum class SE
{
    Hit,

    Max,
};

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

    void LoadAudio();

    void PlayBGM(const BGM& bgm, const bool isLoop = true, const bool isIgnoreQueue = false);

    const int PlaySE(const SE& se); // SEçƒê∂

    void StopBGM(const BGM& bgm);                   // BGMí‚é~
    void StopSE(const SE& se, const int& num);      // SEí‚é~

    void StopAllBGM();                              // ëSBGMí‚é~
    void StopAllSE();                               // ëSSEí‚é~
    void StopAllAudio();                            // ëSâπäyí‚é~

    void SetBGMVolume(const BGM& bgm, const float& volume);

private:
    void LoadBGM();

private:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;

    std::unique_ptr<Audio> bgm_[static_cast<int>(BGM::Max)];

    struct SEData
    {
    public:
        SEData(IXAudio2* xaudio2, const wchar_t* filename, const int& loadNum = 1);
        [[nodiscard]] const int Play();
        void Stop(const int& num);
        void AllStop();
        void Volume(const float& volume);

    private:
        std::vector<std::unique_ptr<Audio>> se_;
        int     counter_ = 0;
        float   volume_ = 0.0f;
    };
    std::vector<SEData> se_;
};