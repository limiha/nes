#pragma once

#include "mem.h"
#include "rom.h"

const u32 SCREEN_HEIGHT = 240;
const u32 SCREEN_WIDTH = 256;
const u32 CYCLES_PER_SCANLINE = 114;
const u32 VBLANK_SCANLINE = 241;
const u32 LAST_SCANLINE = 261;

class VRam : public IMem
{
public:
    VRam(Rom& rom);
    ~VRam();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
private:
    Rom& _rom;

    // FIXME: This is enough VRAM for two name tables
    // FIXME: Which is not correct for all mapper scenarios
    u8 _nametables[0x800];

    u8 _pallete[0x20];
};

// Object Access Memory
// This is Sprite Ram, which confused me forever
class Oam : public IMem
{
public:
    Oam();
    ~Oam();

    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
private:
    u8 _ram[0x100];
};

// PPU Regs
struct PpuCtrl
{
    u8 val;

    PpuCtrl() : val(0) { }

    u16 VRamAddrIncrement() { return (val & (1 << 3)) == 0 ? 1 : 32; }
    bool VBlankNmi() { return (val & (1 << 7)) != 0; }
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
    Ppu(VRam& vram);
    ~Ppu();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void Step(u32& cycles, PpuStepResult& result);
private:
    PpuRegs _regs;
    VRam& _vram;
    Oam _oam;

    u8 _ppuDatatBuffer;

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