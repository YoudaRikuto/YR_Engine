#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <mmreg.h>
#include <memory>
#include "FrameWork/Misc.h"

class AudioDevice
{
public:
    static bool Initialize()
    {
        HRESULT result = S_OK;

        result = XAudio2Create(&xaudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        result = xaudio2_->CreateMasteringVoice(&masterVoice_);
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

        return result == S_OK;
    }

    static void Finalize()
    {
        masterVoice_->DestroyVoice();
        xaudio2_->Release();
    }

    static IXAudio2* xaudio2_;
    static IXAudio2MasteringVoice* masterVoice_;
};

class AudioSourceVoice;
class AudioBuffer
{
public:
    AudioBuffer(const wchar_t* filename);
    virtual ~AudioBuffer();

    friend class AudioSourceVoice;

private:
    WAVEFORMATEXTENSIBLE    wfx_    = { 0 };
    XAUDIO2_BUFFER          buffer_ = { 0 };
};

class AudioSourceVoice
{
public:
    AudioSourceVoice(std::shared_ptr<AudioBuffer>& audioBuffer);
    virtual ~AudioSourceVoice();

    void Play(const int& loopCount = 0);
    void Stop(const bool& playTails = true);
    void Volume(const float& volume);
    void Pan(const float panValue);
    bool Queuing();

private:
    IXAudio2SourceVoice* sourceVoice_;
    std::shared_ptr<AudioBuffer> audioBuffer_;    
};