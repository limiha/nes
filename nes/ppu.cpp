#pragma once

#include "stdafx.h"
#include "ppu.h"

VRam::VRam(Rom& rom)
    : _rom(rom)
{
    ZeroMemory(_nametables, sizeof(_nametables));
    ZeroMemory(_pallete, sizeof(_pallete));
}

VRam::~VRam()
{
}

u8 VRam::loadb(u16 addr)
{
    if (addr < 0x2000)
    {
        return _rom.chr_loadb(addr);
    }
    else if (addr < 0x3f00)
    {
        return _nametables[addr & 0x7ff];
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
    if (addr < 0x2000)
    {
        // uniplemented 
        __debugbreak();
    }
    else if (addr < 0x3f00)
    {
        _nametables[addr & 0x7ff] = val;
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

Ppu::Ppu(VRam& vram)
    : _vram(vram)
    , _cycles(0)
    , _scanline(VBLANK_SCANLINE)
    , _ppuDatatBuffer(0)
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
    u16 addr = _regs.addr.val;
    u8 val = _vram.loadb(addr);
    _regs.addr.val += _regs.ctrl.VRamAddrIncrement();

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
}

void Ppu::WritePpuMask(u8 val)
{
    // TODO: side effects of writing

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
    // TODO
}

void Ppu::WritePpuAddr(u8 val)
{
    if (_regs.addr.next == PpuAddr::WhichByte::Hi)
    {
        _regs.addr.val &= 0x00ff;
        _regs.addr.val |= (((u16)val) << 8);
        _regs.addr.next = PpuAddr::WhichByte::Lo;
    }
    else
    {
        _regs.addr.val &= 0xff00;
        _regs.addr.val |= (u16)val;
        _regs.addr.next = PpuAddr::WhichByte::Hi;

        // TODO: Something about resetting scrolling here?
    }
}

void Ppu::WritePpuData(u8 val)
{
    _vram.storeb(_regs.addr.val, val);

    _regs.addr.val += _regs.ctrl.VRamAddrIncrement();
}

// Rendering

void Ppu::PutPixel(u32 x, u32 y, rgb& pixel)
{
    Screen[(y * SCREEN_WIDTH + x) * 3 + 0] = pixel.r;
    Screen[(y * SCREEN_WIDTH + x) * 3 + 1] = pixel.g;
    Screen[(y * SCREEN_WIDTH + x) * 3 + 2] = pixel.b;
}

u32 Ppu::GetBackgroundColor(u32 x, u32 y)
{
    if (!_regs.mask.ShowBackground())
    {
        return 0;
    }

    // FOR THE TIME BEING I AM ONLY DRAWING THE CONTENTS OF THE FIRST NAME TABLE
    // THIS IS NOT AT ALL CORRECT

    // The Name Tables store tile numbers
    // These Tile Numbers are indices into the pattern tables
    // The first step is to figure out the correct address to read from the name tables

    // Today just use the first name table
    u16 nameTableBaseAddress = 0x2000;

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

    u8 loPaletteIndexBits = (hiPlaneBit << 1) | loPlaneBit;

    u16 attributeTableBaseAddress = nameTableBaseAddress + 0x3c0;

    u16 attributeTableIndexX = nameTableIndexX / 4;
    u16 attributeTableIndexY = nameTableIndexY / 4;
    u16 attributeTableOffset = (attributeTableIndexY * 8) + attributeTableIndexX;
    u16 attributeTableAddress = attributeTableBaseAddress + attributeTableOffset;

    u8 attributeByte = _vram.loadb(attributeTableAddress);

    u8 attributeByteIndexX = (nameTableIndexX % 4) / 2;
    u8 attributeByteIndexY = (nameTableIndexY % 4) / 2;
    u8 attributeByteIndex = (attributeByteIndexY * 2) + attributeByteIndexX;

    u8 hiPaletteIndexBits = (attributeByte >> (attributeByteIndex * 2)) & 0x3;

    u8 tileColor = (hiPaletteIndexBits << 2) | loPaletteIndexBits;

    u8 paletteIndex = _vram.loadb(0x3f00 + (tileColor & 0x3f));

    return paletteIndex;
}

void Ppu::RenderScanline()
{
    rgb pixel;
    for (u32 x = 0; x < SCREEN_WIDTH; x++)
    {
        pixel.Reset();
        u32 paletteIndex = GetBackgroundColor(x, _scanline);
        pixel.SetColor(paletteIndex);
        PutPixel(x, _scanline, pixel);
    }
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
        }
    }
}