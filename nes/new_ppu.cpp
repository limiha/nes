#include "stdafx.h"
#include "new_ppu.h"
#include "rom.h"

Ppu::Ppu(VRam& vram)
    : _vram(vram)
    , _cycle(0)
    , _scanline(241) // play around with this number
{

}

Ppu::~Ppu()
{

}

// IMem
u8 Ppu::loadb(u16 addr)
{
    switch (addr & 0x7)
    {
    case 0:
        // cannot read $2000
        break;
    case 1:
        // cannot read $20001
        break;
    case 2:
        return Read2002();
        break;
    case 3:
        // cannot read $2003
        break;
    case 4:
        //return ReadOamData();
        break;
    case 5:
        // cannot read $2005
        break;
    case 6:
        // cannot read $2006
        break;
    case 7:
        return Read2007();
        break;
    default:
        // can't happen
        break;
    }
    return 0;
}

void Ppu::storeb(u16 addr, u8 val)
{
    switch (addr & 0x7)
    {
    case 0:
        Write2000(val);
        break;
    case 1:
        Write2001(val);
        break;
    case 2:
        // cannot write $2002
        break;
    case 3:
        Write2003(val);
        break;
    case 4:
        Write2004(val);
        break;
    case 5:
        Write2005(val);
        break;
    case 6:
        Write2006(val);
        break;
    case 7:
        Write2007(val);
        break;
    default:
        // can't happen
        break;
    }
}

// PPUSTATUS
u8 Ppu::Read2002()
{
    u8 regVal = _ppuStatus.val;

    // Reading clears VBlank
    _ppuStatus.SetInVBlank(false);

    // Reset write toggle for $2005/$2006
    _w = false;

    return regVal;
}

// PPUDATA
u8 Ppu::Read2007()
{
    u16 addr = _v;
    u8 val = _vram.loadb(addr);
    _v += _vramAddrIncrement;

    // quirk: If reading from nametables, data is buffered
    if (addr < 0x3f00)
    {
        u8 bufferedData = _ppuDataBuffer;
        _ppuDataBuffer = val;
        return bufferedData;
    }
    else
    {
        // FIXME: Reading from the palettes still updates the buffer in someway
        // FIXME: But not with the palette data

        return val;
    }
}

// PPUCTRL
void Ppu::Write2000(u8 val)
{
    _vramAddrIncrement = (val & (1 << 2)) == 0 ? 1 : 32;
    _spriteBaseAddress = (val & (1 << 3)) == 0 ? 0 : 0x1000;
    _backgroundBaseAddress = (val & (1 << 4)) == 0 ? 0 : 0x1000;
    _spriteSize = (val & (1 << 5)) == 0 ? SpriteSize::Spr8x8 : SpriteSize::Spr8x16;
    _doVBlankNmi = (val & (1 << 7)) != 0;
}

// PPUMASK
void Ppu::Write2001(u8 val)
{
    _clipBackground =   (val & (1 << 1)) == 0;
    _clipSprites =      (val & (1 << 2)) == 0;
    _showBackground =   (val & (1 << 3)) != 0;
    _showSprites =      (val & (1 << 4)) != 0;
}

// OAMADDR
void Ppu::Write2003(u8 val)
{
    _oamAddr = val;
}

// OAMDATA
void Ppu::Write2004(u8 val)
{
    _oam.storeb(_oamAddr, val);
    _oamAddr++;
}

// PPUSCROLL
void Ppu::Write2005(u8 val)
{
    if (!_w)
    {
        // t: ....... ...HGFED = d: HGFED...
        // x:              CBA = d: .....CBA
        // w:                  = 1

        _t &= ~(u16(0b11111));
        _t |= ((u16(val) & u16(0b11111000)) >> 3);
        _x = val & 0b111;

        _w = true;
    }
    else
    {
        // t: CBA..HG FED..... = d: HGFEDCBA
        // w:                  = 0
        _t &= ~(u16(0b111) << 12);
        _t |= ((u16(val) & u16(0b111)) << 12);

        _t &= ~(u16(0b11111) << 5);
        _t |= ((u16(val) & u16(0b11111000)) << 2);

        _w = false;
    }
}

// PPUADDR
void Ppu::Write2006(u8 val)
{
    if (!_w)
    {
        // t: .FEDCBA ........ = d: ..FEDCBA
        // t: X...... ........ = 0
        // w:                  = 1

        _t &= ~(u16(0b111111) << 8);
        _t |= ((u16(val) & 0b111111) << 8);
        _t &= 0x3fff;

        _w = true;
    }
    else
    {
        //t: ....... HGFEDCBA = d: HGFEDCBA
        //v                   = t
        //w:                  = 0
        _t &= 0xff00;
        _t |= u16(val);
        _v = _t;

        _w = false;
    }
}

// PPUDATA
void Ppu::Write2007(u8 val)
{
    _vram.storeb(_v, val);
    _v += _vramAddrIncrement;
}

void Ppu::Step(PpuStepResult& result)
{
    if (_scanline >= 0 && _scanline <= 239)
    {
        // visible scanlines

        if (_cycle == 256)
        {
            DrawScanline();
            if (_scanline == 239)
            {
                result.NewFrame = true;
            }
        }
        else if (_cycle == 257)
        {
            /*
            / if rendering,
            / latch X scroll:
            / HoriVEqualHoriT();
            */
        }
    }
    else if (_scanline >= 240 && _scanline <= 260)
    {
        // post render

        if (_scanline == 241 && _cycle == 1)
        {
            _ppuStatus.SetInVBlank(true);

            if (_doVBlankNmi)
            {
                result.VBlankNmi = true;
            }
        }

        // idle
    }
    else if (_scanline == 261)
    {
        // pre render

        if (_cycle == 1)
        {
            // clear VBlank, Sprite 0, Sprite Overflow
            _ppuStatus.val = 0;
        }
        else if (_cycle >= 280 && _cycle <= 304)
        {
            /*
            / if rendering,
            / latch Y scroll:
            / VertVEqualsVertT();
            */
        }
    }

    // Increment and Reset
    _scanline++;
    if (_scanline == 262)
    {
        _scanline = 0;
    }
    _cycle++;
    if (_cycle == 341)
    {
        _cycle = 0;
    }
}

void Ppu::Step(u8 cycles, PpuStepResult& result)
{
    for (u8 i = 0; i < cycles; i++)
    {
        Step(result);
    }
}

void Ppu::DrawScanline()
{
    SpritePriority spritePriority = SpritePriority::Below;
    rgb pixel;

    u8 backdropColorIndex = _vram.loadw(0x3f00) & 0x3f; // get the universal background color

    // _spriteZeroOnLine = false;
    // _spritesOnLine.clear();

    //if (_regs.mask.ShowSprites())
    //{
    //    CalculateSpritesOnLine((u8)_scanline); // This cast works for now because we don't get called when _scanline > 240
    //}

    for (u32 x = 0; x < SCREEN_WIDTH; x++)
    {
        pixel.Reset();

        u8 backgroundPaletteIndex = 0;
        bool backgroundOpaque = false;
        //if (_regs.mask.ShowBackground() && !((x < 8) && _regs.mask.clipBackground()))
        //{
        //    backgroundOpaque = GetBackgroundColor(x, (u8)_scanline, backgroundPaletteIndex);
        //}

        u8 spritePaletteIndex = 0;
        bool spriteOpqaue = false;
        //if (_spritesOnLine.size() > 0 && !((x < 8) && _regs.mask.clipSprites()))
        //{
        //    spriteOpqaue = GetSpriteColor(x, (u8)_scanline, spritePaletteIndex, backgroundPaletteIndex != 0, spritePriority);
        //}

        if (!backgroundOpaque && !spriteOpqaue)
        {
            pixel.SetColor(backdropColorIndex);
        }
        else if (!spriteOpqaue)
        {
            pixel.SetColor(backgroundPaletteIndex);
        }
        else if (!backgroundOpaque)
        {
            pixel.SetColor(spritePaletteIndex);
        }
        else if (spritePriority == SpritePriority::Above)
        {
            pixel.SetColor(spritePaletteIndex);
        }
        else if (spritePriority == SpritePriority::Below)
        {
            pixel.SetColor(backgroundPaletteIndex);
        }

        PutPixel(x, _scanline, pixel);
    }
}

void Ppu::PutPixel(u8 x, u8 y, rgb& pixel)
{
    Screen[(y * SCREEN_WIDTH + x) * 3 + 0] = pixel.r;
    Screen[(y * SCREEN_WIDTH + x) * 3 + 1] = pixel.g;
    Screen[(y * SCREEN_WIDTH + x) * 3 + 2] = pixel.b;
}

///
/// VRam
///

VRam::VRam(std::shared_ptr<IMapper> mapper)
    : _mapper(mapper)
{
    ZeroMemory(_nametables, sizeof(_nametables));
    ZeroMemory(_pallete, sizeof(_pallete));
}

VRam::~VRam()
{
}

u8 VRam::loadb(u16 addr)
{
    addr &= 0x3fff;

    if (addr < 0x2000)
    {
        return _mapper->chr_loadb(addr);
    }
    else if (addr < 0x3f00)
    {
        if (_mapper->Mirroring == NameTableMirroring::SingleScreenLower)
        {
            return _nametables[addr & 0x3ff];
        }
        else if (_mapper->Mirroring == NameTableMirroring::SingleScreenUpper)
        {
            return _nametables[(addr & 0x3ff) + 0x400];
        }
        else if (_mapper->Mirroring == NameTableMirroring::Horizontal)
        {
            if (addr < 0x2800)
            {
                return _nametables[addr & 0x3ff];
            }
            else
            {
                return _nametables[(addr & 0x3ff) + 0x400];
            }
        }
        else
        {
            return _nametables[addr & 0x7ff];
        }
    }
    else if (addr < 0x4000)
    {
        addr &= 0x1f;
        // if addr is a multiple of 4, return palette entry 0
        u16 palette_addr = (addr % 4 == 0) ? 0 : addr;
        return _pallete[palette_addr] & 0x3f;
    }
    return 0;
}

void VRam::storeb(u16 addr, u8 val)
{
    addr &= 0x3fff;

    if (addr < 0x2000)
    {
        _mapper->chr_storeb(addr, val);
    }
    else if (addr < 0x3f00)
    {
        if (_mapper->Mirroring == NameTableMirroring::SingleScreenLower)
        {
            _nametables[addr & 0x3ff] = val;
        }
        else if (_mapper->Mirroring == NameTableMirroring::SingleScreenUpper)
        {
            _nametables[(addr & 0x3ff) + 0x400] = val;
        }
        else if (_mapper->Mirroring == NameTableMirroring::Horizontal)
        {
            if (addr < 0x2800)
            {
                _nametables[addr & 0x3ff] = val;
            }
            else
            {
                _nametables[(addr & 0x3ff) + 0x400] = val;
            }
        }
        else
        {
            _nametables[addr & 0x7ff] = val;
        }
    }
    else if (addr < 0x4000)
    {
        addr &= 0x1f;
        if (addr == 0x10)
        {
            addr = 0x00;
        }
        _pallete[addr] = val;
    }
}

Oam::Oam()
{
    ZeroMemory(_ram, sizeof(_ram));
}

Oam::~Oam()
{
}

u8 Oam::loadb(u16 addr)
{
    return _ram[(u8)addr];
}

void Oam::storeb(u16 addr, u8 val)
{
    _ram[(u8)addr] = val;
}

const Sprite* Oam::operator[](const int index)
{
    return (Sprite*)&_ram[index * 4];
}
