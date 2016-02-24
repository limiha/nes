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
        u16 palette_addr = (addr | 0xFC) ? addr : 0;
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
        _nametables[addr & 0x7ff];
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
    _vram.loadb(_regs.addr.val);

    _regs.addr.val += _regs.ctrl.VRamAddrIncrement();
}

// Rendering

u8 getRand()
{
    return (u8)(rand() % 256);
}

void Ppu::RenderScanline()
{

    for (u32 x = 0; x < SCREEN_WIDTH; x++)
    {
        Screen[(_scanline * SCREEN_WIDTH + x) * 3 + 0] = getRand();
        Screen[(_scanline * SCREEN_WIDTH + x) * 3 + 1] = getRand();
        Screen[(_scanline * SCREEN_WIDTH + x) * 3 + 2] = getRand();
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