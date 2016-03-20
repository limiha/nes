// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "nes.h"
#include "cpu.h"
#include "new_ppu.h"
#include "mem.h"
//#include "ppu.h"
#include "new_ppu.h"
#include "input.h"
#include "rom.h"
#include "apu.h"
#include "mapper.h"
#include "IGfx.h"
#include "IInput.h"

Nes::Nes(std::shared_ptr<Rom> rom, std::shared_ptr<IMapper> mapper)
    : _rom(rom)
    , _mapper(mapper)
{
    _ppu = std::make_shared<Ppu>(_mapper);
    _apu = std::make_shared<Apu>(false);
    _input = std::make_shared<Input>();
    _mem = std::make_shared<MemoryMap>(_ppu, _apu, _input, _mapper);
    _cpu = std::make_unique<Cpu>(_mem);

    // TODO: Move these to an init method
    _cpu->Reset();
    _apu->StartAudio(_mem.get(), 44100); 

    // TODO: Figure out controller creation/lifetime/port management
    _controller0 = std::make_unique<StandardController>();
    _input->Port0 = (IControllerPortDevice*) _controller0.get();
}

Nes::~Nes()
{
}

std::unique_ptr<Nes> Nes::Create(const char* romPath)
{
    auto rom = std::make_shared<Rom>();
    if (!rom->Load(romPath))
        return nullptr;

    return Nes::Create(rom);
}

std::unique_ptr<Nes> Nes::Create(std::shared_ptr<Rom> rom)
{
    auto mapper = IMapper::CreateMapper(rom);
    if (mapper == nullptr)
    {
        return nullptr;
    }

    return std::make_unique<Nes>(rom, mapper);
}

void Nes::DoFrame(u8 screen[])
{
    PpuStepResult ppuResult;
    ApuStepResult apuResult;
    do
    {
        ppuResult.Reset();
        apuResult.Reset();

        _cpu->Step();
        _apu->Step(_cpu->Cycles, _cpu->IsDmaRunning(), apuResult);
        _ppu->Step(_cpu->Cycles * 3, screen, ppuResult);
        
        _cpu->Cycles = 0;
        
        if (ppuResult.WantNmi)
        {
            _cpu->Nmi();
        }
        else if (apuResult.Irq || ppuResult.WantIrq)
        {
            _cpu->Irq();
        }
    } while (!ppuResult.VBlank);
}

void Nes::Run(IGfx* gfx, IHostInput* input)
{
    u8 screen[256 * 240 * 3];
    for (;;)
    {
        memset(screen, 0, sizeof(screen));

        // TODO: get joypadState
        InputResult result = input->CheckInput();

        if (result == InputResult::SaveState)
        {
            SaveState();
        }
        else if (result == InputResult::LoadState)
        {
            LoadState();
        }
        else if (result == InputResult::Quit)
        {
            break;
        }

        DoFrame(screen);
        gfx->Blit(screen);
    }

    _apu->StopAudio();
}

IStandardController* Nes::GetController0()
{
    return static_cast<IStandardController*>(_controller0.get());
}

void Nes::SaveState()
{
    std::ofstream ofs(GetSavePath()->c_str(), std::fstream::binary | std::fstream::trunc);
    _cpu->SaveState(ofs);
    ofs.close();

    printf("State Saved!\n");
}

void Nes::LoadState()
{
    auto savePath = GetSavePath();

    if (!fs::exists(*savePath))
    {
        printf("No save state for this ROM.\n");
        return;
    }

    std::ifstream ifs(GetSavePath()->c_str(), std::fstream::binary);
    _cpu->LoadState(ifs);
    ifs.close();

    printf("State Loaded!\n");
}

std::unique_ptr<fs::path> Nes::GetSavePath()
{
    fs::path savePath(_rom->Path());
    return std::make_unique<fs::path>(savePath.replace_extension("ns"));
}