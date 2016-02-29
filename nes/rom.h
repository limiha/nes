#pragma once

#include "mem.h"

#include <vector>

const u32 PRG_ROM_BANK_SIZE = 0x4000;
const u32 CHR_ROM_BANK_SIZE = 0x2000;

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
        return magic[0] == 'N' && magic[1] == 'E' && magic[2] == 'S' && magic[3] == '\x1a';
    }

    bool HasTrainer()
    {
        return (flags6 & (1 << 1)) != 0;
    }

    u32 MapperNumber()
    {
        u8 lo = (flags6 >> 4) & 0x0f;
        u8 hi = (flags7 >> 4) & 0x0f;

        return (hi << 4) | lo;
    }
};

class Rom
{
public:
    Rom();
    ~Rom();

    bool Load(std::string romPath);

public:
    INesHeader Header;
    std::vector<u8> PrgRom;
    std::vector<u8> ChrRom;
};