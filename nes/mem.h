#pragma once
#include "stdafx.h"

class Ppu;
class Apu;
class Input;
class Rom;

// Standard Memory Interace
class IMem 
{
public:
    virtual u8 loadb(u16 addr) = 0;
    virtual void storeb(u16 addr, u8 val) = 0;

    u16 loadw(u16 addr)
    {
        u16 lo = (u16)loadb(addr);
        u16 hi = (u16)loadb(addr + 1);

        return (hi << 8) | lo;
    }

    void storew(u16 addr, u16 val)
    {
        u8 lo = (u8)(val & 0xff);
        u8 hi = (u8)((val >> 8) & 0xff);

        storeb(addr, lo);
        storeb(addr + 1, hi);
    }

    // this has wrapround behavior for the zero page
    // reading a word at 0xff reads two bytes from 0xff and 0x00
    u16 loadw_zp(u16 addr)
    {
        u16 lo = (u16)loadb(addr);
        u16 hi = (u16)loadb((addr + 1) & 0xff);
        return (hi << 8) | lo;
    }
};

// RAM Module for the nes
class Ram : public IMem {
public:
    Ram();
    ~Ram();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
private:
    u8 _ram[0x800];
};

// CPU Memory Map
class MemoryMap : public IMem
{
public:
    MemoryMap(Ppu& ppu, Apu& apu, Input& input, Rom& rom);
    ~MemoryMap();

    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
private:
    Ram _ram;
    Ppu& _ppu;
    Apu& _apu;
    Input& _input;
    Rom& _rom;
};