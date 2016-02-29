#include "stdafx.h"
#include "mapper.h"
#include "rom.h"

IMapper::IMapper(Rom& rom)
    : _rom(rom)
{
}

IMapper::~IMapper()
{
}

std::shared_ptr<IMapper> IMapper::CreateMapper(Rom& rom)
{
    switch (rom.Header.MapperNumber())
    {
    case 0:
        return std::make_shared<NRom>(rom);
    default:
        printf("Unsupported mapper: %d\n", rom.Header.MapperNumber());
        return nullptr;
    }
}

NRom::NRom(Rom& rom)
    : IMapper(rom)
{
}

NRom::~NRom()
{
}

u8 NRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return 0;
    }
    else
    {
        if (_rom.Header.PrgRomSize == 1)
        {
            return _rom.PrgRom[addr & 0x3fff];
        }
        else
        {
            return _rom.PrgRom[addr & 0x7fff];
        }
    }
}

void NRom::prg_storeb(u16 addr, u8 val)
{
    // Does nothing 
}

u8 NRom::chr_loadb(u16 addr)
{
    return _rom.ChrRom[addr];
}

void NRom::chr_storeb(u16 addr, u8 val)
{
    // Does nothing
}
