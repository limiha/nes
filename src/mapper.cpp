#include "stdafx.h"
#include "mapper.h"
#include "rom.h"

IMapper::IMapper(std::shared_ptr<Rom> rom)
    : _rom(rom)
{
    Mirroring = _rom->Header.Mirroring();
}

IMapper::~IMapper()
{
}

std::shared_ptr<IMapper> IMapper::CreateMapper(std::shared_ptr<Rom> rom)
{
    switch (rom->Header.MapperNumber())
    {
    case 0:
        return std::make_shared<NRom>(rom);
    case 1:
        return std::make_shared<SxRom>(rom);
    case 2:
        return std::make_shared<UxRom>(rom);
    case 3:
        return std::make_shared<CNRom>(rom);
    case 4:
        return std::make_shared<TxRom>(rom);
    case 7:
        return std::make_shared<AxRom>(rom);
    default:
        printf("Unsupported mapper: %d\n", rom->Header.MapperNumber());
        return nullptr;
    }
}

bool IMapper::Scanline()
{
    return false;
}

void IMapper::SaveState(std::ofstream& ofs)
{
    Util::WriteBytes((u8)Mirroring, ofs);
    _rom->SaveState(ofs);
}

void IMapper::LoadState(std::ifstream& ifs)
{
    Util::ReadBytes((u8&)Mirroring, ifs);
    _rom->LoadState(ifs);
}

/// NRom

NRom::NRom(std::shared_ptr<Rom> rom)
    : IMapper(rom)
{
    if (_rom->Header.ChrRomSize > 0)
    {
        _chrBuf = &_rom->ChrRom[0];
    }
    else
    {
        _chrBuf = _chrRam;
    }
}

NRom::~NRom()
{
}

u8 NRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return _rom->PrgRam[addr & 0x1fff];
    }
    else
    {
        if (_rom->Header.PrgRomSize == 1)
        {
            return _rom->PrgRom[addr & 0x3fff];
        }
        else
        {
            return _rom->PrgRom[addr & 0x7fff];
        }
    }
}

void NRom::prg_storeb(u16 addr, u8 val)
{
    if (addr < 0x8000)
    {
        _rom->PrgRam[addr & 0x1fff] = val;
    }
}

u8 NRom::chr_loadb(u16 addr)
{
    return _chrBuf[addr]; // this will return ChrRom if present or ChrRam if not
}

void NRom::chr_storeb(u16 addr, u8 val)
{
    _chrRam[addr] = val; // This will only ever store to ChrRam
}

void NRom::SaveState(std::ofstream& ofs)
{
    IMapper::SaveState(ofs);
    ofs.write((char*)_chrRam, sizeof(_chrRam));
}

void NRom::LoadState(std::ifstream& ifs)
{
    IMapper::LoadState(ifs);
    ifs.read((char*)_chrRam, sizeof(_chrRam));
}

/// SxRom (Mapper #1)

SxRom::SxRom(std::shared_ptr<Rom> rom)
    : IMapper(rom)
    , _prgSize(PrgSize::Size16k)
    , _chrMode(ChrMode::Mode8k)
    , _slotSelect(true)
    , _chrBank0(0)
    , _chrBank1(0)
    , _prgBank(0)
    , _accumulator(0)
    , _writeCount(0)
{
    if (rom->Header.ChrRomSize > 0)
    {
        _chrBuf = &rom->ChrRom[0];
    }
    else
    {
        _chrRam.resize(0x2000);
        _chrBuf = &_chrRam[0];
    }
}

SxRom::~SxRom()
{
}

u8 SxRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return _rom->PrgRam[addr & 0x1fff];
    }
    else
    {
        if (_prgSize == PrgSize::Size32k)
        {
            return _rom->PrgRom[((_prgBank >> 1) * 0x4000 * 2) + (addr & 0x7fff)];
        }
        else if (_prgSize == PrgSize::Size16k)
        {
            if (addr < 0xc000)
            {
                if (!_slotSelect)
                {
                    return _rom->PrgRom[addr & 0x3fff];
                }
                else
                {
                    return _rom->PrgRom[(_prgBank * 0x4000) + (addr & 0x3fff)];
                }
            }
            else
            {
                if (!_slotSelect)
                {
                    return _rom->PrgRom[(_prgBank * 0x4000) + (addr & 0x3fff)];
                }
                else
                {
                    return _rom->PrgRom[((_rom->Header.PrgRomSize - 1) * 0x4000) + (addr & 0x3fff)];
                }
            }
        }
        else
        {
            // can't happen
            __debugbreak();
            return 0;
        }
    }
}

void SxRom::prg_storeb(u16 addr, u8 val)
{
    if (addr < 0x8000)
    {
        _rom->PrgRam[addr & 0x1fff] = val;
        return;
    }

    if ((val & (1 << 7)) != 0)
    {
        _writeCount = 0;
        _accumulator = 0;
        _prgSize = PrgSize::Size16k;
        _slotSelect = true;
        return;
    }

    _accumulator = _accumulator | ((val & 1) << _writeCount++);
    if (_writeCount == 5)
    {
        _writeCount = 0;

        if (addr <= 0x9fff)
        {
            switch (_accumulator & 0x3)
            {
            case 0: Mirroring = NameTableMirroring::SingleScreenLower; break;
            case 1: Mirroring = NameTableMirroring::SingleScreenUpper; break;
            case 2: Mirroring = NameTableMirroring::Vertical; break;
            case 3: Mirroring = NameTableMirroring::Horizontal; break;
            }
            _slotSelect = (_accumulator & (1 << 2)) != 0;
            _prgSize = (_accumulator & (1 << 3)) == 0 ? PrgSize::Size32k : PrgSize::Size16k;
            _chrMode = (_accumulator & (1 << 4)) == 0 ? ChrMode::Mode8k : ChrMode::Mode4k;
        }
        else if (addr <= 0xbfff)
        {
            _chrBank0 = _accumulator & 0x1f;
        }
        else if (addr <= 0xdfff)
        {
            _chrBank1 = _accumulator & 0x1f;
        }
        else
        {
            _prgBank = (_accumulator & 0xf);
        }

        _accumulator = 0;
    }
}

u8 SxRom::chr_loadb(u16 addr)
{
    return _chrBuf[ChrBufAddress(addr)];
}

void SxRom::chr_storeb(u16 addr, u8 val)
{
    _chrBuf[ChrBufAddress(addr)] = val;
}

u32 SxRom::ChrBufAddress(u16 addr)
{
    if (_chrMode == ChrMode::Mode4k)
    {
        if (addr < 0x1000)
        {
            return (_chrBank0 * 0x1000) + addr;
        }
        else
        {
            return (_chrBank1 * 0x1000) + (addr & 0xfff);
        }
    }
    else
    {
        return ((_chrBank0 >> 1) * 0x2000) + addr;
    }
}

void SxRom::SaveState(std::ofstream& ofs)
{
    IMapper::SaveState(ofs);
    Util::WriteBytes((u8)_prgSize, ofs);
    Util::WriteBytes((u8)_chrMode, ofs);
    Util::WriteBytes(_slotSelect, ofs);
    Util::WriteBytes(_chrBank0, ofs);
    Util::WriteBytes(_chrBank1, ofs);
    Util::WriteBytes(_prgBank, ofs);
    Util::WriteBytes(_accumulator, ofs);
    Util::WriteBytes(_writeCount, ofs);
    ofs.write((char*)&_chrRam[0], _chrRam.size());
}

void SxRom::LoadState(std::ifstream& ifs)
{
    IMapper::LoadState(ifs);
    Util::ReadBytes((u8&)_prgSize, ifs);
    Util::ReadBytes((u8&)_chrMode, ifs);
    Util::ReadBytes(_slotSelect, ifs);
    Util::ReadBytes(_chrBank0, ifs);
    Util::ReadBytes(_chrBank1, ifs);
    Util::ReadBytes(_prgBank, ifs);
    Util::ReadBytes(_accumulator, ifs);
    Util::ReadBytes(_writeCount, ifs);
    ifs.read((char*)&_chrRam[0], _chrRam.size());
}

/// UxRom (Mapper #2)

UxRom::UxRom(std::shared_ptr<Rom> rom)
    : NRom(rom)
    , _prgBank(0)
{
    _lastBankOffset = (_rom->Header.PrgRomSize - 1) * PRG_ROM_BANK_SIZE;
}

UxRom::~UxRom()
{
}

void UxRom::SaveState(std::ofstream& ofs)
{
    NRom::SaveState(ofs);
    Util::WriteBytes(_prgBank, ofs);
}

void UxRom::LoadState(std::ifstream& ifs)
{
    NRom::LoadState(ifs);
    Util::ReadBytes(_prgBank, ifs);
}

void UxRom::prg_storeb(u16 addr, u8 val)
{
    _prgBank = val;
}

u8 UxRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return NRom::prg_loadb(addr);
    }
    else if (addr >= 0xC000)
    {
        return _rom->PrgRom[_lastBankOffset + (addr & 0x3FFF)];
    }
    else
    {
        return _rom->PrgRom[(_prgBank * PRG_ROM_BANK_SIZE) + (addr & 0x3FFF)];
    }
}

/// CNRom (Mapper #3)

CNRom::CNRom(std::shared_ptr<Rom> rom)
    : NRom(rom)
    , _chrBank(0)
{
}

CNRom::~CNRom()
{
}

void CNRom::prg_storeb(u16 addr, u8 val)
{
    // CNROM only supports 32KB of CHR ROM, but some games write values
    // such as $FF to switch to bank 3 so we need to mask with 0x03
    _chrBank = val & 0x03;
}

u8 CNRom::chr_loadb(u16 addr)
{
    return _rom->ChrRom[(_chrBank * CHR_ROM_BANK_SIZE) + addr];
}
void CNRom::SaveState(std::ofstream& ofs)
{
    NRom::SaveState(ofs);
    Util::WriteBytes(_chrBank, ofs);
}

void CNRom::LoadState(std::ifstream& ifs)
{
    NRom::LoadState(ifs);
    Util::ReadBytes(_chrBank, ifs);
}

// TXRom (MMC3, mapper #4)

TxRom::TxRom(std::shared_ptr<Rom> rom)
    : IMapper(rom)
    , _prgMode(false)
    , _chrMode(false)
    , _prgReg{0, 0}
    , _chrReg{0, 0, 0, 0, 0, 0}
{
    _lastBankIndex = (_rom->Header.PrgRomSize * 2) - 1; // PrgRomSize is in 0x4000 units, TxRom has 0x2000 size banks
    _secondLastBankIndex = (_rom->Header.PrgRomSize * 2) - 2; // PrgRomSize is in 0x4000 units, TxRom has 0x2000 size banks
}

TxRom::~TxRom()
{
}

u8 TxRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        // TODO: This can be disabled?
        return _rom->PrgRam[addr & 0x1fff];
    }
    else
    {
        if (!_prgMode)
        {
            if (addr < 0xa000)
            {
                return _rom->PrgRom[(_prgReg[0] * 0x2000) + (addr & 0x1fff)];
            }
            else if (addr < 0xc000)
            {
                return _rom->PrgRom[(_prgReg[1] * 0x2000) + (addr & 0x1fff)];
            }
            else if (addr < 0xe000)
            {
                return _rom->PrgRom[(_secondLastBankIndex * 0x2000) + (addr & 0x1fff)];
            }
            else
            {
                return _rom->PrgRom[(_lastBankIndex * 0x2000) + (addr & 0x1fff)];
            }
        }
        else
        {
            if (addr < 0xa000)
            {
                return _rom->PrgRom[(_secondLastBankIndex * 0x2000) + (addr & 0x1fff)];
            }
            else if (addr < 0xc000)
            {
                return _rom->PrgRom[(_prgReg[1] * 0x2000) + (addr & 0x1fff)];
            }
            else if (addr < 0xe000)
            {
                return _rom->PrgRom[(_prgReg[0] * 0x2000) + (addr & 0x1fff)];
            }
            else
            {
                return _rom->PrgRom[(_lastBankIndex * 0x2000) + (addr & 0x1fff)];
            }
        }
    }
}

void TxRom::prg_storeb(u16 addr, u8 val)
{
    if (addr < 0x8000)
    {

        // TODO: This can be disabled?
        _rom->PrgRam[addr & 0x1fff] = val;
    }
    else
    {
        addr &= 0xe001;
        switch (addr)
        {
        case 0x8000:
            _chrMode = (val & (1 << 7)) != 0;
            _prgMode = (val & (1 << 6)) != 0;
            _addr8001 = val & 0b111;
            break;
        case 0x8001:
            if (_addr8001 < 7)
            {
                _chrReg[_addr8001] = val;
            }
            else
            {
                _prgReg[_addr8001 & 1] = val;
            }
            break;
        case 0xa000:
            Mirroring = (val & 1) == 0 ? NameTableMirroring::Vertical : NameTableMirroring::Horizontal;
            break;
        case 0xa001:
            // TODO: Something to do with enabling/disabling WRAM (Which i think is PrgRam);
            break;
        case 0xc000:
            _irqReload = val;
            break;
        case 0xc001:
            _irqCounter = 0;
            break;
        case 0xe000:
            _irqPending = false;
            _irqEnable = false;
            break;
        case 0xe001:
            _irqEnable = true;
            break;
        }
    }
}

u8 TxRom::chr_loadb(u16 addr)
{
    return _rom->ChrRom[ChrBufAddress(addr)];
}

void TxRom::chr_storeb(u16 addr, u8 val)
{
    // not sure if mmc3 can have ChrRam
}

u32 TxRom::ChrBufAddress(u16 addr)
{
    if (!_chrMode)
    {
        if (addr < 0x0800)
        {
            return ((_chrReg[0] >> 1) * 0x0800) + (addr & 0x07ff);
        }
        else if (addr < 0x1000)
        {
            return ((_chrReg[1] >> 1) * 0x0800) + (addr & 0x07ff);
        }
        else if (addr < 0x1400)
        {
            return (_chrReg[2] * 0x0400) + (addr & 0x03ff);
        }
        else if (addr < 0x1800)
        {
            return (_chrReg[3] * 0x0400) + (addr & 0x03ff);
        }
        else if (addr < 0x1c00)
        {
            return (_chrReg[4] * 0x0400) + (addr & 0x03ff);
        }
        else
        {
            return (_chrReg[5] * 0x0400) + (addr & 0x03ff);
        }
    }
    else
    {
        if (addr < 0x0400)
        {
            return (_chrReg[2] * 0x0400) + (addr & 0x03ff);
        }
        else if (addr < 0x0800)
        {
            return (_chrReg[3] * 0x0400) + (addr & 0x03ff);
        }
        else if (addr < 0x0c00)
        {
            return (_chrReg[4] * 0x0400) + (addr & 0x03ff);
        }
        else if (addr < 0x1000)
        {
            return (_chrReg[5] * 0x0400) + (addr & 0x03ff);
        }
        else if (addr < 0x1800)
        {
            return ((_chrReg[0] >> 1) * 0x0800) + (addr & 0x07ff);
        }
        else
        {
            return ((_chrReg[1] >> 1) * 0x0800) + (addr & 0x07ff);
        }
    }
}

bool TxRom::Scanline()
{
    if (_irqCounter == 0)
    {
        _irqCounter = _irqReload;
        return false;
    }
    else
    {
        _irqCounter--;
        if (_irqCounter == 0 && _irqEnable)
        {
            _irqPending = true;
            return true;
        }
        return false;
    }
}

// AxRom, Mapper #7
AxRom::AxRom(std::shared_ptr<Rom> rom)
    : NRom(rom)
    , _prgReg(0)
{
}

AxRom::~AxRom()
{
}

u8 AxRom::prg_loadb(u16 addr)
{
    if (addr < 0x8000)
    {
        return NRom::prg_loadb(addr);
    }
    else
    {
        return _rom->PrgRom[((_prgReg * 0x8000) + (addr & 0x7fff))];
    }
}

void AxRom::prg_storeb(u16 addr, u8 val)
{
    if (addr < 0x8000)
    {
        NRom::prg_storeb(addr, val);
    }
    else
    {
        Mirroring = (val & (1 << 4)) == 0 ? NameTableMirroring::SingleScreenLower : NameTableMirroring::SingleScreenUpper;
        _prgReg = val & 0b111;
    }
}