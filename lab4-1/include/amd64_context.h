#progma once
/*********************************************************************
 *
 *  AMD64Instr
 *
 *   Program registers saved on a callout. These must be restored
 *   when we return to the program.
 *
 *     pc - this is the return address of the callout
 *     retPC - this is only valid if the callout replaced a RET
 *             instruction. This will be the return PC the ret
 *             will jump to.
 *
 *********************************************************************/

#include <stdint.h>
typedef struct {
   uint64_t   eflags;
   uint64_t   rsp;
   uint64_t   rdi;
   uint64_t   rsi;
   uint64_t   rbp;
   uint64_t   rbx;
   uint64_t   rdx;
   uint64_t   rcx;
   uint64_t   rax;
   uint64_t  r[8];
   void      *pc;
   void      *retPC;
} SaveRegs;

