#include "stdafx.h"
#include "new_ppu.h"
#include "rom.h"

Ppu::Ppu(VRam& vram)
    : _vram(vram)
    , _cycle(0)
    , _scanline(241)
    , _frameOdd(false)
    , _v(0)
    , _t(0)
    , _x(0)
    , _w(false)
    , _ppuDataBuffer(0)
    , _vramAddrIncrement(1)
    , _spriteBaseAddress(0)
    , _backgroundBaseAddress(0)
    , _spriteSize(SpriteSize::Spr8x8)
    , _doVBlankNmi(false)
    , _clipBackground(false)
    , _clipSprites(false)
    , _showBackground(false)
    , _showSprites(false)
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
    _t &= ~(0b11 << 10);
    _t |= ((0b11 & val) << 10);
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

// Scrolling Functions
void Ppu::HoriVEqualsHoriT()
{
    // copy scroll X bits from v to t
    _v &= ~(0b10000011111);
    _v |= (_t & 0b10000011111);
}

void Ppu::VertVEqualsVertT()
{
    // copy scroll Y bits from v to t
    _v &= ~(0b111101111100000);
    _v |= (_t & 0b111101111100000);
}

u8 Ppu::ScrollX()
{
    u8 x = ((_v & 0b11111) << 3) | _x;
    //u16 offset = (_t & (1 << 10)) == 0 ? 0 : 256;
    return x;// +offset;
}

u8 Ppu::ScrollY()
{
    u8 y = ((_v & 0b1111100000) >> 2) | ((_v & 0b111000000000000) >> 12);
    //u16 offset = (_t & (1 << 11)) == 0 ? 0 : 240;
    return y;// +offset;
}

void Ppu::Step(PpuStepResult& result)
{
    if (_scanline >= 0 && _scanline <= 239)
    {
        // visible scanlines
        
        if (_cycle == 0)
        {
            _lineSprites.clear();
            _spriteZeroOnLine = false;
            if (_showSprites)
            {
                ProcessSprites();
            }
        }
        if (_cycle >=1 && _cycle <= 256)
        {
            DrawScanline(_cycle - 1);
            if (_scanline == 239 && _cycle == 256)
            {
                result.NewFrame = true;
            }
        }
        else if (_cycle == 257)
        {
            if (IsRendering())
            {
                // latch X Scroll
                HoriVEqualsHoriT();
            }
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
        else if (_cycle == 257)
        {
            if (IsRendering())
            {
                HoriVEqualsHoriT();
            }
        }
        else if (_cycle >= 280 && _cycle <= 304)
        {
            if (IsRendering())
            {
                // latch Y scroll
                VertVEqualsVertT();
            }
        }
        else if (_cycle == 339 && _frameOdd)
        {
            _cycle++;
        }
    }

    // Increment and Reset
    _cycle++;
    if (_cycle == 341)
    {
        _cycle = 0;
        _scanline++;
        if (_scanline == 262)
        {
            _scanline = 0;
            _frameOdd ^= true;
        }
    }
}

void Ppu::Step(u8 cycles, PpuStepResult& result)
{
    for (u8 i = 0; i < cycles; i++)
    {
        Step(result);
    }
}

void Ppu::DrawScanline(u8 x)
{
    SpritePriority spritePriority = SpritePriority::Below;
    rgb pixel;

    u8 backdropColorIndex = _vram.loadw(0x3f00) & 0x3f; // get the universal background color

    pixel.Reset();

    u8 backgroundPaletteIndex = 0;
    bool backgroundOpaque = false;
    if (_showBackground && !((x < 8) && _clipBackground))
    {
        backgroundOpaque = GetBackgroundColor(backgroundPaletteIndex);
    }

    u8 spritePaletteIndex = 0;
    bool spriteOpqaue = false;
    if (_lineSprites.size() > 0 && !((x < 8) && _clipSprites))
    {
        spriteOpqaue = GetSpriteColor(x, (u8)_scanline, backgroundPaletteIndex != 0, spritePaletteIndex, spritePriority);
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

    PutPixel(x, _scanline, pixel);
}

void Ppu::ProcessSprites()
{
    u8 numSpritesOnLine = 0;

    u16 spriteHeight = _spriteSize == SpriteSize::Spr8x8 ? 8 : 16;

    for (int i = 0; i < 64; i++)
    {
        const Sprite* pSprite = _oam[i];

        if ((u16)(pSprite->Y + 1) <= _scanline && (u16)(pSprite->Y + 1 + spriteHeight) > _scanline)
        {
            if (numSpritesOnLine < 8)
            {
                if (i == 0)
                {
                    _spriteZeroOnLine = true;
                }
                _lineSprites.push_back(std::make_unique<Sprite>(*pSprite));
                numSpritesOnLine++;
            }
            else if (numSpritesOnLine == 8)
            {
                _ppuStatus.SetSpriteOverflow(true);
            }
        }
    }
}

bool Ppu::GetBackgroundColor(u8& paletteIndex)
{
    u16 x = _cycle - 1 + ScrollX();
    u16 y = _scanline + ScrollY();

    // wrap values around and toggle name table
    if (x == 256)
    {
        _v ^= (1 << 10);
        x = 0;
    }
    if (y == 240)
    {
        _v ^= (1 << 11);
        y = 0;
    }

    // The Name Tables store tile numbers
    // These Tile Numbers are indices into the pattern tables
    // The first step is to figure out the correct address to read from the name tables

    u8 nameTableBits = (u8)((_v & 0b110000000000) >> 10);

    u16 nameTableBaseAddress;
    switch (nameTableBits)
    {
    case 0: nameTableBaseAddress = 0x2000; break;
    case 1: nameTableBaseAddress = 0x2400; break;
    case 2: nameTableBaseAddress = 0x2800; break;
    case 3: nameTableBaseAddress = 0x2c00; break;
    }

    x %= 256;
    y %= 240;

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
    u16 patternTableBaseAddress = _backgroundBaseAddress;

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

bool Ppu::GetSpriteColor(u8 x, u8 y, bool backgroundOpaque, u8& paletteIndex, SpritePriority& priority)
{
    std::vector<std::unique_ptr<Sprite>>::iterator it = _lineSprites.begin();

    u16 spriteHeight = _spriteSize == SpriteSize::Spr8x8 ? 8 : 16;
    bool is8x16 = _spriteSize == SpriteSize::Spr8x16;

    // which table are sprites in?
    u16 patternTableBaseAddress = _spriteBaseAddress;

    for (; it != _lineSprites.end(); it++)
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
                if (it == _lineSprites.end())
                {
                    return false;
                }
                // This sprite pixel is transparent, continue searching for lower pri sprites
                continue;
            }

            // Now we know we have an opaque sprite pixel
            if (backgroundOpaque && _spriteZeroOnLine && it == _lineSprites.begin())
            {
                _ppuStatus.SetSpriteZeroHit(true);
            }

            priority = spr->Prioirty();

            u8 tileColor = (spr->Palette() << 2) | patternColor;

            paletteIndex = _vram.loadb(0x3f10 + (u16)tileColor) & 0x3f;
            return true;
        }
    }

    return 0;
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


#if defined(RENDER_NAMETABLE)
void Ppu::RenderNameTable(u8 screen[], int index)
{
    u16 backgroundBaseAddress = _backgroundBaseAddress;

    u16 nameTableBaseAddress = 0;
    switch (index)
    {
    case 0: nameTableBaseAddress = 0x2000; break;
    case 1: nameTableBaseAddress = 0x2400; break;
    case 2: nameTableBaseAddress = 0x2800; break;
    case 3: nameTableBaseAddress = 0x2c00; break;
    }
    u16 attributeTableBaseAddress = nameTableBaseAddress + 0x3c0;

    rgb pixel;

    for (u32 ntIndex = 0; ntIndex < 32 * 30; ntIndex++)
    {
        u8 patternLoPlane[8];
        u8 patternHiPlane[8];

        u8 patternIndex = _vram.loadb(nameTableBaseAddress + ntIndex);

        for (int patternRow = 0; patternRow < 8; patternRow++)
        {
            u16 ntAddress = backgroundBaseAddress + (16 * patternIndex) + patternRow;

            patternHiPlane[patternRow] = _vram.loadb(ntAddress);
            patternLoPlane[patternRow] = _vram.loadb(ntAddress + 8);
        }

        u8 patternColor;

        for (int patternRow = 0; patternRow < 8; patternRow++)
        {
            for (int patternCol = 0; patternCol < 8; patternCol++)
            {
                patternColor = ((patternLoPlane[patternRow] >> (7 - patternCol)) & 1) | (((patternHiPlane[patternRow] >> (7 - patternCol)) << 1) & 1);

                pixel.SetColor(_vram.loadb(0x3f00 + patternColor));

                u64 screenLoc = ((ntIndex / 32) * 256 * 8) + ((ntIndex % 32) * 8) + (patternRow * 256) + patternCol;

                screen[(screenLoc * 3) + 0] = pixel.r;
                screen[(screenLoc * 3) + 1] = pixel.g;
                screen[(screenLoc * 3) + 2] = pixel.b;

                pixel.Reset();
            }
        }
    }
}
#endif