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
    // destructor is probably not the best place for IO
    // but here it goes for now
    if (Header.HasSaveRam())
    {
        SaveGame();
    }
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

        if (Header.HasSaveRam())
        {
            LoadGame();
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

void Rom::SaveState(std::ofstream& ofs)
{
    ofs.write((char*)&PrgRam[0], PrgRam.size());
}

void Rom::LoadState(std::ifstream& ifs)
{
    ifs.read((char*)&PrgRam[0], PrgRam.size());
}

void Rom::SaveGame()
{
    std::ofstream ofs(GetSaveGamePath()->c_str(), std::ofstream::binary | std::ofstream::trunc);
    ofs.write((char*)&PrgRam[0], PrgRam.size());
    ofs.close();
}

void Rom::LoadGame()
{
    auto savePath = GetSaveGamePath();
    if (!fs::exists(*savePath))
    {
        return;
    }

    std::ifstream ifs(savePath->c_str(), std::ifstream::binary);
    ifs.read((char*)&PrgRam[0], PrgRam.size());
    ifs.close();
}

std::unique_ptr<fs::path> Rom::GetSaveGamePath()
{
    fs::path savePath(_path);
    return std::make_unique<fs::path>(savePath.replace_extension("sav"));
}