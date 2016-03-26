#pragma once

class Rom;
class Cpu;
class DebugService;
class MemoryMap;
class Ppu;
class Apu;
class Input;

#include "interfaces.h"
#include "..\include\nes_api.h"

class Nes : public INes, public NesObject
{
public:
    Nes(Rom* rom, IMapper* mapper, IAudioProvider* audioProvider);
    virtual ~Nes();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    static bool Create(const char* romPath, IAudioProvider* audioProvider, Nes** nes);
    static bool Create(Rom* rom, IAudioProvider* audioProvider, Nes** nes);

    virtual void Dispose();

    // DoFrame runs all nes components until the ppu hits VBlank
    // This means that one call to DoFrame will render scanlines 241 - 261 then 0 - 240
    // The joypad state provided will be used for the entirety of the frame
    // screen is the pixel data buffer that the ppu will write to
    void DoFrame(u8 screen[]);

    // Gets a standard Nes controller on the specified port
    // Port can only be 0 or 1
    // If there is an existing device on the port,
    // it will be disconnected and it's memory will be freed.
    IStandardController* GetStandardController(unsigned int port);

    void SaveState();
    void LoadState();

private:
    std::unique_ptr<fs::path> GetSavePath();

private:
    NPtr<Rom> _rom;
    NPtr<IMapper> _mapper;
    NPtr<Apu> _apu;
    NPtr<Ppu> _ppu;
    NPtr<Input> _input;
    NPtr<MemoryMap> _mem;
    NPtr<Cpu> _cpu;
    NPtr<DebugService> _debugger;
};