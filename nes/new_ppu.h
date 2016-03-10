#pragma once

#include "mem.h"
#include "mapper.h"

class Gfx;

const u32 SCREEN_HEIGHT = 240;
const u32 SCREEN_WIDTH = 256;
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

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

private:
    std::shared_ptr<IMapper> _mapper;

    // FIXME: This is enough VRAM for two name tables
    // FIXME: Which is not correct for all mapper scenarios
    u8 _nametables[0x800];

    u8 _palette[0x20];
};

enum class SpritePriority : u8
{
    Above = 0,
    Below = 1
};

enum class SpriteSize : u8
{
    Spr8x8 = 0,
    Spr8x16 = 1
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

    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

    const Sprite* operator[](const int index);

private:
    u8 _ram[0x100];
};

struct PpuStepResult
{
    bool VBlankNmi;
    bool WantIrq;

    PpuStepResult()
    {
        Reset();
    }

    void Reset()
    {
        VBlankNmi = false;
        WantIrq = false;
    }
};

// Ppu Registers

struct PpuStatus
{
    u8 val;

    PpuStatus() : val(0) { }

    void SetInVBlank(bool on)
    {
        if (on) val |= (1 << 7);
        else val &= ~(1 << 7);
    }

    void SetSpriteOverflow(bool on)
    {
        if (on) val |= (1 << 5);
        else val &= ~(1 << 5);
    }

    void SetSpriteZeroHit(bool on)
    {
        if (on) val |= (1 << 6);
        else val &= ~(1 << 6);
    }
};

class Ppu : public IMem
{
public:
    Ppu(std::shared_ptr<IMapper> mapper, Gfx& gfx);
    ~Ppu();

public:
    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

public:
    void Step(u8 cycles, PpuStepResult& result);

#if defined(RENDER_NAMETABLE)
    void RenderNameTable(u8 screen[], int i);
#endif
#if defined(RENDER_PATTERNTABLE)
    void RenderPatternTable(u16 baseAddr, u8 pt[]);
#endif

private:
    void DrawFrame();
    void Step(PpuStepResult& resutl);

    u8 Read2002();
    u8 Read2007();

    void Write2000(u8 val);
    void Write2001(u8 val);
    void Write2003(u8 val);
    void Write2004(u8 val);
    void Write2005(u8 val);
    void Write2006(u8 val);
    void Write2007(u8 val);

    // Scrolling
    bool IsRendering() { return _showBackground || _showSprites; }
    void HoriVEqualsHoriT();
    void VertVEqualsVertT();
    void IncVertV();
    u8 ScrollX();
    u8 ScrollY();

    void DrawScanline(u8 x);
    bool GetBackgroundColor(u8& paletteIndex);
    bool GetSpriteColor(u8 x, u8 y, bool backgroundOpaque, u8& paletteIndex, SpritePriority& priority);
    void ProcessSprites();
    void PutPixel(u16 x, u16 y, rgb& pixel);

private:
    Gfx& _gfx;
    std::shared_ptr<IMapper> _mapper;
    VRam _vram;

    u8 _screen[256 * 240 * 3];

    // Sprites
    Oam _oam;
    u16 _oamAddr;
    std::vector<std::unique_ptr<Sprite>> _lineSprites;
    bool _spriteZeroOnLine;

    // Ppu Register Data
    PpuStatus _ppuStatus;
    u8 _ppuDataBuffer;

    // PPUCTRL
    u16 _vramAddrIncrement;
    u16 _spriteBaseAddress;
    u16 _backgroundBaseAddress;
    SpriteSize _spriteSize;
    bool _doVBlankNmi;

    // PPUMASK
    bool _clipBackground;
    bool _clipSprites;
    bool _showBackground;
    bool _showSprites;

    // VRam Registers Registers PPUSCROLL and PPUADDR
    // http://wiki.nesdev.com/w/index.php/PPU_scrolling
    u16 _v; // VRam Address
    u16 _t; // Temporary VRam Address
    u8 _x; // Fine X Scroll
    bool _w; // Write toggle

    // the current cycle number, 341 cycles in a scan line, 0 - 340
    // TODO: figure out the extra _cycle/odd frame thing.
    u16 _cycle;

    // the current scane line number, 
    // 256 pixels wide, 
    // 240 per screen, 
    // 242 per pframe, 
    // 0 - 239: visible, 
    // 240: post render, 
    // 241: - 260 idle, 
    // 261: pre render
    // I will render the scanline on _cycle 256
    // The current scanline number means that vram write occue "while the scanline is being drawn"
    // This means x scrolls before cycle 256 will take effect on the next scanline (hori(v) = hori(t) at 256 of visible scanlines)
    // Y scrolls effect the next full frame and are not latched in until near the end of _scanline 261
    u16 _scanline;

    // Toggled on every frame
    // If true, the last cycle of the pre render scanline is skipped
    bool _frameOdd;
};