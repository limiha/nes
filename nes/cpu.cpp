#pragma once

#include "stdafx.h"
#include "cpu.h"

using namespace std;

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

void Cpu::Step()
{
    Decode(LoadBBumpPC());
}

void Cpu::Decode(u8 op)
{
    IAddressingMode* am;

    switch (op)
    {
        // Loads
    case 0xa1: IndirectIndexedX(am);    lda(am);    break;
    case 0xa5: ZeroPage(am);            lda(am);    break;
    case 0xa9: Immediate(am);           lda(am);    break;
    case 0xad: Absolute(am);            lda(am);    break;
    case 0xb1: IndirectIndexedY(am);    lda(am);    break;
    case 0xb5: ZeroPageX(am);           lda(am);    break;
    case 0xb0: AbsoluteY(am);           lda(am);    break;
    case 0xbd: AbsoluteX(am);           lda(am);    break;
    }
}