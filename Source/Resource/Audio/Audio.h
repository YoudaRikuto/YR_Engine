#pragma once
#include <xaudio2.h>
#include <mmreg.h>

class Audio
{
public:
    Audio(IXAudio2* xaudio2, const wchar_t* filename);
    virtual ~Audio();

    void Play(const int& loopCount = 0);
    void Play(const bool& isLoop = false, const bool& isIgnoreQueue = false);
    void Stop(const bool& playTails = true, const size_t afterSamplesPlayed = 0);
    void Volume(const float& volume);
    const bool Queuing();

private:
    WAVEFORMATEXTENSIBLE    wfx_            = { 0 };
    XAUDIO2_BUFFER          buffer_         = { 0 };
    IXAudio2SourceVoice*    sourceVoice_    = nullptr;
};