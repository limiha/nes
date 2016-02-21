#pragma once

#include "stdafx.h"
#include "cpu.h"

Cpu::Cpu(
	IMem* mem
	) 
	: _mem(mem)
{
}

Cpu::~Cpu() 
{
}

// IMem
u8 Cpu::loadb(u16 addr)
{
	return _mem->loadb(addr);
}

void Cpu::storeb(u16 addr, u8 val)
{
	if (addr == 0x4014)
	{
		// TODO: DMA
	}
	else
	{
		_mem->storeb(addr, val);
	}
}

void Cpu::Reset()
{
	_regs.PC = loadw(RESET_VECTOR);
}