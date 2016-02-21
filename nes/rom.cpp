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

        //TODO: This needs to be handled by mapper support
        if (Header.PrgRomSize > 2)
        {
            return false;
        }
        if (Header.ChrRomSize > 1)
        {
            return false;
        }
        if (Header.HasTrainer())
        {
            return false;
        }

        stream.read((char*)_PrgRom, PRG_ROM_BANK_SIZE * Header.PrgRomSize);
        stream.read((char*)_ChrRom, CHR_ROM_BANK_SIZE * Header.ChrRomSize);

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