// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "input.h"
#include "rom.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    Rom rom;
    rom.Load(std::string(argv[1]));

    Ppu ppu;
    Input input;
    MemoryMap mem(&ppu, &input, &rom);
    Cpu cpu(&mem);

    cpu.Reset();

    for (;;)
    {
        cpu.Step();
        ppu.Step(cpu.Cycles);
    }

    return 0;
}