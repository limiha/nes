#pragma once

#include "mem.h"

class Apu : public IMem
{
public:
    Apu();
    ~Apu();

    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
private:
};