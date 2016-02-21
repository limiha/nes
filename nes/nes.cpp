// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "input.h"

int main()
{
    Ppu ppu;
    Input input;
    MemoryMap mem(&ppu, &input);
    Cpu cpu(&mem);

    cpu.Reset();
    return 0;
}