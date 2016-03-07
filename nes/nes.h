#pragma once

class Rom;
class Cpu;

class Nes
{
public:
    Nes(std::shared_ptr<Rom> rom);
    ~Nes();

    static std::unique_ptr<Nes> Create(const char* romPath);
    static std::unique_ptr<Nes> Create(std::shared_ptr<Rom> rom);

    void Run();

private:
    void SaveState();
    void LoadState();

private:
    std::shared_ptr<Rom> _rom;
    std::unique_ptr<Cpu> _cpu;
};