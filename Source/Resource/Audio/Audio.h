#pragma once
#include <xaudio2.h>
#include <mmreg.h>

class Audio
{
public:
    Audio(IXAudio2* xaudio2, const wchar_t* filename);
    ~Audio();



private:
    WAVEFORMATEXTENSIBLE    wfx_            = {};
    XAUDIO2_BUFFER          buffer_         = {};
    IXAudio2SourceVoice*    sourceVoice_    = nullptr;
};

