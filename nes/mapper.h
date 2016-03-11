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
    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

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
    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);

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

class UxRom : public NRom
{
public:
    UxRom(std::shared_ptr<Rom> rom);
    ~UxRom();

    void prg_storeb(u16 addr, u8 val);
    u8 prg_loadb(u16 addr);

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
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

    // ISaveState
    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
private:
    u8 _chrBank;
};

// MMC3
class TxRom : public IMapper
{
public:
    TxRom(std::shared_ptr<Rom> rom);
    ~TxRom();

    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
    void chr_storeb(u16 addr, u8 val);

    bool Scanline();

private:
    u32 ChrBufAddress(u16 addr);

private:
    bool _chrMode;
    bool _prgMode;
    u8 _addr8001; // "address" to use when writing $80001
    u8 _chrReg[6];
    u8 _prgReg[2];

    u32 _lastBankIndex;
    u32 _secondLastBankIndex;

    u16 _irqCounter;
    u16 _irqReload;
    bool _irqEnable;
    bool _irqPending;
};

// AxRom, #7
class AxRom : public NRom 
{
public:
    AxRom(std::shared_ptr<Rom> rom);
    ~AxRom();

    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);

private:
    u8 _prgReg;
};