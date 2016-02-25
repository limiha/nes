#include "stdafx.h"
#include "rom.h"

#include <fstream>

Rom::Rom()
{
}

Rom::~Rom()
{
}

bool Rom::Load(std::string romPath)
{
    std::ifstream stream(romPath, std::ios::in | std::ios::binary);
    if (stream.is_open())
    {
        // read the header
        stream.read((char*)&Header, sizeof(Header));

        if (!Header.ValidateHeader())
        {
            return false;
        }

        if (Header.MapperNumber() != 0)
        {
            return false;
        }

        //TODO: Figure out how to handle a trainer
        if (Header.HasTrainer())
        {
            return false;
        }

        _PrgRom.resize(Header.PrgRomSize * PRG_ROM_BANK_SIZE);
        stream.read((char*)&_PrgRom[0], PRG_ROM_BANK_SIZE * Header.PrgRomSize);

        _ChrRom.resize(Header.ChrRomSize * CHR_ROM_BANK_SIZE);
        stream.read((char*)&_ChrRom[0], CHR_ROM_BANK_SIZE * Header.ChrRomSize);

        stream.close();

        return true;
    }
    else 
    {
        return false;
    }
}

u8 Rom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return 0;
    }
    else
    {
        if (Header.PrgRomSize == 1)
        {
            return _PrgRom[addr & 0x3fff];
        }
        else
        {
            return _PrgRom[addr & 0x7fff];
        }
    }
}

u8 Rom::chr_loadb(u16 addr)
{
    return _ChrRom[addr];
}