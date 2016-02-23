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
};

struct PpuRegs
{
    PpuCtrl ppuCtrl;
    PpuMask ppuMask;
    PpuStatus ppuStatus;
};

class Ppu : public IMem
{
public:
    Ppu();
    ~Ppu();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void Step(u32& cycles);
private:
    PpuRegs _regs;

    u64 _cycles;
    u16 _scanline;
};