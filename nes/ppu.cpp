#pragma once

#include "stdafx.h"
#include "ppu.h"

Ppu::Ppu()
    : _cycles(0)
    , _scanline(VBLANK_SCANLINE)
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
        return ReadPpuStatus();
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

void Ppu::Step(u32& cycles, PpuStepResult& result)
{
    while (cycles >= CYCLES_PER_SCANLINE)
    {
        cycles -= CYCLES_PER_SCANLINE;
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

void Ppu::StartVBlank(PpuStepResult& result)
{
    _regs.status.SetInVBlank(true);

    if (_regs.ctrl.VBlankNmi())
    {
        result.VBlankNmi = true;
    }
}

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
    // TODO
    return 0;
}

void Ppu::WritePpuCtrl(u8 val)
{
    // TODO

    _regs.ctrl.val = val;
}

void Ppu::WritePpuMask(u8 val)
{
    // TODO

    _regs.mask.val = val;
}

void Ppu::WriteOamAddr(u8 val)
{
    // TODO
}

void Ppu::WriteOamData(u8 val)
{
    // TODO
}

void Ppu::WritePpuScroll(u8 val)
{
    // TODO
}

void Ppu::WritePpuAddr(u8 val)
{
    // TODO
}

void Ppu::WritePpuData(u8 val)
{
    // TODO
}