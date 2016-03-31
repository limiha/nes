
#include "pch.h"
#include "XA2AudioProvider.h"

#define SAMPLE_BUFFER_SIZE 1024

using namespace Platform;

void XA2ThreadFunc(void *params)
{
    ((XA2AudioProvider*)params)->AudioThreadCallback();
}

class XA2VoiceCallback : public IXAudio2VoiceCallback
{
public:
    XA2VoiceCallback(HANDLE hFillNext)
        : _hFillNext(hFillNext)
    {
    }

    // IXAudio2VoiceCallback implementation
public:
    STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext)
    {
        // Tell the audio thread to fill the next buffer
        SetEvent(_hFillNext);
    }

    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) { }
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) { }
    STDMETHOD_(void, OnStreamEnd) (THIS) { }
    STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) { }
    STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) { }
    STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) { }

private:
    HANDLE _hFillNext;
};

XA2AudioProvider::XA2AudioProvider(int sampleRate)
    : _sampleRate(sampleRate)
    , _nextFillBuffer(0)
    , _initialized(false)
    , _paused(false)
    , _firstUnpause(true)
    , _callbackFunc(nullptr)
    , _callbackData(nullptr)
    , _hFillNext(nullptr)
    , _hShutdownRequest(nullptr)
    , _hShutdownComplete(nullptr)
{
}

XA2AudioProvider::~XA2AudioProvider()
{
    if (_initialized)
    {
        // Stop playback
        PauseAudio();

        // Stop the audio callback thread
        SetEvent(_hShutdownRequest);
        WaitForSingleObject(_hShutdownComplete, INFINITE);

        if (_hFillNext != nullptr)
            CloseHandle(_hFillNext);

        if (_hShutdownRequest != nullptr)
            CloseHandle(_hShutdownRequest);

        if (_hShutdownComplete != nullptr)
            CloseHandle(_hShutdownComplete);
    }
}

void XA2AudioProvider::Initialize(AudioCallback* callback, void* callbackData)
{
    HRESULT hr = InitializeInternal(callback, callbackData);
    if (hr != S_OK)
        throw Exception::CreateException(hr);
}

void XA2AudioProvider::PauseAudio()
{
    if (!_paused)
    {
        _sourceVoice->Stop(0);
        _paused = true;
    }
}

void XA2AudioProvider::UnpauseAudio()
{
    if (_initialized && _paused)
    {
        if (_firstUnpause)
        {
            // The first time we unpause, we'll need to signal the callback thread so it knows to
            // begin filling the first buffer.  The callback thread will start the source voice
            // once the first buffer has been filled.
            _firstUnpause = false;
            SetEvent(_hFillNext);
        }
        else
        {
            _sourceVoice->Start(0);
        }

        _paused = false;
    }
}

int XA2AudioProvider::GetSampleRate()
{
    return _sampleRate;
}

int XA2AudioProvider::GetBitsPerSample()
{
    return 8; // TODO: Multi-bitrate support
}

int XA2AudioProvider::GetSilenceValue()
{
    return 127; // TODO: Multi-bitrate support
}

HRESULT XA2AudioProvider::InitializeInternal(AudioCallback* callback, void* callbackData)
{
    HRESULT hr = S_OK;

    _callbackFunc = callback;
    _callbackData = callbackData;

    // Initialize event handles
    _hFillNext = CreateEvent(NULL, FALSE, FALSE, NULL);
    IfNullRet(_hFillNext);

    _hShutdownRequest = CreateEvent(NULL, FALSE, FALSE, NULL);
    IfNullRet(_hShutdownRequest);

    _hShutdownComplete = CreateEvent(NULL, FALSE, FALSE, NULL);
    IfNullRet(_hShutdownComplete);

    // Initialize xaudio2
    hr = XAudio2Create(&_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    IfFailRet(hr);

    hr = _xaudio2->CreateMasteringVoice(&_masterVoice.m_p);
    IfFailRet(hr);

    int bitsPerSample = GetBitsPerSample();
    int bytesPerSample = bitsPerSample / 8;

    WAVEFORMATEX format = { 0 };
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.wBitsPerSample = bitsPerSample;
    format.nSamplesPerSec = _sampleRate;
    format.nBlockAlign = bytesPerSample;
    format.nAvgBytesPerSec = bytesPerSample * _sampleRate;

    _voiceCallback.Attach(new XA2VoiceCallback(_hFillNext));
    hr = _xaudio2->CreateSourceVoice(&_sourceVoice.m_p, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, _voiceCallback);
    IfFailRet(hr);

    // Initialize our buffers
    int bufferSize = SAMPLE_BUFFER_SIZE * bytesPerSample;
    _bufferMemory[0].Allocate(bufferSize);
    _bufferMemory[1].Allocate(bufferSize);
    memset(&_buffers, 0, sizeof(_buffers));
    for (int i = 0; i < 2; i++)
    {
        memset(_bufferMemory[i], 0, bufferSize);
        _buffers[i].AudioBytes = bufferSize;
        _buffers[i].pAudioData = _bufferMemory[i];
    }

    // We can now start the audio callback thread
    uintptr_t hThread = _beginthread(XA2ThreadFunc, 0, this);
    IfNullRet(hThread);

    _paused = true;
    _initialized = true;

    return S_OK;
}

void XA2AudioProvider::AudioThreadCallback()
{
    // Set priority to highest (audio thread is time critical)
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    HANDLE waitHandles[2];
    waitHandles[0] = _hFillNext;
    waitHandles[1] = _hShutdownRequest;

    bool audioStarted = false;
    bool done = false;
    do
    {
        DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE /* wait all = false */, INFINITE);
        if (waitResult == WAIT_FAILED)
        {
            // Shouldn't happen, but kill the thread if it does
            break;
        }

        if (waitResult == WAIT_OBJECT_0)
        {
            // We need to fill the next buffer and submit it
            FillAndSubmitNextBuffer();

            if (!audioStarted)
            {
                // Audio hasn't started yet.  Start the source voice and immediately fill the next buffer.
                _sourceVoice->Start(0);
                audioStarted = true;

                FillAndSubmitNextBuffer();
            }
        }
        else
        {
            done = true;
        }
    } while (!done);

    SetEvent(_hShutdownComplete);
}

void XA2AudioProvider::FillAndSubmitNextBuffer()
{
    _callbackFunc(_callbackData, _bufferMemory[_nextFillBuffer], SAMPLE_BUFFER_SIZE);
    _sourceVoice->SubmitSourceBuffer(&_buffers[_nextFillBuffer]);
    _nextFillBuffer = 1 - _nextFillBuffer;
}