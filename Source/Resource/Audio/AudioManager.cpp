#include "AudioManager.h"
#include "FrameWork/Misc.h"

AudioManager::AudioManager()
{
    HRESULT result = S_OK;

    result = XAudio2Create(xAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    result = xAudio2_->CreateMasteringVoice(&masterVoice_);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

void AudioManager::LoadAudio()
{
    LoadBGM();
}

void AudioManager::LoadBGM()
{
    bgm_[static_cast<int>(BGM::Game)] = std::make_unique<Audio>(xAudio2_.Get(), L"./Resources/Audio/BGM/Game.wav");
}

void AudioManager::PlayBGM(const BGM& bgm, const bool isLoop, const bool isIgnoreQueue)
{
    bgm_[static_cast<int>(bgm)]->Play(isLoop, isIgnoreQueue);
}

const int AudioManager::PlaySE(const SE& se)
{
    return se_[static_cast<int>(se)].Play();
}

void AudioManager::StopBGM(const BGM& bgm)
{
    bgm_[static_cast<int>(bgm)]->Stop();
}

void AudioManager::StopSE(const SE& se, const int& num)
{
    se_[static_cast<int>(se)].Stop(num);
}

void AudioManager::StopAllBGM()
{
    for (std::unique_ptr<Audio>& bgm : bgm_)
    {
        bgm->Stop();
    }
}

void AudioManager::StopAllSE()
{
    for (auto& se : se_)
    {
        se.AllStop();
    }
}

void AudioManager::StopAllAudio()
{
    for (std::unique_ptr<Audio>& bgm : bgm_)
    {
        if (bgm == nullptr) continue;

        bgm->Stop();
    }

    StopAllSE();
}

void AudioManager::SetBGMVolume(const BGM& bgm, const float& volume)
{
    bgm_[static_cast<int>(bgm)]->Volume(volume);
}

AudioManager::SEData::SEData(IXAudio2* xaudio2, const wchar_t* filename, const int& loadNum)
{
    // ロード数が無い
    if (loadNum <= 0) return;

    for (int i = 0; i < loadNum; ++i)
    {
        se_.emplace_back(std::make_unique<Audio>(xaudio2, filename));
    }
}

const int AudioManager::SEData::Play()
{
    // SEが無い
    if (se_.size() == 0) return -1;
    if (counter_ >= se_.size()) counter_ = 0;

    se_.at(counter_)->Play(false);

    return counter_++;
}

void AudioManager::SEData::Stop(const int& num)
{
    // 0以下の数字は受け付けない
    if (num < 0) return;
    // サイズオーバーも受け付けない
    if (num >= se_.size()) return;

    se_.at(num)->Stop();
}

void AudioManager::SEData::AllStop()
{
    if (se_.size() == 0) return;

    for (auto& se : se_)
    {
        se->Stop();
    }
}

void AudioManager::SEData::Volume(const float& volume)
{
    for (auto& se : se_)
    {
        se->Volume(volume);
    }
}
