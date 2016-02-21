// nes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpu.h"
#include "mem.h"

int main()
{

	MemoryMap mem;
	Cpu cpu(&mem);

	cpu.Reset();



    return 0;
}

