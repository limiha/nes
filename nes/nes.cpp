// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"

int main()
{
	Ppu ppu;
	MemoryMap mem(&ppu);
	Cpu cpu(&mem);

	cpu.Reset();



    return 0;
}

