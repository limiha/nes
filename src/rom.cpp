#include "stdafx.h"
#include "rom.h"

#include <fstream>

Rom::Rom(IRomFile* romFile)
    : _romFile(romFile)
    , PrgRom(0)
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

bool Rom::Create(IRomFile* romFile, Rom** rom)
{
    NPtr<Rom> newRom(new Rom(romFile));
    if (newRom->Load())
    {
        *rom = newRom.Detach();
        return true;
    }
    else
    {
        *rom = nullptr;
        return false;
    }
}

bool Rom::Load()
{
    NPtr<IReadStream> stream;
    if (_romFile->GetRomFileStream(&stream))
    {
        stream->ReadBytes((u8*)&Header, sizeof(Header));

        if (!Header.ValidateHeader())
        {
            printf("Invalid nes header\n");
            return false;
        }

        //TODO: Figure out how to handle a trainer
        if (Header.HasTrainer())
        {
            printf("ROMs with trainers are not supported\n");
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
            stream->ReadBytes((u8*)&PrgRom[0], PRG_ROM_BANK_SIZE * Header.PrgRomSize);
        }

        if (Header.ChrRomSize > 0)
        {
            ChrRom.resize(Header.ChrRomSize * CHR_ROM_BANK_SIZE);
            stream->ReadBytes((u8*)&ChrRom[0], CHR_ROM_BANK_SIZE * Header.ChrRomSize);
        }
        return true;
    }
    else
    {
        printf("Unable to open file\n");
        return false;
    }
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
    NPtr<IWriteStream> stream;
    if (_romFile->GetSaveGameStream(&stream))
    {
        stream->WriteBytes(&PrgRam[0], PrgRam.size());
    }
}

void Rom::LoadGame()
{
    NPtr<IReadStream> stream;
    if (_romFile->GetLoadGameStream(&stream))
    {
        stream->ReadBytes(&PrgRam[0], PrgRam.size());
    }
}
