// This macro exisst so that the cpu and dissassembler can make use of the same
// op switch statment

#define DECODE(op) \
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
}