#pragma once

#include "mem.h"

#include <vector>

const u32 PRG_ROM_BANK_SIZE = 0x4000;
const u32 PRG_RAM_UNIT_SIZE = 0x2000;
const u32 CHR_ROM_BANK_SIZE = 0x2000;

enum class NameTableMirroring : u8
{
    SingleScreenUpper = 0,
    SingleScreenLower = 1,
    Vertical = 2,
    Horizontal = 3
};

#pragma pack()
struct INesHeader 
{
    // 'N' 'E' 'S' '\x1a'
    u8 magic[4];
    u8 PrgRomSize;
    u8 ChrRomSize;
    u8 flags6;
    u8 flags7;
    u8 PrgRamSize;
    u8 flags9;
    u8 flags10;
    u8 zero[5];

    bool ValidateHeader()
    {
        if (strncmp((char*)magic, "NES\x1a", 4) != 0)
        {
            return false;
        }

        if (strncmp(&((char*)this)[7], "DiskDude!", 9) == 0)
        {
            // Header Garbage, clear rest of header
            flags7 = 0;
            PrgRamSize = 0;
            flags9 = 0;
            flags10 = 0;
            memset(zero, 0, sizeof(zero));
        }

        return true;
    }

    bool HasTrainer()
    {
        return (flags6 & (1 << 2)) != 0;
    }

    bool HasSaveRam()
    {
        return (flags6 & (1 << 1)) != 0;
    }

    u32 MapperNumber()
    {
        u8 lo = (flags6 >> 4) & 0x0f;
        u8 hi = (flags7 >> 4) & 0x0f;

        return (hi << 4) | lo;
    }

    NameTableMirroring Mirroring()
    {
        return (flags6 & 0x1) == 0 ? NameTableMirroring::Horizontal : NameTableMirroring::Vertical;
    }
};

class Rom : public ISaveState, public NesObject
{
private:
    Rom(IRomFile* romFile);

public:
    static bool Create(IRomFile* romFile, Rom** rom);
    ~Rom();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    virtual void SaveState(std::ofstream& ofs);
    virtual void LoadState(std::ifstream& ifs);

private:
    bool Load();

    void SaveGame();
    void LoadGame();

public:
    INesHeader Header;
    std::vector<u8> PrgRom;
    std::vector<u8> PrgRam;
    std::vector<u8> ChrRom;

private:
    NPtr<IRomFile> _romFile;
};

// TODO: These std stream implementations will be used by save state as well, so move them somewhere more accessible
class StdReadStream : public IReadStream, public NesObject
{
private:
    StdReadStream(const char* path)
        : _stream(path, std::ios::binary)
    {
    }

public:
    static bool Create(const char* path, IReadStream** stream)
    {
        NPtr<StdReadStream> newStream(new StdReadStream(path));
        if (newStream->_stream.is_open())
        {
            *stream = newStream.Detach();
            return true;
        }
        else
        {
            *stream = nullptr;
            return false;
        }
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    int ReadBytes(u8* buf, int count)
    {
        _stream.read((char*)buf, count);
        return _stream.gcount();
    }

private:
    std::ifstream _stream;
};

class StdWriteStream : public IWriteStream, public NesObject
{
private:
    StdWriteStream(const char* path)
        : _stream(path, std::ios::binary | std::ios::trunc) // TODO: Don't default to trunc?
    {
    }

public:
    static bool Create(const char* path, IWriteStream** stream)
    {
        NPtr<StdWriteStream> newStream(new StdWriteStream(path));
        if (newStream->_stream.is_open())
        {
            *stream = newStream.Detach();
            return true;
        }
        else
        {
            *stream = nullptr;
            return false;
        }
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    int WriteBytes(u8* buf, int count)
    {
        _stream.write((char*)buf, count);
        return 0; // TODO: fix?
    }

private:
    std::ofstream _stream;
};

class StdStreamRomFile : public IRomFile, public NesObject
{
public:
    StdStreamRomFile(const char* romPath)
        : _romPath(romPath)
    {
    }

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

public:
    bool GetRomFileStream(IReadStream** stream)
    {
        return StdReadStream::Create(_romPath, stream);
    }

    // TOOD: Path transfomration for these
    bool GetSaveGameStream(IWriteStream** stream)
    {
        return false;
    }

    bool GetLoadGameStream(IReadStream** stream)
    {
        return false;
    }

private:
    const char* _romPath;
};
