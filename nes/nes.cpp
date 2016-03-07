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
#include "gfx.h"
#include "mapper.h"

#include <chrono>

void calc_fps(std::chrono::time_point<std::chrono::high_resolution_clock>& last_time, u32& frames)
{
    auto now = std::chrono::high_resolution_clock::now();

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - last_time).count();
    if (seconds >= 1)
    {
        printf("%d\n", frames);
        frames = 0;
        last_time = now;
    }
    else 
    {
        frames++;
    }
}

#if defined(RENDER_NAMETABLE)
u8 nt_screen[256 * 240 * 3];
#endif

Nes::Nes(std::shared_ptr<Rom> rom)
    : _rom(rom)
{
}

Nes::~Nes()
{
}

std::unique_ptr<Nes> Nes::Create(const char* romPath)
{
    auto rom = std::make_shared<Rom>();
    if (!rom->Load(romPath))
    {
        printf("Incompatible ROM\n");
        return nullptr;
    }

    return std::make_unique<Nes>(rom);
}

std::unique_ptr<Nes> Nes::Create(std::shared_ptr<Rom> rom)
{
    return std::make_unique<Nes>(rom);
}

void Nes::Run()
{
    Gfx gfx(3);

    std::shared_ptr<IMapper> mapper = IMapper::CreateMapper(_rom);
    if (mapper == nullptr)
    {
        return;
    }

    VRam vram(mapper);

    Ppu ppu(vram);
    Apu apu(false /* isPal */);
    Input input;
    MemoryMap mem(ppu, apu, input, mapper);
    //Cpu cpu(&mem);
    _cpu = std::make_unique<Cpu>(mem);

    _cpu->Reset();
    
    apu.StartAudio(44100);

    auto last_time = std::chrono::high_resolution_clock::now();
    u32 frames = 0;

    InputResult inputResult;
    ApuStepResult apuResult;
    PpuStepResult ppuResult;
    for (;;)
    {
        apuResult.Reset();
        ppuResult.Reset();

         inputResult = input.CheckInput();
         if (inputResult == InputResult::SaveState)
         {
             SaveState();
         }
         else if (inputResult == InputResult::LoadState)
         {
             LoadState();
         }

        _cpu->Step();

        apu.Step(_cpu->Cycles, apuResult);
        ppu.Step(_cpu->Cycles * 3, ppuResult);

        if (ppuResult.VBlankNmi)
        {
            _cpu->Nmi();
        } 
        // else if IRQ

        if (ppuResult.NewFrame)
        {
#if defined(RENDER_NAMETABLE)
            for (int i = 0; i < 4; i++)
            {
                ppu.RenderNameTable(nt_screen, i);
                gfx.BlitNameTable(nt_screen, i);
            }
#endif
            gfx.Blit(ppu.Screen);
            calc_fps(last_time, frames);
        }

        _cpu->Cycles = 0;
    }

    apu.StopAudio();
}

void Nes::SaveState()
{
    std::ofstream ofs("savestate.savestate", std::fstream::binary | std::fstream::trunc);
    _cpu->Save(ofs);
}

void Nes::LoadState()
{
    std::ifstream ifs("savestate.savestate", std::fstream::binary);
    _cpu->Load(ifs);
}