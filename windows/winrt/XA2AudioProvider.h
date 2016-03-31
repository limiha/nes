#pragma once

class XA2VoiceCallback;

class XA2AudioProvider : public IAudioProvider, public NesObject
{
public:
    XA2AudioProvider(int sampleRate);
    virtual ~XA2AudioProvider();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // IAudioProvider implementation
public:
    virtual void Initialize(AudioCallback* callback, void* callbackData);
    virtual void PauseAudio();
    virtual void UnpauseAudio();
    virtual int GetSampleRate();
    virtual int GetBitsPerSample();
    virtual int GetSilenceValue();

private:
    HRESULT InitializeInternal(AudioCallback* callback, void* callbackData);

    void AudioThreadCallback();
    void FillAndSubmitNextBuffer();

private:
    int _sampleRate;
    int _nextFillBuffer;
    bool _initialized;
    bool _paused;
    bool _firstUnpause;
    AudioCallback* _callbackFunc;
    void* _callbackData;
    HANDLE _hFillNext;
    HANDLE _hShutdownRequest;
    HANDLE _hShutdownComplete;
    CComPtr<IXAudio2> _xaudio2;
    CAutoPtr<IXAudio2MasteringVoice> _masterVoice;
    CAutoPtr<IXAudio2SourceVoice> _sourceVoice;
    CAutoPtr<XA2VoiceCallback> _voiceCallback;
    CAutoVectorPtr<BYTE> _bufferMemory[2];
    XAUDIO2_BUFFER _buffers[2];

    friend void XA2ThreadFunc(void *params);
};