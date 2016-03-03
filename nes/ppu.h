#pragma once

#include "mem.h"
#include "mapper.h"

const u32 SCREEN_HEIGHT = 240;
const u32 SCREEN_WIDTH = 256;
const u32 CYCLES_PER_SCANLINE = 114;
const u32 VBLANK_SCANLINE = 241;
const u32 LAST_SCANLINE = 261;

// palette taken from http://nesdev.com/NESTechFAQ.htm#accuratepal
static const u8 PALETTE[] = {
    0x80, 0x80, 0x80, 0x00, 0x3D, 0xA6, 0x00, 0x12, 0xB0, 0x44, 0x00, 0x96,
    0xA1, 0x00, 0x5E, 0xC7, 0x00, 0x28, 0xBA, 0x06, 0x00, 0x8C, 0x17, 0x00,
    0x5C, 0x2F, 0x00, 0x10, 0x45, 0x00, 0x05, 0x4A, 0x00, 0x00, 0x47, 0x2E,
    0x00, 0x41, 0x66, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0xC7, 0xC7, 0xC7, 0x00, 0x77, 0xFF, 0x21, 0x55, 0xFF, 0x82, 0x37, 0xFA,
    0xEB, 0x2F, 0xB5, 0xFF, 0x29, 0x50, 0xFF, 0x22, 0x00, 0xD6, 0x32, 0x00,
    0xC4, 0x62, 0x00, 0x35, 0x80, 0x00, 0x05, 0x8F, 0x00, 0x00, 0x8A, 0x55,
    0x00, 0x99, 0xCC, 0x21, 0x21, 0x21, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0xFF, 0xFF, 0xFF, 0x0F, 0xD7, 0xFF, 0x69, 0xA2, 0xFF, 0xD4, 0x80, 0xFF,
    0xFF, 0x45, 0xF3, 0xFF, 0x61, 0x8B, 0xFF, 0x88, 0x33, 0xFF, 0x9C, 0x12,
    0xFA, 0xBC, 0x20, 0x9F, 0xE3, 0x0E, 0x2B, 0xF0, 0x35, 0x0C, 0xF0, 0xA4,
    0x05, 0xFB, 0xFF, 0x5E, 0x5E, 0x5E, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0xFF, 0xFF, 0xFF, 0xA6, 0xFC, 0xFF, 0xB3, 0xEC, 0xFF, 0xDA, 0xAB, 0xEB,
    0xFF, 0xA8, 0xF9, 0xFF, 0xAB, 0xB3, 0xFF, 0xD2, 0xB0, 0xFF, 0xEF, 0xA6,
    0xFF, 0xF7, 0x9C, 0xD7, 0xE8, 0x95, 0xA6, 0xED, 0xAF, 0xA2, 0xF2, 0xDA,
    0x99, 0xFF, 0xFC, 0xDD, 0xDD, 0xDD, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};


struct rgb 
{
    u8 r;
    u8 g;
    u8 b;

    void Reset()
    {
        r = 0x00;
        g = 0x00;
        b = 0x00;
    }

    void SetColor(u8 palette_index)
    {
        palette_index &= 0x3f;
        r = PALETTE[(palette_index * 3) + 0];
        g = PALETTE[(palette_index * 3) + 1];
        b = PALETTE[(palette_index * 3) + 2];
    };

    rgb()
    {
        Reset();
    }
};

class VRam : public IMem
{
public:
    VRam(std::shared_ptr<IMapper>);
    ~VRam();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
private:
    std::shared_ptr<IMapper> _mapper;

    // FIXME: This is enough VRAM for two name tables
    // FIXME: Which is not correct for all mapper scenarios
    u8 _nametables[0x800];

    u8 _pallete[0x20];
};

enum class SpritePriority
{
    Above,
    Below
};

#pragma pack()
struct Sprite
{
    u8 Y; // Y coordinate (minus 1) of this sprite
    u8 TileIndex; // The tile index of this sprite in the pattern tables
    u8 Attributes; // Sprite Attributes, 
    u8 X; // X coordinate of this sprite;

    u8 Palette() { return Attributes & 0x03; }
    SpritePriority Prioirty() { return (Attributes & (1 << 5)) == 0 ? SpritePriority::Above : SpritePriority::Below; }
    bool FlipHorizontal() { return (Attributes & (1 << 6)) != 0; }
    bool FlipVertical() { return (Attributes & (1 << 7)) != 0; }
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

    const Sprite* operator[](const int index);

private:
    u8 _ram[0x100];
};

// PPU Regs
struct PpuCtrl
{
    u8 val;

    PpuCtrl() : val(0) { }

    u16 ScrollXOffset() { return (val & (1 << 0)) == 0 ? 0 : 256; }
    u16 ScrollYOffset() { return (val & (1 << 1)) == 0 ? 0 : 240; }


    u16 VRamAddrIncrement()         { return (val & (1 << 2)) == 0 ? 1 : 32; }
    u16 SpriteBaseAddress()         { return (val & (1 << 3)) == 0 ? 0 : 0x1000; }
    u16 BackgroundBaseAddress()     { return (val & (1 << 4)) == 0 ? 0 : 0x1000; }
    bool VBlankNmi()                { return (val & (1 << 7)) != 0; }
};

struct PpuMask
{
    u8 val;

    PpuMask() : val(0) { }

    bool clipBackground()   { return (val & (1 << 1)) == 0; }
    bool clipSprites()      { return (val & (1 << 2)) == 0; }
    bool ShowBackground()   { return (val & (1 << 3)) != 0; }
    bool ShowSprites()      { return (val & (1 << 4)) != 0; }
};

struct PpuStatus
{
    u8 val;

    PpuStatus() : val(0) { }

    void SetInVBlank(bool on)
    {
        if (on)
        {
            val |= (1 << 7);
        }
        else
        {
            val &= ~(1 << 7);
        }
    }

    void SetSpriteOverflow(bool on)
    {
        if (on)
        {
            val |= (1 << 5);
        }
        else
        {
            val &= ~(1 << 5);
        }
    }

    void SetSpriteZeroHit(bool on)
    {
        if (on)
        {
            val |= (1 << 6);
        }
        else
        {
            val &= ~(1 << 6);
        }
    }
};

struct PpuScroll
{
    enum class Direction
    {
        X, 
        Y
    };

    u8 X;
    u8 Y;
    Direction NextDirection;

    PpuScroll() : X(0), Y(0), NextDirection(Direction::X) { }
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

public:
    u8 Screen[SCREEN_HEIGHT * SCREEN_WIDTH * 3];
private:
    PpuRegs _regs;

    VRam& _vram;
    Oam _oam;

    u8 _ppuDatatBuffer;

    u64 _cycles;
    i32 _scanline;

    bool _spriteZeroOnLine;
    std::vector<std::unique_ptr<Sprite>> _spritesOnLine;

    u16 _scrollX;
    u16 _scrollY;

    // background registers
    u16 _v;
    u16 _t;
    u8 _x;
    bool _w;

    void IncHoriV();
    void IncVertV();
    void HoriVEqualsHoriT();

    u8 ScrollX();
    u8 ScrollY();

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

    // Rendering Operations

    void PutPixel(u8 x, u8 y, rgb& pixel);
    void CalculateSpritesOnLine(u8 y);
    
    // Returns the Palette Index of the Background pixel at (x,y)
    u8 GetBackgroundColor(u8 x_in, u8 y_in);

    // Returns the palette index of the sprite pixel at (x,y)
    u8 GetSpriteColor(u8 x, u8 y, bool backgroundOpaque, SpritePriority& priority);

    void RenderScanline();
};