#pragma once

class Ppu;
class Apu;
class Input;
class Rom;

#include "interfaces.h"

// CPU Memory Map
class MemoryMap : public IMem, public NesObject
{
public:
    MemoryMap(Ppu*, Apu*, Input*, IMapper*);
    virtual ~MemoryMap();

public:
    DELEGATE_NESOBJECT_REFCOUNTING();

    void Reset(bool hard);

    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
private:
    u8 _ram[0x800];
    NPtr<Ppu> _ppu;
    NPtr<Apu> _apu;
    NPtr<Input> _input;
    NPtr<IMapper> _mapper;
};