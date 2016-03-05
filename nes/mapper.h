#pragma once

class Rom;

enum class NameTableMirroring;

class IMapper
{
protected:
    IMapper(Rom& rom);
    ~IMapper();

public:
    static std::shared_ptr<IMapper> CreateMapper(Rom& rom);

public:
    virtual u8 prg_loadb(u16 addr) = 0;
    virtual void prg_storeb(u16 addr, u8 val) = 0;
    virtual u8 chr_loadb(u16 addr) = 0;
    virtual void chr_storeb(u16 addr, u8 val) = 0;

public:
    NameTableMirroring Mirroring;
    
protected:
    Rom& _rom;
};

class NRom : public IMapper
{
public:
    NRom(Rom& rom);
    ~NRom();

public:
    virtual u8 prg_loadb(u16 addr);
    virtual void prg_storeb(u16 addr, u8 val);
    virtual u8 chr_loadb(u16 addr);
    virtual void chr_storeb(u16 addr, u8 val);

private:
    u8* _buf;
    u8 _chrRam[0x2000]; // If no ChrRom is provided we will give ChrRam
};

class SxRom : public IMapper
{
public:
    SxRom(Rom& rom);
    ~SxRom();

public:
    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
    void chr_storeb(u16 addr, u8 val);

private:
    enum class ChrMode
    {
        Mode8k = 0,
        Mode4k = 1
    };

    enum class PrgSize
    {
        Size32k = 0,
        Size16k = 1,
    };


    enum class SxPrgBankMode
    {
        Switch32K,
        FixFirstBank,
        FixLastBank
    };

    struct SxControl
    {
        u8 val;

        SxPrgBankMode PrgRomMode()
        {
            switch ((val >> 2) & 0x3)
            {
            case 0:
            case 1:
                return SxPrgBankMode::Switch32K;
            case 2:
                return SxPrgBankMode::FixFirstBank;
            case 3:
                return SxPrgBankMode::FixLastBank;
            }
        }
    };

private:
    SxControl control;
    u8 _chrBank0;
    u8 _chrBank1;
    u8 _prgBank;
    u8 _accumulator;
    u8 _writeCount;
    u8 _chrRam[0x2000];
    u8 _prgRam[0x2000];
};

class CNRom : public NRom
{
public:
    CNRom(Rom& rom);
    ~CNRom();

    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
private:
    u8 _chrBank;
};
