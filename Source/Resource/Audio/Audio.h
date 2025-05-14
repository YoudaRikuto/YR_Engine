#pragma once
#include <xaudio2.h>
#include <mmreg.h>

class Audio
{
public:
    Audio(IXAudio2* xaudio2, const wchar_t* filename);
    ~Audio();

    void Play(const int& loopCount);
    void Play(const bool& loop = false, const bool& isIgnoreQueue = false);
    void Stop(const bool& playTails = true, const size_t& afterSamplesPlayed = 0);
    void Volume(const float& volume);
    const bool Queuing();

private:
    WAVEFORMATEXTENSIBLE    wfx_            = {};
    XAUDIO2_BUFFER          buffer_         = {};
    IXAudio2SourceVoice*    sourceVoice_    = nullptr;
};

