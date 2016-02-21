#pragma once

#include "stdafx.h"
#include "mem.h"

Ram::Ram() 
{
	ZeroMemory(_ram, sizeof(_ram));
}

Ram::~Ram()
{

}

u8 Ram::loadb(u16 addr)
{
	return _ram[addr & 0x7ff];
}

void Ram::storeb(u16 addr, u8 val)
{
	_ram[addr & 0x7ff] = val;
}

MemoryMap::MemoryMap()
{

}

MemoryMap::~MemoryMap()
{

}

u8 MemoryMap::loadb(u16 addr)
{
	if (addr < 0x2000)
	{
		return _ram.loadb(addr);
	}
	else if (addr < 0x8000)
	{
		// TODO: ppu
		// TODO: input
		// TOOD: apu
		// TODO: Expansion ROM
		// TODO: SRAM
		return 0;
	}
	else
	{
		// TODO: PRG-ROM
		// TODO: Mapper
		return 0;
	}
}

void MemoryMap::storeb(u16 addr, u8 val)
{
	if (addr < 0x2000)
	{
		_ram.storeb(addr, val);
	}
	else if (addr < 0x8000)
	{
		// TODO: ppu
		// TODO: input
		// TOOD: apu
		// TODO: Expansion ROM
		// TODO: SRAM
	}
	else
	{
		// TODO PRG-ROM
		// TODO Mapper
	}
}