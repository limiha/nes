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

// Forward Declaratiosn
struct INes;
struct IStandardController;

struct INes
{
    virtual void DoFrame(unsigned char screen[]) = 0;
    virtual IStandardController* GetStandardController(unsigned int port) = 0;
    virtual void SaveState() = 0;
    virtual void LoadState() = 0;
};

// Standard Controller Interface
struct IStandardController
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
    NES_API INes* Nes_Create(const char* romPath);
}