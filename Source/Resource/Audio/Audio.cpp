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
        case 'FFIR'/*RIFF*/:
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

    DWORD numberOfBytesRead;

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    if (0 == ReadFile(hFile, buffer, bufferSize, &numberOfBytesRead, NULL))
    {
        result = HRESULT_FROM_WIN32(GetLastError());
    }

    return result;
}

Audio::Audio(IXAudio2* xaudio2, const wchar_t* filename)
{
    HRESULT result;

    // Open the file
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
    // check the file type, should be 'WAVE' or 'XWMA'
    FindChunk(hFile, 'FFIR'/*RIFF*/, chunkSize, chunkPosition);
    DWORD fileType;
    ReadChunkData(hFile, &fileType, sizeof(DWORD), chunkPosition);
    _ASSERT_EXPR(fileType == 'EVAW'/*WAVE*/, L"Onlt support 'WAVE'");

    FindChunk(hFile, ' tmf'/*FMT*/, chunkSize, chunkPosition);
    ReadChunkData(hFile, &wfx_, chunkSize, chunkPosition);
       
    FindChunk(hFile, 'atad'/*DATA*/, chunkSize, chunkPosition);
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
