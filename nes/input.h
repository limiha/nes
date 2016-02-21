#pragma once

#include "mem.h"

class Input : public IMem
{
public:
    Input();
    ~Input();

    // IMem
    u8 loadb(u16 addr);
    void storeb(u16 addr, u8 val);
};
