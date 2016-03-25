#pragma once

#include "baseinterface.h"
#include "nptr.h"
#include "object.h"

#if defined(_WIN32)
  #if defined(NES_EXPORTS)
    #define NES_API __declspec(dllexport)
  #else
    #define NES_API __declspec(dllimport)
  #endif
#else
  #define NES_API
#endif

// Forward Declaratiosn
struct INes;
struct IStandardController;

struct INes : public IBaseInterface
{
    virtual void DoFrame(unsigned char screen[]) = 0;
    virtual IStandardController* GetStandardController(unsigned int port) = 0;
    virtual void SaveState() = 0;
    virtual void LoadState() = 0;
};

// Audio interface (implemented by host)
typedef void AudioCallback(void *userdata, unsigned char *stream, int len);
struct IAudioProvider : public IBaseInterface
{
    // Called by nes with its callback when it's ready to begin audio rendering
    virtual void Initialize(AudioCallback* callback, void* callbackData) = 0;

    // The following calls will only be made after Initialize is called
    virtual void PauseAudio() = 0;
    virtual void UnpauseAudio() = 0;
    virtual int GetSampleRate() = 0;
    virtual int GetBitsPerSample() = 0;
    virtual int GetSilenceValue() = 0;
};

// Standard Controller Interface
struct IStandardController : public IBaseInterface
{
    virtual void A(bool state) = 0;
    virtual void B(bool state) = 0;
    virtual void Select(bool state) = 0;
    virtual void Start(bool state) = 0;
    virtual void Up(bool state) = 0;
    virtual void Down(bool state) = 0;
    virtual void Left(bool state) = 0;
    virtual void Right(bool state) = 0;
};

extern "C"
{
    NES_API bool Nes_Create(const char* romPath, IAudioProvider* audioProvider, INes** ines);
}