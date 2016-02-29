// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "input.h"
#include "rom.h"
#include "apu.h"
#include "gfx.h"

#include <time.h>

void calc_fps(time_t& last_time, u32& frames)
{
    time_t now = time(0);
    if (now >= last_time + 1)
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

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("No ROM specified\n");
        return -1;
    }

    Gfx gfx(3);

    Rom rom;
    if (!rom.Load(std::string(argv[1])))
    {
        printf("Incompatible ROM\n");
        return -1;
    }

    VRam vram(rom);

    Ppu ppu(vram);
    Apu apu;
    Input input;
    MemoryMap mem(ppu, apu, input, rom);
    Cpu cpu(&mem);

    cpu.Reset();

    time_t last_time = time(0);
    u32 frames = 0;

    PpuStepResult ppuResult;
    for (;;)
    {
        ppuResult.Reset();

        input.CheckInput();

        cpu.Step();

        ppu.Step(cpu.Cycles, ppuResult);
        if (ppuResult.VBlankNmi)
        {
            cpu.Nmi();
        } 
        // else if IRQ

        if (ppuResult.NewFrame)
        {
            gfx.Blit(ppu.Screen);
            calc_fps(last_time, frames);
        }
    }

    return 0;
}