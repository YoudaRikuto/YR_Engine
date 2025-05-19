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

// ImGui—p
void AudioManager::DrawDebug()
{
}
