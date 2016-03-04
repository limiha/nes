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
    , Cycles(0)
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
        Dma(val);
    }
    else
    {
        _mem->storeb(addr, val);
    }
}

void Cpu::Dma(u8 val)
{
    u16 addr = ((u16)val) << 8;

    for (u16 i = 0; i < 0x100; i++)
    {
        storeb(0x2004, loadb(addr++));
        Cycles += 2;
    }
}

void Cpu::Reset()
{
    _regs.PC = loadw(RESET_VECTOR);

}

#include <vector>
std::vector<u16> breakAddress;

void Cpu::Step()
{
#if defined(TRACE)
    Trace();
#endif

//#if defined(DEBUG)
//    if (_regs.PC == 0xb4d1)
//    {
//        __debugbreak();
//    }
//#endif

    _op = LoadBBumpPC();

    IAddressingMode* am = nullptr;
    DECODE(_op)

    Cycles += CYCLE_TABLE[_op];

    if (am != nullptr)
    {
        delete am;
    }
}

void Cpu::Nmi()
{
    PushW(_regs.PC);
    PushB(_regs.P);
    _regs.PC = loadw(NMI_VECTOR);
    Cycles += 7;
}

void Cpu::Irq()
{
    if (_regs.GetFlag(Flag::IRQ))
    {
        return;
    }

    PushW(_regs.PC);
    PushB(_regs.P);
    _regs.SetFlag(Flag::IRQ, true);
    _regs.PC = loadw(IRQ_VECTOR);
    Cycles += 7;
}

void Cpu::Trace()
{
    Disassembler disassembler(_regs.PC, _mem);

    DisassembledInstruction* instruction = nullptr;
    disassembler.Disassemble(&instruction);

    printf("%04X %-10s %-12s A:%02x X:%02X Y:%02X P:%02X S:%02X CY:%d\n",
        _regs.PC,
        instruction->GetFormattedBytes().c_str(),
        instruction->GetDisassemblyString().c_str(),
        _regs.A,
        _regs.X,
        _regs.Y,
        _regs.P,
        _regs.S,
        Cycles
        );

    if (instruction != nullptr)
    {
        delete instruction;
    }
}