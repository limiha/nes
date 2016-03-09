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

    Ppu ppu(mapper);
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
    bool wantSaveState = false;
    bool wantLoadState = false;
    for (;;)
    {
        apuResult.Reset();
        ppuResult.Reset();

         inputResult = input.CheckInput();
         if (inputResult == InputResult::SaveState)
         {
             wantSaveState = true;
         }
         else if (inputResult == InputResult::LoadState)
         {
             wantLoadState = true;
         }
         else if (inputResult == InputResult::Quit)
         {
             break;
         }

        _cpu->Step();

        apu.Step(_cpu->Cycles, _cpu->IsDmaRunning(), apuResult);
        ppu.Step(_cpu->Cycles * 3, ppuResult);

        if (ppuResult.VBlankNmi)
        {
            _cpu->Nmi();
        }
        else if (apuResult.Irq || ppuResult.WantIrq)
        {
            _cpu->Irq();
        }

        if (ppuResult.NewFrame)
        {
#if defined(RENDER_NAMETABLE)
            u8 nt_screen[256 * 240 * 3];
            for (int i = 0; i < 4; i++)
            {
                ppu.RenderNameTable(nt_screen, i);
                gfx.BlitNameTable(nt_screen, i);
            }
#endif
#if defined(RENDER_PATTERNTABLE)
            u8 pt_left[8 * 8 * 32 * 8 * 3];
            u8 pt_right[8 * 8 * 32 * 8 * 3];
            ppu.RenderPatternTable(0x0000, pt_left);
            ppu.RenderPatternTable(0x1000, pt_right);
            gfx.BlitPatternTable(pt_left, pt_right);
     
#endif
            gfx.Blit(ppu.Screen);
            calc_fps(last_time, frames);

            if (wantSaveState)
            {
                SaveState();
                wantSaveState = false;
            }
            if (wantLoadState)
            {
                LoadState();
                wantLoadState = false;
            }
        }

        _cpu->Cycles = 0;
    }

    apu.StopAudio();
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