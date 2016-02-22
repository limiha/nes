#pragma once

// This macro exisst so that the cpu and dissassembler can make use of the same
// op switch statment

#define DECODE(op) \
{\
    switch (op) \
    { \
    /*loads*/ \
    case 0xa1: IndirectIndexedX(am);    lda(am);    break; \
    case 0xa5: ZeroPage(am);            lda(am);    break; \
    case 0xa9: Immediate(am);           lda(am);    break; \
    case 0xad: Absolute(am);            lda(am);    break; \
    case 0xb1: IndirectIndexedY(am);    lda(am);    break; \
    case 0xb5: ZeroPageX(am);           lda(am);    break; \
    case 0xb0: AbsoluteY(am);           lda(am);    break; \
    case 0xbd: AbsoluteX(am);           lda(am);    break; \
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