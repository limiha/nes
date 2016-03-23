#pragma once

// This macro exisst so that the cpu and dissassembler can make use of the same
// op switch statment

#define DECODE(op) \
{ \
    switch (op) \
    { \
    /*loads*/ \
    case 0xa1: IndexedIndirectX(am);    lda(am);    break; \
    case 0xa5: ZeroPage(am);            lda(am);    break; \
    case 0xa9: Immediate(am);           lda(am);    break; \
    case 0xad: Absolute(am);            lda(am);    break; \
    case 0xb1: IndirectIndexedY(am);    lda(am);    break; \
    case 0xb5: ZeroPageX(am);           lda(am);    break; \
    case 0xb9: AbsoluteY(am);           lda(am);    break; \
    case 0xbd: AbsoluteX(am);           lda(am);    break; \
    \
    case 0xa2: Immediate(am);           ldx(am);    break; \
    case 0xa6: ZeroPage(am);            ldx(am);    break; \
    case 0xb6: ZeroPageY(am);           ldx(am);    break; \
    case 0xae: Absolute(am);            ldx(am);    break; \
    case 0xbe: AbsoluteY(am);           ldx(am);    break; \
    \
    case 0xa0: Immediate(am);           ldy(am);    break; \
    case 0xa4: ZeroPage(am);            ldy(am);    break; \
    case 0xb4: ZeroPageX(am);           ldy(am);    break; \
    case 0xac: Absolute(am);            ldy(am);    break; \
    case 0xbc: AbsoluteX(am);           ldy(am);    break; \
    \
    /*stores*/ \
    case 0x85: ZeroPage(am);            sta(am);    break; \
    case 0x95: ZeroPageX(am);           sta(am);    break; \
    case 0x8d: Absolute(am);            sta(am);    break; \
    case 0x9d: AbsoluteX(am);           sta(am);    break; \
    case 0x99: AbsoluteY(am);           sta(am);    break; \
    case 0x81: IndexedIndirectX(am);    sta(am);    break; \
    case 0x91: IndirectIndexedY(am);    sta(am);    break; \
    \
    case 0x86: ZeroPage(am);            stx(am);    break; \
    case 0x96: ZeroPageY(am);           stx(am);    break; \
    case 0x8e: Absolute(am);            stx(am);    break; \
    \
    case 0x84: ZeroPage(am);            sty(am);    break; \
    case 0x94: ZeroPageX(am);           sty(am);    break; \
    case 0x8c: Absolute(am);            sty(am);    break; \
    \
    /*arithmetic*/ \
    case 0x69: Immediate(am);           adc(am);    break; \
    case 0x65: ZeroPage(am);            adc(am);    break; \
    case 0x75: ZeroPageX(am);           adc(am);    break; \
    case 0x6d: Absolute(am);            adc(am);    break; \
    case 0x7d: AbsoluteX(am);           adc(am);    break; \
    case 0x79: AbsoluteY(am);           adc(am);    break; \
    case 0x61: IndexedIndirectX(am);    adc(am);    break; \
    case 0x71: IndirectIndexedY(am);    adc(am);    break; \
    \
    case 0xe9: Immediate(am);           sbc(am);    break; \
    case 0xe5: ZeroPage(am);            sbc(am);    break; \
    case 0xf5: ZeroPageX(am);           sbc(am);    break; \
    case 0xed: Absolute(am);            sbc(am);    break; \
    case 0xfd: AbsoluteX(am);           sbc(am);    break; \
    case 0xf9: AbsoluteY(am);           sbc(am);    break; \
    case 0xe1: IndexedIndirectX(am);    sbc(am);    break; \
    case 0xf1: IndirectIndexedY(am);    sbc(am);    break; \
    \
    /*comparisons*/ \
    case 0xc9: Immediate(am);           cmp(am);    break; \
    case 0xc5: ZeroPage(am);            cmp(am);    break; \
    case 0xd5: ZeroPageX(am);           cmp(am);    break; \
    case 0xcd: Absolute(am);            cmp(am);    break; \
    case 0xdd: AbsoluteX(am);           cmp(am);    break; \
    case 0xd9: AbsoluteY(am);           cmp(am);    break; \
    case 0xc1: IndexedIndirectX(am);    cmp(am);    break; \
    case 0xd1: IndirectIndexedY(am);    cmp(am);    break; \
    \
    case 0xe0: Immediate(am);           cpx(am);    break; \
    case 0xe4: ZeroPage(am);            cpx(am);    break; \
    case 0xec: Absolute(am);            cpx(am);    break; \
    \
    case 0xc0: Immediate(am);           cpy(am);    break; \
    case 0xc4: ZeroPage(am);            cpy(am);    break; \
    case 0xcc: Absolute(am);            cpy(am);    break; \
    \
    /*bitwise operations*/ \
    case 0x29: Immediate(am);           and(am);    break; \
    case 0x25: ZeroPage(am);            and(am);    break; \
    case 0x35: ZeroPageX(am);           and(am);    break; \
    case 0x2d: Absolute(am);            and(am);    break; \
    case 0x3d: AbsoluteX(am);           and(am);    break; \
    case 0x39: AbsoluteY(am);           and(am);    break; \
    case 0x21: IndexedIndirectX(am);    and(am);    break; \
    case 0x31: IndirectIndexedY(am);    and(am);    break; \
    \
    case 0x09: Immediate(am);           ora(am);    break; \
    case 0x05: ZeroPage(am);            ora(am);    break; \
    case 0x15: ZeroPageX(am);           ora(am);    break; \
    case 0x0d: Absolute(am);            ora(am);    break; \
    case 0x1d: AbsoluteX(am);           ora(am);    break; \
    case 0x19: AbsoluteY(am);           ora(am);    break; \
    case 0x01: IndexedIndirectX(am);    ora(am);    break; \
    case 0x11: IndirectIndexedY(am);    ora(am);    break; \
    \
    case 0x49: Immediate(am);           eor(am);    break; \
    case 0x45: ZeroPage(am);            eor(am);    break; \
    case 0x55: ZeroPageX(am);           eor(am);    break; \
    case 0x4d: Absolute(am);            eor(am);    break; \
    case 0x5d: AbsoluteX(am);           eor(am);    break; \
    case 0x59: AbsoluteY(am);           eor(am);    break; \
    case 0x41: IndexedIndirectX(am);    eor(am);    break; \
    case 0x51: IndirectIndexedY(am);    eor(am);    break; \
    \
    case 0x24: ZeroPage(am);            bit(am);    break; \
    case 0x2c: Absolute(am);            bit(am);    break; \
    \
    /*shifts and rotates*/ \
    case 0x2a: Accumulator(am);         rol(am);    break; \
    case 0x26: ZeroPage(am);            rol(am);    break; \
    case 0x36: ZeroPageX(am);           rol(am);    break; \
    case 0x2e: Absolute(am);            rol(am);    break; \
    case 0x3e: AbsoluteX(am);           rol(am);    break; \
    \
    case 0x6a: Accumulator(am);         ror(am);    break; \
    case 0x66: ZeroPage(am);            ror(am);    break; \
    case 0x76: ZeroPageX(am);           ror(am);    break; \
    case 0x6e: Absolute(am);            ror(am);    break; \
    case 0x7e: AbsoluteX(am);           ror(am);    break; \
    \
    case 0x0a: Accumulator(am);         asl(am);    break; \
    case 0x06: ZeroPage(am);            asl(am);    break; \
    case 0x16: ZeroPageX(am);           asl(am);    break; \
    case 0x0e: Absolute(am);            asl(am);    break; \
    case 0x1e: AbsoluteX(am);           asl(am);    break; \
    \
    case 0x4a: Accumulator(am);         lsr(am);    break; \
    case 0x46: ZeroPage(am);            lsr(am);    break; \
    case 0x56: ZeroPageX(am);           lsr(am);    break; \
    case 0x4e: Absolute(am);            lsr(am);    break; \
    case 0x5e: AbsoluteX(am);           lsr(am);    break; \
    \
    /*increments and decrements*/ \
    case 0xe6: ZeroPage(am);            inc(am);    break; \
    case 0xf6: ZeroPageX(am);           inc(am);    break; \
    case 0xee: Absolute(am);            inc(am);    break; \
    case 0xfe: AbsoluteX(am);           inc(am);    break; \
    \
    case 0xc6: ZeroPage(am);            dec(am);    break; \
    case 0xd6: ZeroPageX(am);           dec(am);    break; \
    case 0xce: Absolute(am);            dec(am);    break; \
    case 0xde: AbsoluteX(am);           dec(am);    break; \
    \
    case 0xe8: inx(am); break; \
    case 0xca: dex(am); break; \
    case 0xc8: iny(am); break; \
    case 0x88: dey(am); break; \
    \
    /*register moves*/ \
    case 0xaa: tax(am); break; \
    case 0xa8: tay(am); break; \
    case 0x8a: txa(am); break; \
    case 0x98: tya(am); break; \
    case 0x9a: txs(am); break; \
    case 0xba: tsx(am); break; \
    \
    /*flag operations*/ \
    case 0x18: clc(am); break; \
    case 0x38: sec(am); break; \
    case 0x58: cli(am); break; \
    case 0x78: sei(am); break; \
    case 0xb8: clv(am); break; \
    case 0xd8: cld(am); break; \
    case 0xf8: sed(am); break; \
    \
    /*branches*/ \
    case 0x10: bpl(am); break; \
    case 0x30: bmi(am); break; \
    case 0x50: bvc(am); break; \
    case 0x70: bvs(am); break; \
    case 0x90: bcc(am); break; \
    case 0xb0: bcs(am); break; \
    case 0xd0: bne(am); break; \
    case 0xf0: beq(am); break; \
    \
    case 0x4c: jmp(am); break; \
    case 0x6c: jmpi(am); break; \
    \
    /*procedure calls*/ \
    case 0x20: jsr(am); break; \
    case 0x60: rts(am); break; \
    case 0x00: brk(am); break; \
    case 0x40: rti(am); break; \
    \
    /*stack operations*/ \
    case 0x48: pha(am); break; \
    case 0x68: pla(am); break; \
    case 0x08: php(am); break; \
    case 0x28: plp(am); break; \
    \
    /*no operation*/ \
    case 0xea: nop(am); break; \
    \
    default: \
        printf("Unimplemented instruction: 0x%02x\n", op); \
        __debugbreak(); \
    } \
}