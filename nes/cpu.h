#pragma once

#include "mem.h"

// CPU Interrupt Vectors
const u16 NMI_VECTOR    = 0xfffa;
const u16 RESET_VECTOR    = 0xfffc;
const u16 IRQ_VECTGOR    = 0xfffe;

// CPU Status Register Flags
enum class Flag : u8
{
    Carry       = 1 << 0,
    Zero        = 1 << 1,
    IRQ         = 1 << 2,
    Decimal     = 1 << 3,
    Break       = 1 << 4,
    Overflow    = 1 << 6,
    Negative    = 1 << 7
};

struct CpuRegs
{
    u8 A;    // Accumulator
    u8 X;    // Index Register
    u8 Y;    // Index Register
    u8 P;    // Process Status
    u8 SP;    // Stack Pointer

    u16 PC; // Program Counter

    CpuRegs()
        : A(0)
        , X(0)
        , Y(0)
        , P((u8)Flag::Decimal | (1 << 5)) // DECIMAL_FLAG is always set on nes and bit 5 is unused, always set
        , SP(0xfd) // Startup value according to http://wiki.nesdev.com/w/index.php/CPU_power_up_state
        , PC(0x8000)
    {
    }
};

class Cpu : public IMem
{
public:
    Cpu(IMem* mem);
    ~Cpu();

    //IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void Reset();

    void Step();

private:
    CpuRegs _regs;
    IMem* _mem;

private:
    void Decode(u8 op);
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

    void Immediate(IAddressingMode* am) { am = new ImmediateAddressingMode(*this); }
    void Accumulator(IAddressingMode* am) { am = new AccumulatorAddressingMode(*this); }
    void ZeroPage(IAddressingMode* am) { am = new MemoryAddressingMode(*this, (u16)LoadBBumpPC()); }
    void ZeroPageX(IAddressingMode* am) { am = new MemoryAddressingMode(*this, LoadBBumpPC() + _regs.X); }
    void ZeroPageY(IAddressingMode* am) { am = new MemoryAddressingMode(*this, LoadBBumpPC() + _regs.Y); }
    void Absolute(IAddressingMode* am) { am = new MemoryAddressingMode(*this, LoadWBumpPC()); }
    void AbsoluteX(IAddressingMode* am) { am = new MemoryAddressingMode(*this, LoadWBumpPC() + _regs.X); }
    void AbsoluteY(IAddressingMode* am) { am = new MemoryAddressingMode(*this, LoadWBumpPC() + _regs.Y); }
    void IndirectIndexedX(IAddressingMode* am) { am = new MemoryAddressingMode(*this, loadw_zp(LoadBBumpPC() + _regs.X)); }
    void IndirectIndexedY(IAddressingMode* am) { am = new MemoryAddressingMode(*this, loadw_zp(LoadBBumpPC()) + _regs.Y); }

    // Memory Acess Helpers
    u8 LoadBBumpPC() { return loadb(_regs.PC++); }
    u16 LoadWBumpPC()
    {
        u16 val = loadw(_regs.PC);
        _regs.PC += 2;
        return val;
    }

    // Flag Helpers
    bool GetFlag(Flag flag) 
    {
        return (_regs.P & (u8)flag) != 0;
    }
    void SetFlag(Flag flag, bool on)
    {
        if (on)
        {
            _regs.P |= (u8)flag;
        }
        else
        {
            _regs.P &= ~((u8)flag);
        }
    }
    u8 SetZN(u8 val) { 
        SetFlag(Flag::Zero, val == 0);
        SetFlag(Flag::Negative, (val & 0x80) != 0);
        return val;
    }

    // Instructions

    // Loads
    void lda(IAddressingMode* am) { _regs.A = SetZN(am->Load()); }
    void ldx(IAddressingMode* am) { _regs.X = SetZN(am->Load()); }
    void ldy(IAddressingMode* am) { _regs.Y = SetZN(am->Load()); }

    // Flag Operations
    void sei(IAddressingMode* am) { SetFlag(Flag::IRQ, true); }
};