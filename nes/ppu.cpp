#pragma once

#include "stdafx.h"
#include "ppu.h"
#include "rom.h" // for nametable mirrorin TODO: move

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
        // if addr is a multiple of 4, return palette entry 0
        u16 palette_addr = (addr % 4 == 0) ? 0 : addr;
        return _pallete[palette_addr & 0x1f];
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
        u16 palette_addr = addr & 0x1f;
        if (palette_addr == 0x10)
        {
            palette_addr = 0x00;
        }
        _pallete[palette_addr] = val;
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


Ppu::Ppu(VRam& vram)
    : _vram(vram)
    , _cycles(0)
    , _scanline(VBLANK_SCANLINE)
    , _ppuDatatBuffer(0)
    , _spritesOnLine(8)
    , _scrollX(0)
    , _scrollY(0)
    , _v(0)
    , _t(0)
    , _x(0)
    , _w(false)
{
    ZeroMemory(Screen, sizeof(Screen));
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
        return ReadPpuStatus();
        break;
    case 3:
        // cannot read $2003
        break;
    case 4:
        return ReadOamData();
        break;
    case 5:
        // cannot read $2005
        break;
    case 6:
        // cannot read $2006
        break;
    case 7:
        return ReadPpuData();
        break;
    default:
        // can't happen
        break;
    }
    return 0;
}

void Ppu::storeb(u16 addr, u8 val)
{
    switch(addr & 0x7)
    {
    case 0:
        WritePpuCtrl(val);
        break;
    case 1:
        WritePpuMask(val);
        break;
    case 2:
        // cannot write $2002
        break;
    case 3:
        WriteOamAddr(val);
        break;
    case 4:
        WriteOamData(val);
        break;
    case 5:
        WritePpuScroll(val);
        break;
    case 6:
        WritePpuAddr(val);
        break;
    case 7:
        WritePpuData(val);
        break;
    default:
        // can't happen
        break;
    }
}

// Register Control

u8 Ppu::ReadPpuStatus() {
    u8 regVal = _regs.status.val;

    // Docs say that reading this resets VBlank status;
    _regs.status.SetInVBlank(false);
    
    // TODO: scrolling reset
    //_regs.addr.next = PpuAddr::WhichByte::Hi;
    //_regs.scroll.NextDirection = PpuScroll::Direction::X;

    // reset write toggle for $2005 and $2006
    _w = false;

    return regVal;
}

u8 Ppu::ReadOamData()
{
    // TODO
    // Reading from here is super weird, don't need to implement initially
    return 0;
}

u8 Ppu::ReadPpuData()
{
    //u16 addr = _regs.addr.val;
    //u8 val = _vram.loadb(addr);
    //_regs.addr.val += _regs.ctrl.VRamAddrIncrement();

    u16 addr = _v;
    u8 val = _vram.loadb(addr);
    _v += _regs.ctrl.VRamAddrIncrement();

    u8 bufferedData = _ppuDatatBuffer;

    if (addr < 0x3f00)
    {
        _ppuDatatBuffer = val;
        return bufferedData;
    }
    else
    {
        // FIXME: Reading from the palettes still updates the buffer in someway
        // FIXME: But not with the palette data

        return val;
    }

    return 0;
}

void Ppu::WritePpuCtrl(u8 val)
{
    // TODO: side effects of writing

    _regs.ctrl.val = val;

    //_scrollX = (_scrollX & 0xff) | _regs.ctrl.ScrollXOffset();
    //_scrollY = (_scrollY & 0xff) | _regs.ctrl.ScrollYOffset();

    _t &= ~(u16(0b11) << 10);
    _t |= ((u16(val) & u16(0b11)) << 10);
}

void Ppu::WritePpuMask(u8 val)
{
    _regs.mask.val = val;
}

void Ppu::WriteOamAddr(u8 val)
{
    _regs.oamAddr = val;
}

void Ppu::WriteOamData(u8 val)
{
    _oam.storeb(_regs.oamAddr++, val);
}

void Ppu::WritePpuScroll(u8 val)
{
    //if (_regs.scroll.NextDirection == PpuScroll::Direction::X)
    //{
    //    _scrollX = val;

    //    _regs.scroll.X = val;
    //    _regs.scroll.NextDirection = PpuScroll::Direction::Y;
    //}
    //else if (_regs.scroll.NextDirection == PpuScroll::Direction::Y)
    //{
    //    _scrollY = val;

    //    _regs.scroll.Y = val;
    //    _regs.scroll.NextDirection = PpuScroll::Direction::X;
    //}

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

void Ppu::WritePpuAddr(u8 val)
{
    //if (_regs.addr.next == PpuAddr::WhichByte::Hi)
    //{
    //    _regs.addr.val &= 0x00ff;
    //    _regs.addr.val |= (((u16)val) << 8);
    //    _regs.addr.next = PpuAddr::WhichByte::Lo;
    //}
    //else
    //{
    //    _regs.addr.val &= 0xff00;
    //    _regs.addr.val |= (u16)val;
    //    _regs.addr.next = PpuAddr::WhichByte::Hi;

    //    // TODO: Something about resetting scrolling here?
    //}

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

void Ppu::WritePpuData(u8 val)
{
    //_vram.storeb(_regs.addr.val, val);
    //_regs.addr.val += _regs.ctrl.VRamAddrIncrement();

    // printf("addr: %04X\tdata: %02X\n", _v, val);

    _vram.storeb(_v, val);
    _v += _regs.ctrl.VRamAddrIncrement();
}

// Rendering

void Ppu::PutPixel(u8 x, u8 y, rgb& pixel)
{
    Screen[(y * SCREEN_WIDTH + x) * 3 + 0] = pixel.r;
    Screen[(y * SCREEN_WIDTH + x) * 3 + 1] = pixel.g;
    Screen[(y * SCREEN_WIDTH + x) * 3 + 2] = pixel.b;
}

bool Ppu::GetBackgroundColor(u8 x_in, u8 y_in, u8& paletteIndex)
{

    // attempt a static scroll of 4 pixels
    u16 x = (u16)x_in + ScrollX();
    //u16 x = x_in + (256 + 255);
    //u16 x = x_in + (256 - 5);
    u16 y = (u16)y_in + ScrollY();
    //u16 x = (u16)x_in + (256 + 128);
    //u16 y = (u16)y_in + (240 - 240);


    // The Name Tables store tile numbers
    // These Tile Numbers are indices into the pattern tables
    // The first step is to figure out the correct address to read from the name tables

    bool toggleXNameTable = false;
    bool toggleYNameTable = false;

    if (x >= 256)
    {
        toggleXNameTable = true;
    }
    x %= 256;

    if (y >= 240)
    {
        toggleYNameTable = true;;
    }
    y %= 240;

    u8 nameTableBits = (u8)((_t & 0b110000000000) >> 10);

    if (toggleXNameTable)
    {
        nameTableBits ^= 0x1;
    }
    if (toggleYNameTable)
    {
        nameTableBits ^= 0x2;
    }

    u16 nameTableBaseAddress;
    switch (nameTableBits)
    {
    case 0: nameTableBaseAddress = 0x2000; break;
    case 1: nameTableBaseAddress = 0x2400; break;
    case 2: nameTableBaseAddress = 0x2800; break;
    case 3: nameTableBaseAddress = 0x2c00; break;
    }

    // A Name Table represents a 32 * 30 grid of tiles
    // A tile is 8x8, so to the tile index divide by 8
    u8 nameTableIndexX = x / 8;
    u8 nameTableIndexY = y / 8;

    u16 nameTableOffset = (nameTableIndexY * 32) + nameTableIndexX;
    u16 nameTableAddress = nameTableBaseAddress + nameTableOffset;

    // Get 
    u8 patternTableIndex = _vram.loadb(nameTableAddress);

    // Now we have the index of the tile we are drawing.
    // A tile is a 16 byte (128 bit) structure stored in the pattern tables
    // It consists of two 8 byte 'planes'
    // We need the correct bit from both planes
    // The bit in the second plane will be 8 bytes after the first

    // First we need to know if the background tiles are held in 
    // the left Pattern Table or the right Pattern Table
    u16 patternTableBaseAddress = _regs.ctrl.BackgroundBaseAddress();

    // Next we add to this the index * 16 since each tile is 16 bytes
    u16 patternTableBaseOffset = patternTableIndex * 16;

    // Now we need the row offset into the pattern itself
    u16 patternRowOffset = y % 8;

    // Now we have the address of the row we need from the first plane
    u16 patternRowAddress = patternTableBaseAddress + patternTableBaseOffset + patternRowOffset;

    // Load the byte representing this row from both planes
    u8 loPlaneRow = _vram.loadb(patternRowAddress);
    u8 hiPlaneRow = _vram.loadb(patternRowAddress + 8);

    u32 patternBitIndex = x % 8;

    // We need to index into these bytes to get the specific bit for this pixel
    u8 loPlaneBit = (loPlaneRow >> (7 - patternBitIndex)) & 0x1;
    u8 hiPlaneBit = (hiPlaneRow >> (7 - patternBitIndex)) & 0x1;

    u8 patternColor = (hiPlaneBit << 1) | loPlaneBit;

    if (patternColor == 0)
    {
        return false;
    }

    u16 attributeTableBaseAddress = nameTableBaseAddress + 0x3c0;

    u16 attributeTableIndexX = nameTableIndexX / 4;
    u16 attributeTableIndexY = nameTableIndexY / 4;
    u16 attributeTableOffset = (attributeTableIndexY * 8) + attributeTableIndexX;
    u16 attributeTableAddress = attributeTableBaseAddress + attributeTableOffset;

    u8 attributeByte = _vram.loadb(attributeTableAddress);

    u8 attributeByteIndexX = (nameTableIndexX % 4) / 2;
    u8 attributeByteIndexY = (nameTableIndexY % 4) / 2;
    u8 attributeByteIndex = (attributeByteIndexY * 2) + attributeByteIndexX;

    u8 attributeColor = (attributeByte >> (attributeByteIndex * 2)) & 0x3;

    u8 tileColor = (attributeColor << 2) | patternColor;

    paletteIndex = _vram.loadb(0x3f00 + (u16)tileColor) & 0x3f;

    return true;
}

void Ppu::CalculateSpritesOnLine(u16 y)
{
    u8 numSpritesOnLine = 0;
    
    u16 spriteHeight = _regs.ctrl.SpriteSize() == SpriteSize::Spr8x8 ? 8 : 16;

    for (int i = 0; i < 64; i++)
    {
        const Sprite* pSprite = _oam[i];

        if ((u16)(pSprite->Y + 1) <= y && (u16)(pSprite->Y + 1 + spriteHeight) > y)
        {
            if (numSpritesOnLine < 8)
            {
                if (i == 0)
                {
                    _spriteZeroOnLine = true;
                }
                _spritesOnLine.push_back(std::make_unique<Sprite>(*pSprite));
                numSpritesOnLine++;
            }
            else if (numSpritesOnLine == 8)
            {
                _regs.status.SetSpriteOverflow(true);
            }
        }
    }
}

bool Ppu::GetSpriteColor(u8 x, u8 y, u8& paletteIndex, bool backgroundOpaque, SpritePriority& priority)
{
    std::vector<std::unique_ptr<Sprite>>::iterator it = _spritesOnLine.begin();

    u16 spriteHeight = _regs.ctrl.SpriteSize() == SpriteSize::Spr8x8 ? 8 : 16;
    bool is8x16 = _regs.ctrl.SpriteSize() == SpriteSize::Spr8x16;

    // which table are sprites in?
    u16 patternTableBaseAddress = _regs.ctrl.SpriteBaseAddress();

    for (; it != _spritesOnLine.end(); it++)
    {
        Sprite* spr = it->get();

        if (spr->X <= x && spr->X + 8 > x)
        {
            // our pixel is within this sprite

            u16 patternTableBaseOffset;
            u16 patternRowOffset = y - (spr->Y + 1);
            if (is8x16)
            {
                patternTableBaseAddress = (spr->TileIndex & 1) == 0 ? 0 : 0x1000;

                u16 topSpriteOffset = (spr->TileIndex & 0xFE) * 16;
                u16 bottomSpriteOffset = topSpriteOffset + 16;

                if (spr->FlipVertical())
                {
                    if (patternRowOffset < 8)
                    {
                        patternTableBaseOffset = bottomSpriteOffset;
                    }
                    else
                    {
                        patternTableBaseOffset = topSpriteOffset;
                        patternRowOffset -= 8;
                    }
                }
                else
                {
                    if (patternRowOffset < 8)
                    {
                        patternTableBaseOffset = topSpriteOffset;
                    }
                    else
                    {
                        patternTableBaseOffset = bottomSpriteOffset;
                        patternRowOffset -= 8;
                    }
                }
            }
            else
            {
                patternTableBaseOffset = spr->TileIndex * 16;
            }

            if (spr->FlipVertical())
            {
                patternRowOffset = 7 - patternRowOffset;
            }

            u16 patternRowAddress = patternTableBaseAddress + patternTableBaseOffset + patternRowOffset;

            u8 loPlaneRow = _vram.loadb(patternRowAddress);
            u8 hiPlaneRow = _vram.loadb(patternRowAddress + 8);

            u32 patternBitIndex = x - spr->X;
            if (spr->FlipHorizontal())
            {
                patternBitIndex = 7 - patternBitIndex;
            }

            u8 loPlaneBit = (loPlaneRow >> (7 - patternBitIndex)) & 0x01;
            u8 hiPlaneBit = (hiPlaneRow >> (7 - patternBitIndex)) & 0x01;

            u8 patternColor = (hiPlaneBit << 1) | loPlaneBit;

            if (patternColor == 0)
            {
                if (it == _spritesOnLine.end())
                {
                    return false;
                }
                // This sprite pixel is transparent, continue searching for lower pri sprites
                continue;
            }

            // Now we know we have an opaque sprite pixel
            if (backgroundOpaque && _spriteZeroOnLine && it == _spritesOnLine.begin())
            {
                _regs.status.SetSpriteZeroHit(true);
            }

            priority = spr->Prioirty();

            u8 tileColor = (spr->Palette() << 2) | patternColor;

            paletteIndex = _vram.loadb(0x3f10 + (u16)tileColor) & 0x3f;
            return true;
        }
    }

    return 0;
}

void Ppu::IncHoriV()
{
    if ((_v & 0x1f) == 31)
    {
        _v &= ~(u16(0b11111)); // coarse x = 0
        _v ^= 0x0400; // toggle horizontal name table
    }
    else
    {
        _v++;
    }
}

void Ppu::IncVertV()
{
    if ((_v & 0x7000) != 0x7000)
    {
        _v += 0x1000;
    }
    else
    {
        _v &= ~0x7000;
        u16 y = ((_v & 0x3e0) >> 5);
        if (y == 29)
        {
            _v ^= 0x0800;
        }
        else if (y == 31)
        {
            y += 1;
        }

        _v = (_v & ~0x3e0) | (y << 5);
    }
}

void Ppu::HoriVEqualsHoriT()
{
    _v &= ~(0b10000011111);
    _v |= (_t & 0b10000011111);

}

u8 Ppu::ScrollX()
{
    u8 x = ((_t & 0b11111) << 3) | _x;
    //u16 offset = (_t & (1 << 10)) == 0 ? 0 : 256;
    return x;// +offset;
}

u8 Ppu::ScrollY()
{
    u8 y = ((_t & 0b1111100000) >> 2) | ((_t & 0b111000000000000) >> 12);
    //u16 offset = (_t & (1 << 11)) == 0 ? 0 : 240;
    return y;// +offset;
}

void Ppu::RenderScanline()
{
    SpritePriority spritePriority = SpritePriority::Below;
    rgb pixel;

    u8 backdropColorIndex = _vram.loadw(0x3f00) & 0x3f; // get the universal background color

    _spriteZeroOnLine = false;
    _spritesOnLine.clear();

    if (_regs.mask.ShowSprites())
    {
        CalculateSpritesOnLine((u8)_scanline); // This cast works for now because we don't get called when _scanline > 240
    }

    for (u32 x = 0; x < SCREEN_WIDTH; x++)
    {
        pixel.Reset();

        u8 backgroundPaletteIndex = 0;
        bool backgroundOpaque = false;
        if (_regs.mask.ShowBackground() && !((x < 8) && _regs.mask.clipBackground()))
        {
            backgroundOpaque = GetBackgroundColor(x, (u8)_scanline, backgroundPaletteIndex);
        }

        u8 spritePaletteIndex = 0;
        bool spriteOpqaue = false;
        if (_spritesOnLine.size() > 0 && !((x < 8) && _regs.mask.clipSprites()))
        {
            spriteOpqaue = GetSpriteColor(x, (u8)_scanline, spritePaletteIndex, backgroundPaletteIndex != 0, spritePriority);
        }

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

        PutPixel(x, (u8)_scanline, pixel);
    }
    //HoriVEqualsHoriT();
}

void Ppu::StartVBlank(PpuStepResult& result)
{
    _regs.status.SetInVBlank(true);

    if (_regs.ctrl.VBlankNmi())
    {
        result.VBlankNmi = true;
    }
}

void Ppu::Step(u32& cycles, PpuStepResult& result)
{
    while (cycles >= CYCLES_PER_SCANLINE)
    {
        cycles -= CYCLES_PER_SCANLINE;

        if (_scanline < SCREEN_HEIGHT)
        {
            RenderScanline();
        }
        _scanline++;

        if (_scanline == VBLANK_SCANLINE)
        {
            StartVBlank(result);
        }

        if (_scanline == LAST_SCANLINE)
        {
            result.NewFrame = true;
            _scanline = 0;
            _regs.status.SetInVBlank(false);
            _regs.status.SetSpriteOverflow(false);
            _regs.status.SetSpriteZeroHit(false);
        }
    }
}