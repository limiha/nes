#include "stdafx.h"
#include "diassembler.h"
#include "decode.h"

Disassembler::Disassembler(u16 PC, IMem* mem)
    : _PC(PC)
    , _mem(mem)
{
}

Disassembler::~Disassembler()
{
}

std::string Disassembler::Disassemble()
{
    std::stringstream am(std::stringstream::out | std::stringstream::ate);

    u8 op = LoadBBumpPC();

    DECODE(op)

    return am.str();
}