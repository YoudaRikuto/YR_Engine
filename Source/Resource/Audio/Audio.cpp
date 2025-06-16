#include "Audio.h"
#include <x3daudio.h>
#include <Windows.h>
#include <winerror.h>

IXAudio2* AudioDevice::xaudio2_ = NULL;
IXAudio2MasteringVoice* AudioDevice::masterVoice_ = NULL;

HRESULT FindChunk(const HANDLE& hfile, const DWORD& fourcc, DWORD& chunkSize, DWORD& chunkDataPosition)
{
    HRESULT result = S_OK;

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
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
        if (0 == ReadFile(hfile, &chunkType, sizeof(DWORD), &numberOfBytesRead, NULL))
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }

        if (0 == ReadFile(hfile, &chunkDataSize, sizeof(DWORD), &numberOfBytesRead, NULL))
        {
            result = HRESULT_FROM_WIN32(GetLastError());
        }

        switch (chunkType)
        {
        case 'FFIR':
            riffDataSize = chunkDataSize;
            chunkDataSize = 4;
            if (0 == ReadFile(hfile, &fileType, sizeof(DWORD), &numberOfBytesRead, NULL))
            {
                result = HRESULT_FROM_WIN32(GetLastError());
            }
            break;

        default:
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, chunkDataSize, NULL, FILE_CURRENT))
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

HRESULT ReadChunkData(const HANDLE& hFile, const LPVOID& buffer, const DWORD& bufferSize, const DWORD& bufferOffset)
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

AudioBuffer::AudioBuffer(const wchar_t* filename)
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
    _ASSERT_EXPR(fileType == 'EVAW', L"Only support 'WAVE'");

    //FindChunk()


}

AudioBuffer::~AudioBuffer()
{
}

AudioSourceVoice::AudioSourceVoice(std::shared_ptr<AudioBuffer>& audioBuffer)
{
}

AudioSourceVoice::~AudioSourceVoice()
{
}

void AudioSourceVoice::Play(const int& loopCount)
{
}

void AudioSourceVoice::Stop(const bool& playTails)
{
}

void AudioSourceVoice::Volume(const float& volume)
{
}

void AudioSourceVoice::Pan(const float panValue)
{
}

bool AudioSourceVoice::Queuing()
{
    return false;
}
