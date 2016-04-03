#include "stdafx.h"
#include "cpu.h"
#include "debug.h"
#include "decode.h"
#include "diassembler.h"

using namespace std;

Cpu::Cpu(
    IMem* mem,
    DebugService* debugger
    )
    : _mem(mem)
    , _debugger(debugger)
    , Cycles(0)
    , _dmaBytesRemaining(0)
    , _accumulatorAM(*this)
    , _immediateAM(*this)
    , _memoryAM(*this, 0)
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

// ISaveState
void Cpu::SaveState(std::ofstream& ofs)
{
    Util::WriteBytes(_regs.A, ofs);
    Util::WriteBytes(_regs.X, ofs);
    Util::WriteBytes(_regs.Y, ofs);
    Util::WriteBytes(_regs.P, ofs);
    Util::WriteBytes(_regs.S, ofs);
    Util::WriteBytes(_regs.PC, ofs);
    _mem->SaveState(ofs);
}

void Cpu::LoadState(std::ifstream& ifs)
{
    Util::ReadBytes(_regs.A, ifs);
    Util::ReadBytes(_regs.X, ifs);
    Util::ReadBytes(_regs.Y, ifs);
    Util::ReadBytes(_regs.P, ifs);
    Util::ReadBytes(_regs.S, ifs);
    Util::ReadBytes(_regs.PC, ifs);
    _mem->LoadState(ifs);
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

    if (_dmaBytesRemaining > 0)
    {
        // DMA is in progress.
        // The CPU is paused during the DMA process, but the APU and PPU continue to run.
        storeb(0x2004, loadb(_dmaReadAddress++));
        _dmaBytesRemaining--;
        Cycles += 2;

        return;
    }

    _debugger->OnBeforeExecuteInstruction(_regs.PC);
    _op = LoadBBumpPC();

    DECODE(_op)

    Cycles += CYCLE_TABLE[_op];
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

#if defined(TRACE)
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
#endif