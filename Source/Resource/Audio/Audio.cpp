#include "Audio.h"
#include <Windows.h>
#include <winerror.h>
#include "FrameWork/Misc.h"

const HRESULT FindChunk(const HANDLE& hFile, const DWORD& fourcc, DWORD& chunkSize, DWORD& chunkDataPosition)
{
    HRESULT result = S_OK;

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD chunkType;
    DWORD chunkDataSize;
    DWORD riffDataSize = 0;
    DWORD fileType;
    DWORD bytesRead = 0;
    DWORD offset = 0;

    while (result == S_OK)
    {
        DWORD numberOfBytesRead;
        if (0 == ReadFile(hFile, &chunkType, sizeof(DWORD), &numberOfBytesRead, NULL))
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }

        if (0 == ReadFile(hFile, &chunkDataSize, sizeof(DWORD), &numberOfBytesRead, NULL))
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }

        switch (chunkType)
        {
        case 'FFIR':
            riffDataSize = chunkDataSize;
            chunkDataSize = 4;
            if (0 == ReadFile(hFile, &fileType, sizeof(DWORD), &numberOfBytesRead, NULL))
            {
                result = HRESULT_FROM_WIN32(GetLastError());
            }
            break;

        default:
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, chunkDataSize, NULL, FILE_CURRENT))
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }

        offset += sizeof(DWORD) * 2;

        if (chunkType == fourcc)
        {
            chunkSize = chunkDataSize;
            chunkDataPosition = offset;
            return S_OK;
        }

        offset += chunkDataSize;

        if (bytesRead >= riffDataSize)
        {
            return S_FALSE;
        }
    }

    return S_OK;
}

const HRESULT ReadChunkData(const HANDLE& hFile, const LPVOID& buffer, const DWORD& bufferSize, const DWORD& bufferOffset)
{
    HRESULT result = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    DWORD numberOfBytesRead;
    if (0 == ReadFile(hFile, buffer, bufferSize, &numberOfBytesRead, NULL))
    {
        result = HRESULT_FROM_WIN32(GetLastError());
    }
    return result;
}

Audio::Audio(IXAudio2* xaudio2, const wchar_t* filename)
{
    HRESULT result;

    HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        result = HRESULT_FROM_WIN32(GetLastError());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
    {
        result = HRESULT_FROM_WIN32(GetLastError());
        _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
    }

    DWORD chunkSize;
    DWORD chunkPosition;

    FindChunk(hFile, 'FFIR', chunkSize, chunkPosition);
    DWORD fileType;
    ReadChunkData(hFile, &fileType, sizeof(DWORD), chunkPosition);
    _ASSERT_EXPR(fileType == 'EVAW', L"Onlt support 'WAVE'");

    FindChunk(hFile, ' tmf', chunkSize, chunkPosition);
    ReadChunkData(hFile, &wfx_, chunkSize, chunkPosition);

    FindChunk(hFile, 'atad', chunkSize, chunkPosition);
    BYTE* data = new BYTE[chunkSize];
    ReadChunkData(hFile, data, chunkSize, chunkPosition);

    buffer_.AudioBytes = chunkSize;  
    buffer_.pAudioData = data;   
    buffer_.Flags = XAUDIO2_END_OF_STREAM;

    result = xaudio2->CreateSourceVoice(&sourceVoice_, (WAVEFORMATEX*)&wfx_);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

Audio::~Audio()
{
    sourceVoice_->DestroyVoice();
    delete[] buffer_.pAudioData;
}

void Audio::Play(const int& loopCount)
{
    HRESULT result;

    XAUDIO2_VOICE_STATE voiceState = {};
    sourceVoice_->GetState(&voiceState);

    if (voiceState.BuffersQueued)
    {
        return;
    }

    buffer_.LoopCount = loopCount;
    result = sourceVoice_->SubmitSourceBuffer(&buffer_);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    result = sourceVoice_->Start(0);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

void Audio::Play(const bool& isLoop, const bool& isIgnoreQueue)
{
    HRESULT result;

    XAUDIO2_VOICE_STATE voiceState = {};
    sourceVoice_->GetState(&voiceState);

    if (!isIgnoreQueue && voiceState.BuffersQueued) return;

    const int loopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;

    buffer_.LoopCount = loopCount;
    result = sourceVoice_->SubmitSourceBuffer(&buffer_);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    result = sourceVoice_->Start(0);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

void Audio::Stop(const bool& playTails, const size_t afterSamplesPlayed)
{
    XAUDIO2_VOICE_STATE voiceState = {};
    sourceVoice_->GetState(&voiceState);
    if (!voiceState.BuffersQueued)
    {
        return;
    }

    if (voiceState.SamplesPlayed < afterSamplesPlayed)
    {
        return;
    }

    HRESULT result;
    result = sourceVoice_->Stop(playTails ? XAUDIO2_PLAY_TAILS : 0);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));

    result = sourceVoice_->FlushSourceBuffers();
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

void Audio::Volume(const float& volume)
{
    HRESULT result = sourceVoice_->SetVolume(volume);
    _ASSERT_EXPR(SUCCEEDED(result), HRTrace(result));
}

const bool Audio::Queuing()
{
    XAUDIO2_VOICE_STATE voiceState = {};
    sourceVoice_->GetState(&voiceState);
    return voiceState.BuffersQueued;
}
