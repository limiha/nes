#include "stdafx.h"
#include "mem.h"
#include "ppu.h"
#include "apu.h"
#include "input.h"

/*
    MemoryMap
*/
MemoryMap::MemoryMap(Ppu* ppu, Apu* apu, Input* input, IMapper* mapper)
    : _ppu(ppu)
    , _apu(apu)
    , _input(input)
    , _mapper(mapper)
{
    memset(_ram, 0, sizeof(_ram));
}

MemoryMap::~MemoryMap()
{

}

// IMem
u8 MemoryMap::loadb(u16 addr)
{
    if (addr < 0x2000)
    {
        return _ram[addr & 0x7ff];
    }
    else if (addr < 0x4000)
    {
        return _ppu->loadb(addr);
    }
    else if (addr < 0x4016)
    {
        return _apu->loadb(addr);
    }
    else if (addr < 0x4020)
    {
        return _input->loadb(addr);
    }
    else if (addr < 0x6000)
    {
        // TODO: Expansion ROM
        // TODO: SRAM
        return 0;
    }
    else
    {
        return _mapper->prg_loadb(addr);
    }
}

void MemoryMap::storeb(u16 addr, u8 val)
{
    if (addr < 0x2000)
    {
        _ram[addr & 0x7ff] = val;
    }
    else if (addr < 0x4000)
    {
        _ppu->storeb(addr, val);
    }
    else if (addr < 0x4016)
    {
        _apu->storeb(addr, val);
    }
    else if (addr < 0x4020)
    {
        if (addr == 0x4017)
            _apu->storeb(addr, val);
        else
            _input->storeb(addr, val);
    }
    else if (addr < 0x6000)
    {
        // TODO: Expansion ROM
        // TODO: SRAM
    }
    else
    {
        _mapper->prg_storeb(addr, val);
    }
}

void MemoryMap::SaveState(std::ofstream& ofs)
{
    ofs.write((char*)_ram, sizeof(_ram));
    _ppu->SaveState(ofs);
    _apu->SaveState(ofs);
    _mapper->SaveState(ofs);
}

void MemoryMap::LoadState(std::ifstream& ifs)
{
    ifs.read((char*)_ram, sizeof(_ram));
    _ppu->LoadState(ifs);
    _apu->LoadState(ifs);
    _mapper->LoadState(ifs);
}