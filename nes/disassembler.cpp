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

void Disassembler::Disassemble(DisassembledInstruction** ppDisassembledInstruction)
{
    DisassembledInstruction* am = new DisassembledInstruction();

    u8 op = LoadBBumpPC();
    am->_bytes.push_back(op);

    DECODE(op)

    *ppDisassembledInstruction = am;
}
