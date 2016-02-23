#pragma once

#include "mem.h"

const u32 SCREEN_HEIGHT = 240;
const u32 SCREEN_WIDTH = 256;
const u32 CYCLES_PER_SCANLINE = 114;
const u32 VBLANK_SCANLINE = 241;
const u32 LAST_SCANLINE = 261;

// PPU Regs
struct PpuCtrl
{
    u8 val;

    PpuCtrl() : val(0) { }
    bool VBlankNmi()
    {
        return (val & 0x80) != 0;
    }
};

struct PpuMask
{
    u8 val;

    PpuMask() : val(0) { }
};

struct PpuStatus
{
    u8 val;

    PpuStatus() : val(0) { }

    void SetInVBlank(bool on)
    {
        if (on)
        {
            val |= 0x80;
        }
        else
        {
            val &= ~0x80;
        }
    }
};

struct PpuScroll
{
    u8 val;

    PpuScroll() : val(0) { }
};

struct PpuAddr
{
    enum class WhichByte
    {
        Lo,
        Hi
    };

    u16  val;
    WhichByte next;

    PpuAddr() : val(0), next(WhichByte::Hi) { }
};

struct PpuRegs
{
    PpuCtrl ctrl;
    PpuMask mask;
    PpuStatus status;
    u8 oamAddr;
    PpuScroll scroll;
    PpuAddr addr;
};

struct PpuStepResult
{
    bool VBlankNmi;
    bool NewFrame;

    PpuStepResult()
    {
        Reset();
    }

    void Reset()
    {
        VBlankNmi = false;
        NewFrame = false;
    }
};

class Ppu : public IMem
{
public:
    Ppu();
    ~Ppu();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void Step(u32& cycles, PpuStepResult& result);
private:
    PpuRegs _regs;

    u64 _cycles;
    u16 _scanline;
private:
    void StartVBlank(PpuStepResult& result);

    // Register Operations
    u8 ReadPpuStatus();
    u8 ReadOamData();
    u8 ReadPpuData();

    void WritePpuCtrl(u8 val);
    void WritePpuMask(u8 val);
    void WriteOamAddr(u8 val);
    void WriteOamData(u8 val);
    void WritePpuScroll(u8 val);
    void WritePpuAddr(u8 val);
    void WritePpuData(u8 val);
};