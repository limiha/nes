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
    // TOOD
    return 0;
}

void Ppu::storeb(u16 addr, u8 val)
{
    // TODO
}

void Ppu::Step(u32& cycles)
{
    while (cycles >= CYCLES_PER_SCANLINE)
    {
        cycles -= CYCLES_PER_SCANLINE;
        _scanline++;

        if (_scanline == VBLANK_SCANLINE)
        {
            // set VBLANK
        }

        if (_scanline == LAST_SCANLINE)
        {
            // draw screen
            _scanline = 0;
        }
    }
}