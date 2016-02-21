#pragma once

#include "mem.h"

// CPU Interrupt Vectors
const u16 NMI_VECTOR	= 0xfffa;
const u16 RESET_VECTOR	= 0xfffc;
const u16 IRQ_VECTGOR	= 0xfffe;

// CPU Register Flags
const u8 CARRY_FLAG		= 1 << 0;
const u8 ZERO_FLAG		= 1 << 1;
const u8 IRQ_FLAG		= 1 << 2;
const u8 DECIMAL_FLAG	= 1 << 3;
const u8 BREAK_FLAG		= 1 << 4;
const u8 OVERFLOW_FLAG	= 1 << 6;
const u8 NEGATIVE_FLAG	= 1 << 7;

struct CpuRegs
{
	u8 A;	// Accumulator
	u8 X;	// Index Register
	u8 Y;	// Index Register
	u8 P;	// Process Status
	u8 SP;	// Stack Pointer

	u16 PC; // Program Counter

	CpuRegs()
		: A(0)
		, X(0)
		, Y(0)
		, P(DECIMAL_FLAG | (1 << 5)) // DECIMAL_FLAG is always set on nes and bit 5 is unused, always set
		, SP(0xfd)
		, PC(0x8000)
	{
	}
};

class Cpu : public IMem
{
public:
	Cpu(IMem* mem);
	~Cpu();

	u8 loadb(u16 addr);
	void storeb(u16 addr, u8 val);

	void Reset();

private:
	CpuRegs _regs;
	IMem* _mem;;
};