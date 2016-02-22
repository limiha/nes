#pragma once

#include "mem.h"

class Disassembler
{
public:
    Disassembler(u16 PC, IMem* mem);
    ~Disassembler();

    std::string Disassemble();

private:
    u16 _PC;
    IMem* _mem;

    // Helpers
    u8 LoadBBumpPC()
    {
        return _mem->loadb(_PC++);
    }

    u16 LoadWBumpPC()
    {
        u16 val = _mem->loadw(_PC);
        _PC += 2;
        return val;
    }

    std::string DisBBumpPC()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::setw(2) << std::setfill('0') << LoadBBumpPC();
        return ss.str();
    }

    std::string DisWBumpPC()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::setw(4) << std::setfill('0') << LoadWBumpPC();
        return ss.str();
    }

    // Addressing Modes
    void Immediate(std::stringstream& ss) { ss << '#' << DisWBumpPC(); }
    void Accumulator(std::stringstream& ss) { }
    void ZeroPage(std::stringstream& ss) { ss << DisBBumpPC(); }
    void ZeroPageX(std::stringstream& ss) { ss << DisBBumpPC() << ",X"; }
    void ZeroPageY(std::stringstream& ss) { ss << DisBBumpPC() << ",Y"; }
    void Absolute(std::stringstream& ss) { ss << DisWBumpPC(); }
    void AbsoluteX(std::stringstream& ss) { ss << DisWBumpPC() << ",X"; }
    void AbsoluteY(std::stringstream& ss) { ss << DisWBumpPC() << ",Y"; }
    void IndexedIndirectX(std::stringstream& ss) { ss << '(' << DisBBumpPC() << ',X)'; }
    void IndirectIndexedY(std::stringstream& ss) { ss << '(' << DisBBumpPC() << '),Y'; }

    // Instructions

#define PREPEND(instr) \
std::string address = ss.str(); \
ss.str(instr); \
ss << ' ' << address; 

#define INSTRUCTION(codeName, displayName) \
void codeName(std::stringstream& ss) { PREPEND(displayName); }

    // Loads
    INSTRUCTION(lda, "LDA")
    INSTRUCTION(ldx, "LDX")
    INSTRUCTION(ldy, "LDY")

    // Stores
    INSTRUCTION(sta, "STA")
    INSTRUCTION(stx, "STX")
    INSTRUCTION(sty, "STY")

    // Flag Operations
    void clc(std::stringstream& ss) { ss << "CLC";  }
    void sec(std::stringstream& ss) { ss << "SEC"; }
    void cli(std::stringstream& ss) { ss << "CLI"; }
    void sei(std::stringstream& ss) { ss << "SEI"; }
    void clv(std::stringstream& ss) { ss << "CLV"; }
    void cld(std::stringstream& ss) { ss << "CLD"; }
    void sed(std::stringstream& ss) { ss << "SED"; }
};