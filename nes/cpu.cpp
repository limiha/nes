#pragma once

#include "stdafx.h"
#include "cpu.h"
#include "decode.h"
#include "diassembler.h"

using namespace std;

Cpu::Cpu(
    IMem& mem
    )
    : _mem(mem)
    , Cycles(0)
    , _dmaBytesRemaining(0)
{
}

Cpu::~Cpu() 
{
}

// IMem
u8 Cpu::loadb(u16 addr)
{
    return _mem.loadb(addr);
}

void Cpu::storeb(u16 addr, u8 val)
{
    if (addr == 0x4014)
    {
        Dma(val);
    }
    else
    {
        _mem.storeb(addr, val);
    }
}

// ISave
void Cpu::Save(std::ofstream& ofs)
{
    ofs << _regs.A;
    ofs << _regs.X;
    ofs << _regs.Y;
    ofs << _regs.P;
    ofs << _regs.S;
    ofs << _regs.PC;
    _mem.Save(ofs);
}

void Cpu::Load(std::ifstream& ifs)
{
    ifs >> _regs.A;
    ifs >> _regs.X;
    ifs >> _regs.Y;
    ifs >> _regs.P;
    ifs >> _regs.S;
    ifs >> _regs.PC;
    _mem.Load(ifs);
}

void Cpu::Dma(u8 val)
{
    _dmaReadAddress = ((u16)val) << 8;
    _dmaBytesRemaining = 0x0100;
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

    if (_dmaBytesRemaining > 0)
    {
        // DMA is in progress.
        // The CPU is paused during the DMA process, but the APU and PPU continue to run.
        storeb(0x2004, loadb(_dmaReadAddress++));
        _dmaBytesRemaining--;
        Cycles += 2;

        return;
    }

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