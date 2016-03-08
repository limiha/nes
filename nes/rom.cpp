#include "stdafx.h"
#include "rom.h"

#include <fstream>

Rom::Rom()
    : PrgRom(0)
    , ChrRom(0)
{
}

Rom::~Rom()
{
}

bool Rom::Load(std::string romPath)
{
    _path = romPath;

    if (_path.is_relative())
    {
        _path = fs::current_path().append(_path);
    }

    if (!fs::exists(romPath))
    {
        printf("File does not exsit\n");
        return false;
    }

    std::ifstream stream(romPath, std::ios::in | std::ios::binary);
    if (stream.is_open())
    {
        // read the header
        stream.read((char*)&Header, sizeof(Header));

        if (!Header.ValidateHeader())
        {
            return false;
        }

        //TODO: Figure out how to handle a trainer
        if (Header.HasTrainer())
        {
            return false;
        }

        if (Header.PrgRamSize == 0)
        {
            PrgRam.resize(PRG_RAM_UNIT_SIZE);
        }
        else
        {
            PrgRam.resize(Header.PrgRamSize * PRG_RAM_UNIT_SIZE);
        }

        if (Header.PrgRomSize > 0)
        {
            PrgRom.resize(Header.PrgRomSize * PRG_ROM_BANK_SIZE);
            stream.read((char*)&PrgRom[0], PRG_ROM_BANK_SIZE * Header.PrgRomSize);
        }

        if (Header.ChrRomSize > 0)
        {
            ChrRom.resize(Header.ChrRomSize * CHR_ROM_BANK_SIZE);
            stream.read((char*)&ChrRom[0], CHR_ROM_BANK_SIZE * Header.ChrRomSize);
        }

        stream.close();

        return true;
    }
    else 
    {
        return false;
    }
}

const fs::path& Rom::Path()
{
    return _path;
}

void Rom::Save(std::ofstream& ofs)
{
    ofs.write((char*)&PrgRam[0], PrgRam.size());
}

void Rom::Load(std::ifstream& ifs)
{
    ifs.read((char*)&PrgRam[0], PrgRam.size());
}
