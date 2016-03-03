#include "stdafx.h"
#include "mapper.h"
#include "rom.h"

IMapper::IMapper(Rom& rom)
    : _rom(rom)
{
    Mirroring = _rom.Header.Mirroring();
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
    case 1:
        return std::make_shared<SxRom>(rom);
    case 3:
        return std::make_shared<CNRom>(rom);
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

/// SxRom (Mapper #1)

SxRom::SxRom(Rom& rom)
    : IMapper(rom)
    , _chrBank0(0)
    , _chrBank1(0)
    , _prgBank(0)
    , _accumulator(0)
    , _writeCount(0)
{
    ZeroMemory(_chrRam, sizeof(_chrRam));
    ZeroMemory(_prgRam, sizeof(_prgRam));
    control.val = 3 << 2;
}

SxRom::~SxRom()
{
}

u8 SxRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return _prgRam[addr & 0x1fff];
    }
    else if (addr < 0xc000)
    {
        u8 bank;
        switch (control.PrgRomMode())
        {
        case SxPrgBankMode::Switch32K:
            bank = _prgBank & 0xfe;
            break;
        case SxPrgBankMode::FixFirstBank: 
            bank = 0;
            break;
        case SxPrgBankMode::FixLastBank:
            bank = _prgBank;
            break;
        }
        return _rom.PrgRom[((u16)bank * PRG_ROM_BANK_SIZE) | (addr & 0x3fff)];
    }
    else
    {
        u8 bank;
        switch (control.PrgRomMode())
        {
        case SxPrgBankMode::Switch32K:
            bank = (_prgBank & 0xfe) | 1;
            break;
        case SxPrgBankMode::FixFirstBank:
            bank = _prgBank;
            break;
        case SxPrgBankMode::FixLastBank:
            bank = _rom.Header.PrgRomSize - 1;
            break;
        }
        return _rom.PrgRom[((u16)bank * PRG_ROM_BANK_SIZE) | (addr & 0x3fff)];
    }
}

void SxRom::prg_storeb(u16 addr, u8 val)
{
    if (addr < 0x8000)
    {
        _prgRam[addr & 0x1fff] = val;
        return;
    }

    if ((val & (1 << 7)) != 0)
    {
        _writeCount = 0;
        _accumulator = 0;
        control.val = control.val | (3 << 2);
        return;
    }

    _accumulator = _accumulator | ((val & 1) << _writeCount++);
    if (_writeCount == 5)
    {
        _writeCount = 0;

        if (addr <= 0x9fff)
        {
            control.val = _accumulator;
            if ((control.val & 0x3) == 0)
            {
                Mirroring = NameTableMirroring::SingleScreenLower;
            }
            else if ((control.val & 0x3) == 1)
            {
                Mirroring = NameTableMirroring::SingleScreenUpper;
            }
            else if ((control.val & 0x3) == 2)
            {
                Mirroring = NameTableMirroring::Vertical;
            }
            else
            {
                Mirroring = NameTableMirroring::Horizontal;
            }
        }
        else if (addr <= 0xbfff)
        {
            _chrBank0 = _accumulator;
        }
        else if (addr <= 0xdfff)
        {
            _chrBank1 = _accumulator;
        }
        else
        {
            _prgBank = _accumulator;
        }

        _accumulator = 0;
    }
}

u8 SxRom::chr_loadb(u16 addr)
{
    return _chrRam[addr];
}

void SxRom::chr_storeb(u16 addr, u8 val)
{
    _chrRam[addr] = val;
}

/// CNRom (Mapper #3)

CNRom::CNRom(Rom& rom)
    : NRom(rom)
    , _chrBank(0)
{
}

CNRom::~CNRom()
{
}

void CNRom::prg_storeb(u16 addr, u8 val)
{
    _chrBank = val;
}

u8 CNRom::chr_loadb(u16 addr)
{
    return _rom.ChrRom[(_chrBank * CHR_ROM_BANK_SIZE) + addr];
}