#pragma once

#include "stdafx.h"
#include "cpu.h"
#include "decode.h"
#include "diassembler.h"

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
    Trace();
    Decode(LoadBBumpPC());
}

void Cpu::Decode(u8 op)
{
    IAddressingMode* am = nullptr;

    DECODE(op)

    if (am != nullptr)
    {
        delete am;
    }
}

void Cpu::Trace()
{
    Disassembler disassembler(_regs.PC, _mem);

    DisassembledInstruction* instruction;
    disassembler.Disassemble(&instruction);

    printf("%04X %-10s %-12s A:%02x X:%02X Y:%02X P:%02X S:%02X\n",
        _regs.PC,
        instruction->GetFormattedBytes().c_str(),
        instruction->GetDisassemblyString().c_str(),
        _regs.A,
        _regs.X,
        _regs.Y,
        _regs.P,
        _regs.S
        );
}