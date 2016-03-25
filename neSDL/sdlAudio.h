#pragma once

class SdlAudioProvider : public IAudioProvider, public NesObject
{
public:
    SdlAudioProvider(int sampleRate);
    virtual ~SdlAudioProvider();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    // IAudioProvider implementation
    virtual void Initialize(AudioCallback* callback, void* callbackData);
    virtual void PauseAudio();
    virtual void UnpauseAudio();
    virtual int GetSampleRate();
    virtual int GetBitsPerSample();
    virtual int GetSilenceValue();

private:
    void AudioError(const char* error);

private:
    SDL_AudioDeviceID _deviceId;
    SDL_AudioFormat _format;
    int _sampleRate;
    int _bufferSize;
    int _silenceValue;
};