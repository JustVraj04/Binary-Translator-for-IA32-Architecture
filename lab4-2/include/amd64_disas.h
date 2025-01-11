#progma once

#include <stdint.h>

#define AMD64_DATA            0       // default
#define AMD64_notimpl         1
#define AMD64_PREFIX          2
#define AMD64_2BYTE           3
#define AMD64_CFLOW           4
#define AMD64_DECODE_TYPE(_d) ((_d) & 0x000f)

/*
 *  The following defines add extra information on top of the decode type.
 */

#define AMD64_MODRM           0x0010
#define AMD64_IMM8            0x0020   // REL8 also
#define AMD64_IMM32           0x0040   // REL32 also

#define AMD64_RET             0x0100
#define AMD64_JCC             0x0200
#define AMD64_JMP             0x0400
#define AMD64_CALL            0x0800

extern unsigned ia32DecodeTable[]; /* see below */

/*********************************************************************
 *
 *  AMD64Instr
 *
 *   Decoded information about a single ia32 instruction.
 *
 *********************************************************************/

typedef struct {
   uint16_t  opcode;
   uint8_t   len;
   unsigned  modRM;
   uint32_t  imm;
} AMD64Instr;
