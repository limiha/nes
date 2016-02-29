#pragma once

class Rom;

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
    
protected:
    Rom& _rom;
};

class NRom : public IMapper
{
public:
    NRom(Rom& rom);
    ~NRom();

public:
    u8 prg_loadb(u16 addr);
    void prg_storeb(u16 addr, u8 val);
    u8 chr_loadb(u16 addr);
    void chr_storeb(u16 addr, u8 val);
};
