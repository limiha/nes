#pragma once

// This macro exisst so that the cpu and dissassembler can make use of the same
// op switch statment

#define DECODE(op) \
{ \
    switch (op) \
    { \
    /*loads*/ \
    case 0xa1: IndexedIndirectX();    lda();    break; \
    case 0xa5: ZeroPage();            lda();    break; \
    case 0xa9: Immediate();           lda();    break; \
    case 0xad: Absolute();            lda();    break; \
    case 0xb1: IndirectIndexedY();    lda();    break; \
    case 0xb5: ZeroPageX();           lda();    break; \
    case 0xb9: AbsoluteY();           lda();    break; \
    case 0xbd: AbsoluteX();           lda();    break; \
    \
    case 0xa2: Immediate();           ldx();    break; \
    case 0xa6: ZeroPage();            ldx();    break; \
    case 0xb6: ZeroPageY();           ldx();    break; \
    case 0xae: Absolute();            ldx();    break; \
    case 0xbe: AbsoluteY();           ldx();    break; \
    \
    case 0xa0: Immediate();           ldy();    break; \
    case 0xa4: ZeroPage();            ldy();    break; \
    case 0xb4: ZeroPageX();           ldy();    break; \
    case 0xac: Absolute();            ldy();    break; \
    case 0xbc: AbsoluteX();           ldy();    break; \
    \
    /*stores*/ \
    case 0x85: ZeroPage();            sta();    break; \
    case 0x95: ZeroPageX();           sta();    break; \
    case 0x8d: Absolute();            sta();    break; \
    case 0x9d: AbsoluteX();           sta();    break; \
    case 0x99: AbsoluteY();           sta();    break; \
    case 0x81: IndexedIndirectX();    sta();    break; \
    case 0x91: IndirectIndexedY();    sta();    break; \
    \
    case 0x86: ZeroPage();            stx();    break; \
    case 0x96: ZeroPageY();           stx();    break; \
    case 0x8e: Absolute();            stx();    break; \
    \
    case 0x84: ZeroPage();            sty();    break; \
    case 0x94: ZeroPageX();           sty();    break; \
    case 0x8c: Absolute();            sty();    break; \
    \
    /*arithmetic*/ \
    case 0x69: Immediate();           adc();    break; \
    case 0x65: ZeroPage();            adc();    break; \
    case 0x75: ZeroPageX();           adc();    break; \
    case 0x6d: Absolute();            adc();    break; \
    case 0x7d: AbsoluteX();           adc();    break; \
    case 0x79: AbsoluteY();           adc();    break; \
    case 0x61: IndexedIndirectX();    adc();    break; \
    case 0x71: IndirectIndexedY();    adc();    break; \
    \
    case 0xe9: Immediate();           sbc();    break; \
    case 0xe5: ZeroPage();            sbc();    break; \
    case 0xf5: ZeroPageX();           sbc();    break; \
    case 0xed: Absolute();            sbc();    break; \
    case 0xfd: AbsoluteX();           sbc();    break; \
    case 0xf9: AbsoluteY();           sbc();    break; \
    case 0xe1: IndexedIndirectX();    sbc();    break; \
    case 0xf1: IndirectIndexedY();    sbc();    break; \
    \
    /*comparisons*/ \
    case 0xc9: Immediate();           cmp();    break; \
    case 0xc5: ZeroPage();            cmp();    break; \
    case 0xd5: ZeroPageX();           cmp();    break; \
    case 0xcd: Absolute();            cmp();    break; \
    case 0xdd: AbsoluteX();           cmp();    break; \
    case 0xd9: AbsoluteY();           cmp();    break; \
    case 0xc1: IndexedIndirectX();    cmp();    break; \
    case 0xd1: IndirectIndexedY();    cmp();    break; \
    \
    case 0xe0: Immediate();           cpx();    break; \
    case 0xe4: ZeroPage();            cpx();    break; \
    case 0xec: Absolute();            cpx();    break; \
    \
    case 0xc0: Immediate();           cpy();    break; \
    case 0xc4: ZeroPage();            cpy();    break; \
    case 0xcc: Absolute();            cpy();    break; \
    \
    /*bitwise operations*/ \
    case 0x29: Immediate();           and();    break; \
    case 0x25: ZeroPage();            and();    break; \
    case 0x35: ZeroPageX();           and();    break; \
    case 0x2d: Absolute();            and();    break; \
    case 0x3d: AbsoluteX();           and();    break; \
    case 0x39: AbsoluteY();           and();    break; \
    case 0x21: IndexedIndirectX();    and();    break; \
    case 0x31: IndirectIndexedY();    and();    break; \
    \
    case 0x09: Immediate();           ora();    break; \
    case 0x05: ZeroPage();            ora();    break; \
    case 0x15: ZeroPageX();           ora();    break; \
    case 0x0d: Absolute();            ora();    break; \
    case 0x1d: AbsoluteX();           ora();    break; \
    case 0x19: AbsoluteY();           ora();    break; \
    case 0x01: IndexedIndirectX();    ora();    break; \
    case 0x11: IndirectIndexedY();    ora();    break; \
    \
    case 0x49: Immediate();           eor();    break; \
    case 0x45: ZeroPage();            eor();    break; \
    case 0x55: ZeroPageX();           eor();    break; \
    case 0x4d: Absolute();            eor();    break; \
    case 0x5d: AbsoluteX();           eor();    break; \
    case 0x59: AbsoluteY();           eor();    break; \
    case 0x41: IndexedIndirectX();    eor();    break; \
    case 0x51: IndirectIndexedY();    eor();    break; \
    \
    case 0x24: ZeroPage();            bit();    break; \
    case 0x2c: Absolute();            bit();    break; \
    \
    /*shifts and rotates*/ \
    case 0x2a: Accumulator();         rol();    break; \
    case 0x26: ZeroPage();            rol();    break; \
    case 0x36: ZeroPageX();           rol();    break; \
    case 0x2e: Absolute();            rol();    break; \
    case 0x3e: AbsoluteX();           rol();    break; \
    \
    case 0x6a: Accumulator();         ror();    break; \
    case 0x66: ZeroPage();            ror();    break; \
    case 0x76: ZeroPageX();           ror();    break; \
    case 0x6e: Absolute();            ror();    break; \
    case 0x7e: AbsoluteX();           ror();    break; \
    \
    case 0x0a: Accumulator();         asl();    break; \
    case 0x06: ZeroPage();            asl();    break; \
    case 0x16: ZeroPageX();           asl();    break; \
    case 0x0e: Absolute();            asl();    break; \
    case 0x1e: AbsoluteX();           asl();    break; \
    \
    case 0x4a: Accumulator();         lsr();    break; \
    case 0x46: ZeroPage();            lsr();    break; \
    case 0x56: ZeroPageX();           lsr();    break; \
    case 0x4e: Absolute();            lsr();    break; \
    case 0x5e: AbsoluteX();           lsr();    break; \
    \
    /*increments and decrements*/ \
    case 0xe6: ZeroPage();            inc();    break; \
    case 0xf6: ZeroPageX();           inc();    break; \
    case 0xee: Absolute();            inc();    break; \
    case 0xfe: AbsoluteX();           inc();    break; \
    \
    case 0xc6: ZeroPage();            dec();    break; \
    case 0xd6: ZeroPageX();           dec();    break; \
    case 0xce: Absolute();            dec();    break; \
    case 0xde: AbsoluteX();           dec();    break; \
    \
    case 0xe8: inx(); break; \
    case 0xca: dex(); break; \
    case 0xc8: iny(); break; \
    case 0x88: dey(); break; \
    \
    /*register moves*/ \
    case 0xaa: tax(); break; \
    case 0xa8: tay(); break; \
    case 0x8a: txa(); break; \
    case 0x98: tya(); break; \
    case 0x9a: txs(); break; \
    case 0xba: tsx(); break; \
    \
    /*flag operations*/ \
    case 0x18: clc(); break; \
    case 0x38: sec(); break; \
    case 0x58: cli(); break; \
    case 0x78: sei(); break; \
    case 0xb8: clv(); break; \
    case 0xd8: cld(); break; \
    case 0xf8: sed(); break; \
    \
    /*branches*/ \
    case 0x10: bpl(); break; \
    case 0x30: bmi(); break; \
    case 0x50: bvc(); break; \
    case 0x70: bvs(); break; \
    case 0x90: bcc(); break; \
    case 0xb0: bcs(); break; \
    case 0xd0: bne(); break; \
    case 0xf0: beq(); break; \
    \
    case 0x4c: jmp(); break; \
    case 0x6c: jmpi(); break; \
    \
    /*procedure calls*/ \
    case 0x20: jsr(); break; \
    case 0x60: rts(); break; \
    case 0x00: brk(); break; \
    case 0x40: rti(); break; \
    \
    /*stack operations*/ \
    case 0x48: pha(); break; \
    case 0x68: pla(); break; \
    case 0x08: php(); break; \
    case 0x28: plp(); break; \
    \
    /*no operation*/ \
    case 0xea: nop(); break; \
    \
    default: \
        printf("Unimplemented instruction: 0x%02x\n", op); \
        __debugbreak(); \
    } \
}