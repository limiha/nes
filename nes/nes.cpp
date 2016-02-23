// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "input.h"
#include "rom.h"
#include "apu.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    Rom rom;
    rom.Load(std::string(argv[1]));

    VRam vram(rom);

    Ppu ppu(vram);
    Apu apu;
    Input input;
    MemoryMap mem(ppu, apu, input, rom);
    Cpu cpu(&mem);

    cpu.Reset();

    PpuStepResult ppuResult;
    for (;;)
    {
        ppuResult.Reset();

        cpu.Step();

        ppu.Step(cpu.Cycles, ppuResult);
        if (ppuResult.VBlankNmi)
        {
            cpu.Nmi();
        } 
        // else if IRQ
    }

    return 0;
}