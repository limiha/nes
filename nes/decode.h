#pragma once

// This macro exisst so that the cpu and dissassembler can make use of the same
// op switch statment

#define DECODE(op) \
{\
    switch (op) \
    { \
    /*loads*/ \
    case 0xa1: IndexedIndirectX(am);    lda(am);    break; \
    case 0xa5: ZeroPage(am);            lda(am);    break; \
    case 0xa9: Immediate(am);           lda(am);    break; \
    case 0xad: Absolute(am);            lda(am);    break; \
    case 0xb1: IndirectIndexedY(am);    lda(am);    break; \
    case 0xb5: ZeroPageX(am);           lda(am);    break; \
    case 0xb0: AbsoluteY(am);           lda(am);    break; \
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
    /*flag operations*/ \
    case 0x18: clc(am); break; \
    case 0x38: sec(am); break; \
    case 0x58: cli(am); break; \
    case 0x78: sei(am); break; \
    case 0xb8: clv(am); break; \
    case 0xd8: cld(am); break; \
    case 0xf8: sed(am); break; \
    \
    default: \
        printf("Unimplemented instruction: 0x%02x\n", op); \
        __debugbreak; \
    }\
}