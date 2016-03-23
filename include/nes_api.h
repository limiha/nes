#pragma once

#if defined(_WIN32)
  #if defined(NES_EXPORTS)
    #define NES_API __declspec(dllexport)
  #else
    #define NES_API __declspec(dllimport)
  #endif
#else
  #define NES_API
#endif

class Nes;

// Standard Controller Interface
class IStandardController
{
public:
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
    NES_API Nes* Nes_Create(const char* romPath);
    NES_API void Nes_DoFrame(Nes* nes, unsigned char screen[]);
    NES_API IStandardController* Nes_GetStandardController(Nes* nes, unsigned int port);
    NES_API void Nes_SaveState(Nes* nes);
    NES_API void Nes_LoadState(Nes* nes);
}