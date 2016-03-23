#pragma once

#include "mem.h"

// Base Cycle Counts 
static u8 CYCLE_TABLE[0x100] = {
    /*0x00*/ 7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,
    /*0x10*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x20*/ 6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,
    /*0x30*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x40*/ 6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,
    /*0x50*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x60*/ 6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,
    /*0x70*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x80*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    /*0x90*/ 2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,
    /*0xA0*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    /*0xB0*/ 2,5,2,5,4,4,4,4,2,4,2,4,4,4,4,4,
    /*0xC0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    /*0xD0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0xE0*/ 2,6,3,8,3,3,5,5,2,2,2,2,4,4,6,6,
    /*0xF0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
};

// CPU Interrupt Vectors
const u16 NMI_VECTOR    = 0xfffa;
const u16 RESET_VECTOR  = 0xfffc;
const u16 IRQ_VECTOR    = 0xfffe;

// CPU Status Register Flags
enum class Flag : u8
{
    Carry       = 1 << 0,
    Zero        = 1 << 1,
    IRQ         = 1 << 2,
    Decimal     = 1 << 3,
    Break       = 1 << 4,
    Unused      = 1 << 5,
    Overflow    = 1 << 6,
    Negative    = 1 << 7
};

struct CpuRegs
{
    u8 A;    // Accumulator
    u8 X;    // Index Register
    u8 Y;    // Index Register
    u8 P;    // Process Status
    u8 S;    // Stack Pointer

    u16 PC; // Program Counter

    CpuRegs()
        : A(0)
        , X(0)
        , Y(0)
        , P((u8)Flag::Unused | (u8)Flag::IRQ) // DECIMAL_FLAG is always set on nes and bit 5 is unused, always set
        , S(0xfd) // Startup value according to http://wiki.nesdev.com/w/index.php/CPU_power_up_state
        , PC(0x8000)
    {
    }

    bool GetFlag(Flag flag)
    {
        return (P & (u8)flag) != 0;
    }
    void SetFlag(Flag flag, bool on)
    {
        if (on)
        {
            P |= (u8)flag;
        }
        else
        {
            P &= ~((u8)flag);
        }
    }
    u8 SetZN(u8 val) {
        SetFlag(Flag::Zero, val == 0);
        SetFlag(Flag::Negative, (val & 0x80) != 0);
        return val;
    }
};

class Cpu : public IMem
{
public:
    Cpu(std::shared_ptr<IMem>);
    ~Cpu();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

    void Reset();
    void Step();
    void Nmi();
    void Irq();

    bool IsDmaRunning()
    {
        return _dmaBytesRemaining > 0;
    }

public:
    u32 Cycles;

private:
    CpuRegs _regs;
    std::shared_ptr<IMem> _mem;

    u8 _op;
    u32 _dmaBytesRemaining;
    u32 _dmaReadAddress;

private:
    void Dma(u8 val);
    void Trace();

    // Adressing Modes
    class IAddressingMode 
    {
    public:
        IAddressingMode(Cpu& cpu) : _cpu(cpu) { }
        virtual u8 Load() = 0;
        virtual void Store(u8 val) = 0;
    protected:
        Cpu& _cpu;
    };

    class AccumulatorAddressingMode : public IAddressingMode 
    {
    public:
        AccumulatorAddressingMode(Cpu& cpu) : IAddressingMode(cpu) { }
        u8 Load() { return _cpu._regs.A; }
        void Store(u8 val) { _cpu._regs.A = val; }
    };

    class ImmediateAddressingMode : public IAddressingMode 
    {
    public:
        ImmediateAddressingMode(Cpu& cpu) : IAddressingMode(cpu) { }
        u8 Load() { return _cpu.LoadBBumpPC(); }
        void Store(u8 val) { /* Can't store to immediate */ }
    };

    class MemoryAddressingMode : public IAddressingMode 
    {
    public:
        MemoryAddressingMode(Cpu& cpu, u16 addr) : IAddressingMode(cpu), _addr(addr) { }
        u8 Load() { return _cpu.loadb(_addr); }
        void Store(u8 val) { _cpu.storeb(_addr, val); }
    private:
        u16 _addr;
    };

    void Immediate(IAddressingMode* &am) { am = new ImmediateAddressingMode(*this); }
    void Accumulator(IAddressingMode* &am) { am = new AccumulatorAddressingMode(*this); }
    void ZeroPage(IAddressingMode* &am) { am = new MemoryAddressingMode(*this, (u16)LoadBBumpPC()); }
    void ZeroPageX(IAddressingMode* &am) { am = new MemoryAddressingMode(*this, (u16)(u8)(LoadBBumpPC() + _regs.X)); }
    void ZeroPageY(IAddressingMode* &am) { am = new MemoryAddressingMode(*this, (u16)(u8)(LoadBBumpPC() + _regs.Y)); }
    void Absolute(IAddressingMode* &am) { am = new MemoryAddressingMode(*this, LoadWBumpPC()); }
    
    void AbsoluteX(IAddressingMode* &am)
    {
        u16 addr = LoadWBumpPC();
        u16 indexedAddr = addr + (u16)_regs.X;

        if (_op != 0x1e // asl
            && _op != 0xde // dec
            && _op != 0x5e // lsr
            && _op != 0x3e // rol
            && _op != 0x7e // ror
            && _op != 0x9d // sta
            )
        {
            checkPageCross(addr, indexedAddr);
        }

        am = new MemoryAddressingMode(*this, indexedAddr);
    }
    void AbsoluteY(IAddressingMode* &am)
    {
        u16 addr = LoadWBumpPC();
        u16 indexedAddr = addr + (u16)_regs.Y;

        if (_op != 0x99) // sta
        {
            checkPageCross(addr, indexedAddr);
        }
        am = new MemoryAddressingMode(*this, indexedAddr);
    }
    
    void IndexedIndirectX(IAddressingMode* &am) { am = new MemoryAddressingMode(*this, loadw_zp(LoadBBumpPC() + _regs.X)); }

    void IndirectIndexedY(IAddressingMode* &am)
    {
        u16 addr = loadw_zp(LoadBBumpPC());
        u16 indexedAddr = addr + (u16)_regs.Y;
        checkPageCross(addr, indexedAddr);
        am = new MemoryAddressingMode(*this, indexedAddr);
    }

    // Memory Acess Helpers
    u8 LoadBBumpPC() { return loadb(_regs.PC++); }
    u16 LoadWBumpPC()
    {
        u16 val = loadw(_regs.PC);
        _regs.PC += 2;
        return val;
    }

    void checkPageCross(u16 lhs, u16 rhs)
    {
        if ((lhs & 0xff00) != (rhs & 0xff00))
        {
            Cycles++;
        }
    }

    // Stack Helpers
    void PushB(u8 val)
    {
        storeb(0x100 | (u16)_regs.S, val);
        _regs.S--;
    }

    void PushW(u16 val)
    {
        PushB((val >> 8) & 0xff);
        PushB(val & 0xff);
    }

    u8 PopB()
    {
        u8 val = loadb(0x100 | (u16)(++_regs.S));

        return val;
    }

    u16 PopW()
    {
        u16 lo = (u16)PopB();
        u16 hi = (u16)PopB();

        return (hi << 8) | lo;
    }

    // Instructions

    // Loads
    void lda(IAddressingMode* am) { _regs.A = _regs.SetZN(am->Load()); }
    void ldx(IAddressingMode* am) { _regs.X = _regs.SetZN(am->Load()); }
    void ldy(IAddressingMode* am) { _regs.Y = _regs.SetZN(am->Load()); }

    // Stores
    void sta(IAddressingMode* am) { am->Store(_regs.A); }
    void stx(IAddressingMode* am) { am->Store(_regs.X); }
    void sty(IAddressingMode* am) { am->Store(_regs.Y); }

    // Arithemtic
    void adc(IAddressingMode* am) 
    {
        u8 val = am->Load();
        u32 result = (u32)_regs.A + (u32)val;
        if (_regs.GetFlag(Flag::Carry)) result += 1;
        _regs.SetFlag(Flag::Carry, (result & 0x100) != 0);

        u8 resultByte = result & 0xff;
        u8 a = _regs.A;
        _regs.SetFlag(Flag::Overflow, (((a ^ val) & 0x80) == 0) && (((a ^ resultByte) & 0x80) == 0x80));
        _regs.A = _regs.SetZN(resultByte);
    }
    void sbc(IAddressingMode* am)
    {
        u8 val = am->Load();
        u32 result = (u32)_regs.A - (u32)val;
        if (!_regs.GetFlag(Flag::Carry)) result -= 1;
        _regs.SetFlag(Flag::Carry, (result & 0x100) == 0);

        u8 resultByte = result & 0xff;
        u8 a = _regs.A;
        _regs.SetFlag(Flag::Overflow, (((a ^ resultByte) & 0x80) != 0) && ((a ^ val) & 0x80) == 0x80);
        _regs.A = _regs.SetZN(resultByte);
    }

    // Comparisons
    void cmp_base(IAddressingMode* am, u8 val)
    {
        u32 result = (u32)val - (u32)am->Load();
        _regs.SetFlag(Flag::Carry, (result & 0x100) == 0);
        _regs.SetZN((u8)result);
    }
    void cmp(IAddressingMode* am) { cmp_base(am, _regs.A); }
    void cpx(IAddressingMode* am) { cmp_base(am, _regs.X); }
    void cpy(IAddressingMode* am) { cmp_base(am, _regs.Y); }

    // Bitwise Operations
    void and(IAddressingMode* am) { _regs.A = _regs.SetZN(_regs.A & am->Load()); }
    void ora(IAddressingMode* am) { _regs.A = _regs.SetZN(_regs.A | am->Load()); }
    void eor(IAddressingMode* am) { _regs.A = _regs.SetZN(_regs.A ^ am->Load()); }
    void bit(IAddressingMode* am) 
    {
        u8 val = am->Load();
        _regs.SetFlag(Flag::Zero, (val &_regs.A) == 0);
        _regs.SetFlag(Flag::Negative, (val & (1 << 7)) != 0);
        _regs.SetFlag(Flag::Overflow, (val & (1 << 6)) != 0);
    }

    // Shifts and Rotates
    void shl_base(bool lsb, IAddressingMode* am)
    {
        u8 val = am->Load();
        bool newCarry = (val & 0x80) != 0;
        u8 result = (val << 1) | (lsb ? 1 : 0);
        _regs.SetFlag(Flag::Carry, newCarry);
        am->Store(_regs.SetZN(result));
    }
    void shr_base(bool msb, IAddressingMode* am)
    {
        u8 val = am->Load();
        bool newCarry = (val & 0x01) != 0;
        u8 result = (val >> 1) | (msb ? 0x80 : 0);
        _regs.SetFlag(Flag::Carry, newCarry);
        am->Store(_regs.SetZN(result));
    }
    void rol(IAddressingMode* am)
    {
        bool oldCarry = _regs.GetFlag(Flag::Carry);
        shl_base(oldCarry, am);
    }
    void ror(IAddressingMode* am)
    {
        bool oldCarry = _regs.GetFlag(Flag::Carry);
        shr_base(oldCarry, am);
    }
    void asl(IAddressingMode* am) { shl_base(false, am); }
    void lsr(IAddressingMode* am) { shr_base(false, am); }

    // Increments and Decrements
    void inc(IAddressingMode* am) { am->Store(_regs.SetZN(am->Load() + 1)); }
    void dec(IAddressingMode* am) { am->Store(_regs.SetZN(am->Load() - 1)); }
    void inx(IAddressingMode* am) { _regs.SetZN(++_regs.X); }
    void dex(IAddressingMode* am) { _regs.SetZN(--_regs.X); }
    void iny(IAddressingMode* am) { _regs.SetZN(++_regs.Y); }
    void dey(IAddressingMode* am) { _regs.SetZN(--_regs.Y); }

    // Register Moves
    void tax(IAddressingMode*) { _regs.X = _regs.SetZN(_regs.A); }
    void tay(IAddressingMode*) { _regs.Y = _regs.SetZN(_regs.A); }
    void txa(IAddressingMode*) { _regs.A = _regs.SetZN(_regs.X); }
    void tya(IAddressingMode*) { _regs.A = _regs.SetZN(_regs.Y); }
    void txs(IAddressingMode*) { _regs.S = _regs.X; }
    void tsx(IAddressingMode*) { _regs.X = _regs.SetZN(_regs.S); }

    // Flag Operations
    // FIXME: The way the decode macro is written and shared between this and the disassembler
    // FIXME: means that all these functions must take an IAdressingMode even though they don't use it.
    void clc(IAddressingMode* am) { _regs.SetFlag(Flag::Carry, false); }
    void sec(IAddressingMode* am) { _regs.SetFlag(Flag::Carry, true); }
    void cli(IAddressingMode* am) { _regs.SetFlag(Flag::IRQ, false); }
    void sei(IAddressingMode* am) { _regs.SetFlag(Flag::IRQ, true); }
    void clv(IAddressingMode* am) { _regs.SetFlag(Flag::Overflow, false); }
    void cld(IAddressingMode* am) { _regs.SetFlag(Flag::Decimal, false); }
    void sed(IAddressingMode* am) { _regs.SetFlag(Flag::Decimal, true); }

    // Branches
    void branch_base(bool cond)
    {
        i8 disp = (i8)LoadBBumpPC();
        if (cond)
        {
            Cycles++;
            u16 newPC = (u16)((i32)_regs.PC + (i32)disp);
            checkPageCross(_regs.PC, newPC);

            _regs.PC = newPC;
        }
    }
    void bpl(IAddressingMode* am) { branch_base(!_regs.GetFlag(Flag::Negative)); }
    void bmi(IAddressingMode* am) { branch_base(_regs.GetFlag(Flag::Negative)); }
    void bvc(IAddressingMode* am) { branch_base(!_regs.GetFlag(Flag::Overflow)); }
    void bvs(IAddressingMode* am) { branch_base(_regs.GetFlag(Flag::Overflow)); }
    void bcc(IAddressingMode* am) { branch_base(!_regs.GetFlag(Flag::Carry)); }
    void bcs(IAddressingMode* am) { branch_base(_regs.GetFlag(Flag::Carry)); }
    void bne(IAddressingMode* am) { branch_base(!_regs.GetFlag(Flag::Zero)); }
    void beq(IAddressingMode* am) { branch_base(_regs.GetFlag(Flag::Zero)); }

    // Jumps
    void jmp(IAddressingMode* am)
    {
        _regs.PC = LoadWBumpPC();
    }
    void jmpi(IAddressingMode* am)
    {
        u16 addr = LoadWBumpPC();

        // recreate processor bug
        u16 lo = (u16)loadb(addr);
        u16 hi = (u16)loadb((addr & 0xff00) | ((addr + 1) & 0x00ff));
        
        _regs.PC = (hi << 8) | lo;
    }

    // Procedure Calls
    void jsr(IAddressingMode* am) 
    {
        // jsr pushes the address of the NEXT instruction MINUS ONE
        // weird, I know.
        u16 target = LoadWBumpPC();
        PushW(_regs.PC - 1);
        _regs.PC = target;
    }
    void rts(IAddressingMode* am)
    {
        _regs.PC = PopW() + 1;
    }
    void brk(IAddressingMode* am)
    {
        PushW(_regs.PC + 1);
        PushB(_regs.P | (u8)Flag::Break | (u8)Flag::Unused);
        _regs.SetFlag(Flag::IRQ, true);
        _regs.SetFlag(Flag::Unused, true);
        _regs.PC = loadw(IRQ_VECTOR);
    }
    void rti(IAddressingMode* am)
    {
        _regs.P = PopB() & ~((u8)Flag::Break);
        _regs.PC = PopW();
    }

    // Stack Operations
    void pha(IAddressingMode* am) { PushB(_regs.A); }
    void pla(IAddressingMode* am) { _regs.A = _regs.SetZN(PopB()); }
    void php(IAddressingMode* am) { PushB(_regs.P | (u8)Flag::Break | (u8)Flag::Unused); } // FIXME: is b_flag right here?
    void plp(IAddressingMode* am) { _regs.P = PopB(); }

    // No Operation
    void nop(IAddressingMode* am) { }
};
