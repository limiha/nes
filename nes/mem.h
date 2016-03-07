#pragma once

class Ppu;
class Apu;
class Input;
class Rom;

// CPU Memory Map
class MemoryMap : public IMem
{
public:
    MemoryMap(Ppu& ppu, Apu& apu, Input& input, std::shared_ptr<IMapper>);
    ~MemoryMap();

    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void Save(std::ofstream& ofs);
    void Load(std::ifstream& ifs);
private:
    u8 _ram[0x800];
    Ppu& _ppu;
    Apu& _apu;
    Input& _input;
    std::shared_ptr<IMapper> _mapper;
};