#pragma once

#include <nes_api.h>
#include <SDL_audio.h>

class SdlAudioProvider : public IAudioProvider
{
public:
    SdlAudioProvider(int sampleRate);
    virtual ~SdlAudioProvider();

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