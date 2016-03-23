#pragma once

#include "mem.h"

#include <vector>

class DisassembledInstruction 
{
public:
    DisassembledInstruction() 
        : _ss(std::ios_base::out | std::ios_base::ate)
    { 
    }
    ~DisassembledInstruction() { }

    std::string GetFormattedBytes()
    {
        std::stringstream ss;
        ss << std::uppercase << std::hex;

        std::vector<u8>::iterator it = _bytes.begin();
        do
        {
            ss << std::setfill('0') << std::setw(2) << (i32)*it << ' ';
            it++;
        } while (it != _bytes.end());

        return ss.str();
    }

    std::string GetDisassemblyString()
    {
        return _ss.str();
    }

private:
    friend class Disassembler;
    std::stringstream _ss;
    std::vector<u8> _bytes;
};


class Disassembler
{
public:
    Disassembler(u16 PC, std::shared_ptr<IMem> mem);
    ~Disassembler();

    void Disassemble(DisassembledInstruction** disassembledInstruction);

private:
    u16 _PC;
    std::shared_ptr<IMem> _mem;

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

    u16 GetBranchTarget()
    {
        i8 disp = (i8)LoadBBumpPC();
        return (u16)((i32)_PC + (i32)disp);
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

    std::string DisBranchTarget()
    {
        std::stringstream ss;
        ss << '$' << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << (int)GetBranchTarget();
        return ss.str();
    }

    u8 PeekPC(int offset = 0)
    {
        return _mem->loadb(_PC + offset);
    }

    // Addressing Modes
    void Immediate(DisassembledInstruction* instr) 
    { 
        instr->_bytes.push_back(PeekPC());
        instr->_ss << '#' << DisBBumpPC(); 
    }

    void Accumulator(DisassembledInstruction* instr) { }

    void ZeroPage(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC());
        instr->_ss << DisBBumpPC(); 
    }

    void ZeroPageX(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC()); 
        instr->_ss << DisBBumpPC() << ",X"; 
    }

    void ZeroPageY(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC());
        instr->_ss << DisBBumpPC() << ",Y"; 
    }

    void Absolute(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC());
        instr->_bytes.push_back(PeekPC(1));
        instr->_ss << DisWBumpPC(); 
    }

    void AbsoluteX(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC());
        instr->_bytes.push_back(PeekPC(1));
        instr->_ss << DisWBumpPC() << ",X"; 
    }

    void AbsoluteY(DisassembledInstruction* instr)
    {
        instr->_bytes.push_back(PeekPC());
        instr->_bytes.push_back(PeekPC(1));
        instr->_ss << DisWBumpPC() << ",Y"; 
    }

    void IndexedIndirectX(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC());
        instr->_ss << '(' << DisBBumpPC() << ",X)"; 
    }

    void IndirectIndexedY(DisassembledInstruction* instr) 
    {
        instr->_bytes.push_back(PeekPC());
        instr->_ss << '(' << DisBBumpPC() << "),Y"; 
    }

    // Instructions
#define INSTRUCTION(codeName, displayName) \
void codeName(DisassembledInstruction* instr) \
{ \
    std::string address = instr->_ss.str(); \
    instr->_ss.str(displayName); \
    instr->_ss << ' ' << address; \
} 

#define IMPLIED(codeName, displayName) \
void codeName(DisassembledInstruction* instr) { \
    instr->_ss << displayName; \
}

#define BRANCH(codeName, displayName) \
void codeName(DisassembledInstruction* instr) \
{ \
    instr->_bytes.push_back(PeekPC()); \
    instr->_ss << displayName << ' ' << DisBranchTarget(); \
}

    // Loads
    INSTRUCTION(lda, "LDA")
    INSTRUCTION(ldx, "LDX")
    INSTRUCTION(ldy, "LDY")

    // Stores
    INSTRUCTION(sta, "STA")
    INSTRUCTION(stx, "STX")
    INSTRUCTION(sty, "STY")

    // Arithmetic
    INSTRUCTION(adc, "ADC")
    INSTRUCTION(sbc, "SBD")

    // Comparisons
    INSTRUCTION(cmp, "CMP")
    INSTRUCTION(cpx, "CPX")
    INSTRUCTION(cpy, "CPY")

    // Bitwise Operations
    INSTRUCTION(and, "AND")
    INSTRUCTION(ora, "ORA")
    INSTRUCTION(eor, "EOR")
    INSTRUCTION(bit, "BIT")

    // Shifts and Rotates
    INSTRUCTION(rol, "ROL")
    INSTRUCTION(ror, "ROR")
    INSTRUCTION(asl, "ASL")
    INSTRUCTION(lsr, "LSR")

    // Increments and Decrements
    INSTRUCTION(inc, "INC")
    INSTRUCTION(dec, "DEC")
    IMPLIED(inx, "INX")
    IMPLIED(dex, "DEX")
    IMPLIED(iny, "INY")
    IMPLIED(dey, "DEY")

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

    // Jumps
    void jmp(DisassembledInstruction* instr)
    {
        instr->_bytes.push_back(PeekPC());
        instr->_bytes.push_back(PeekPC(1));

        instr->_ss << "JMP " << DisWBumpPC();
    }

    void jmpi(DisassembledInstruction* instr)
    {
        instr->_bytes.push_back(PeekPC());
        instr->_bytes.push_back(PeekPC(1));

        u16 val = _mem->loadw(LoadWBumpPC());

        instr->_ss << "JMP ($" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << val << ')';
    }

    // Procedure Calls
    void jsr(DisassembledInstruction* instr)
    {
        instr->_bytes.push_back(PeekPC());
        instr->_bytes.push_back(PeekPC(1));

        instr->_ss << "JSR " << DisWBumpPC();
    }
    IMPLIED(rts, "RTS")
    IMPLIED(brk, "BRK")
    IMPLIED(rti, "RTI")

    // Stack Operations
    IMPLIED(pha, "PHA")
    IMPLIED(pla, "PLA")
    IMPLIED(php, "PHP")
    IMPLIED(plp, "PLP")

    // No Operation
    IMPLIED(nop, "NOP")
};