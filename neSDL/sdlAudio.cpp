#include "stdafx.h"
#include "sdlAudio.h"

#include <SDL.h>

#define SAMPLE_BUFFER_SIZE 1024

SdlAudioProvider::SdlAudioProvider(int sampleRate)
    : _deviceId(0)
    , _format(AUDIO_U8)
    , _sampleRate(sampleRate)
    , _bufferSize(SAMPLE_BUFFER_SIZE)
    , _silenceValue(0)
{
}

SdlAudioProvider::~SdlAudioProvider()
{
    if (_deviceId != 0)
    {
        SDL_CloseAudioDevice(_deviceId);

        _deviceId = 0;
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SdlAudioProvider::Initialize(AudioCallback* callback, void* callbackData)
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec desired = { 0 };
    SDL_AudioSpec obtained = { 0 };
    desired.format = _format;
    desired.channels = 1;
    desired.freq = _sampleRate;
    desired.samples = _bufferSize;
    desired.callback = callback;
    desired.userdata = callbackData;

    _deviceId = SDL_OpenAudioDevice(
        nullptr /* Choose reasonable default device */,
        false /* Capture disabled */,
        &desired,
        &obtained,
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (_deviceId == 0)
    {
        AudioError(SDL_GetError());
        return;
    }

    _sampleRate = obtained.freq;
    _silenceValue = obtained.silence;
}

void SdlAudioProvider::PauseAudio()
{
    SDL_PauseAudioDevice(_deviceId, 1);
}

void SdlAudioProvider::UnpauseAudio()
{
    SDL_PauseAudioDevice(_deviceId, 0);
}

int SdlAudioProvider::GetSampleRate()
{
    return _sampleRate;
}

int SdlAudioProvider::GetBitsPerSample()
{
    return 8; // TODO: Multi-bitrate support
}

int SdlAudioProvider::GetSilenceValue()
{
    return _silenceValue;
}

void SdlAudioProvider::AudioError(const char* error)
{
    // TODO: Do something different in the emulator?
    printf_s("Audio Engine Error: %s", error);
}