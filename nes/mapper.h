#pragma once

class NRom : public IMapper
{
public:
    NRom(std::shared_ptr<Rom> rom);
    ~NRom();

public:
    virtual u8 prg_loadb(u16 addr);
    virtual void prg_storeb(u16 addr, u8 val);
    virtual u8 chr_loadb(u16 addr);
    virtual void chr_storeb(u16 addr, u8 val);

public:
    // ISave
    void Save(std::ofstream& ofs);
    void Load(std::ifstream& ifs);

private:
    u8* _chrBuf;
    u8 _chrRam[0x2000]; // If no ChrRom is provided we will give ChrRam
};

class SxRom : public IMapper
{
public:
    SxRom(std::shared_ptr<Rom> rom);
    ~SxRom();

public:
    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
    void chr_storeb(u16 addr, u8 val);

public:
    // ISave
    void Save(std::ofstream& ofs);
    void Load(std::ifstream& ifs);

private:
    u32 ChrBufAddress(u16 addr);

private:
    enum class ChrMode : u8
    {
        Mode8k = 0,
        Mode4k = 1
    };

    enum class PrgSize : u8
    {
        Size32k = 0,
        Size16k = 1,
    };

private:
    PrgSize _prgSize;
    ChrMode _chrMode;
    bool _slotSelect;
    u8 _chrBank0;
    u8 _chrBank1;
    u8 _prgBank;
    u8 _accumulator;
    u8 _writeCount;
    u8* _chrBuf;
    std::vector<u8> _chrRam;
};

class UNRom : public NRom
{
public:
    UNRom(std::shared_ptr<Rom> rom);
    ~UNRom();

    void prg_storeb(u16 addr, u8 val);
    u8 prg_loadb(u16 addr);
private:
    int _lastBankOffset;
    u8 _prgBank;
};

class CNRom : public NRom
{
public:
    CNRom(std::shared_ptr<Rom> rom);
    ~CNRom();

    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
private:
    u8 _chrBank;
};
