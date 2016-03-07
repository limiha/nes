#pragma once

class ISave
{
public:
    virtual void Save(std::ofstream& ofs) = 0;
    virtual void Load(std::ifstream& ifs) = 0;
};

// Standard Memory Interace
class IMem : public ISave
{
public:
    virtual u8 loadb(u16 addr) = 0;
    virtual void storeb(u16 addr, u8 val) = 0;

    // default ISave
    virtual void Save(std::ofstream& ofs) { }

    virtual void Load(std::ifstream& ifs) { }

    u16 loadw(u16 addr)
    {
        u16 lo = (u16)loadb(addr);
        u16 hi = (u16)loadb(addr + 1);

        return (hi << 8) | lo;
    }

    void storew(u16 addr, u16 val)
    {
        u8 lo = (u8)(val & 0xff);
        u8 hi = (u8)((val >> 8) & 0xff);

        storeb(addr, lo);
        storeb(addr + 1, hi);
    }

    // this has wrapround behavior for the zero page
    // reading a word at 0xff reads two bytes from 0xff and 0x00
    u16 loadw_zp(u8 addr)
    {
        u16 lo = (u16)loadb((u16)addr);
        u16 hi = (u16)loadb((u16)((u8)(addr + 1)));
        return (hi << 8) | lo;
    }
};

// Mapper Interface
class Rom;
enum class NameTableMirroring : u8;
class IMapper : public ISave
{
protected:
    IMapper(std::shared_ptr<Rom> rom);
    ~IMapper();

public:
    static std::shared_ptr<IMapper> CreateMapper(std::shared_ptr<Rom> rom);

public:
    virtual u8 prg_loadb(u16 addr) = 0;
    virtual void prg_storeb(u16 addr, u8 val) = 0;
    virtual u8 chr_loadb(u16 addr) = 0;
    virtual void chr_storeb(u16 addr, u8 val) = 0;

public:
    virtual void Save(std::ofstream& ofs);
    virtual void Load(std::ifstream& ifs);

public:
    NameTableMirroring Mirroring;

protected:
    std::shared_ptr<Rom> _rom;
};
