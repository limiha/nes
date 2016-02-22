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
        ss << '$' << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)LoadBBumpPC();
        return ss.str();
    }

    std::string DisWBumpPC()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << (int)LoadWBumpPC();
        return ss.str();
    }

    // Addressing Modes
    void Immediate(std::stringstream& ss) { ss << '#' << DisBBumpPC(); }
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

#define IMPLIED(codeName, displayName) \
void codeName(std::stringstream& ss) { ss << displayName; }

#define BRANCH(codeName, displayName) \
void codeName(std::stringstream& ss) { ss << displayName << ' ' << DisBBumpPC(); }

    // Loads
    INSTRUCTION(lda, "LDA")
    INSTRUCTION(ldx, "LDX")
    INSTRUCTION(ldy, "LDY")

    // Stores
    INSTRUCTION(sta, "STA")
    INSTRUCTION(stx, "STX")
    INSTRUCTION(sty, "STY")

    // Register Moves
    IMPLIED(tax, "TAX")
    IMPLIED(tay, "TAY")
    IMPLIED(txa, "TXA")
    IMPLIED(tya, "TYA")
    IMPLIED(txs, "TXS")
    IMPLIED(tsx, "TSX")

    // Flag Operations
    IMPLIED(clc, "CLC")
    IMPLIED(sec, "SEC")
    IMPLIED(cli, "CLI")
    IMPLIED(sei, "SEI")
    IMPLIED(clv, "CLV")
    IMPLIED(cld, "CLD")
    IMPLIED(sed, "SED")

    // Branches
    BRANCH(bpl, "BPL")
    BRANCH(bmi, "BMI")
    BRANCH(bvc, "BVC")
    BRANCH(bvs, "BVS")
    BRANCH(bcc, "BCC")
    BRANCH(bcs, "BCS")
    BRANCH(bne, "BNE")
    BRANCH(beq, "BEQ")
};