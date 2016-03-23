#pragma once

class Ppu;
class Apu;
class Input;
class Rom;

// CPU Memory Map
class MemoryMap : public IMem
{
public:
    MemoryMap(std::shared_ptr<Ppu>, std::shared_ptr<Apu>, std::shared_ptr<Input>, std::shared_ptr<IMapper>);
    ~MemoryMap();

    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);

    void SaveState(std::ofstream& ofs);
    void LoadState(std::ifstream& ifs);
private:
    u8 _ram[0x800];
    std::shared_ptr<Ppu> _ppu;
    std::shared_ptr<Apu> _apu;
    std::shared_ptr<Input> _input;
    std::shared_ptr<IMapper> _mapper;
};