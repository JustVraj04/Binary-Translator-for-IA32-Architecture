#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include "macros.h"
#include "ia32_context.h"
#include "ia32_disas.h"

/* addresses of asm callout glue code */

extern void *jccCallout;
extern void *jmpCallout;
extern void *callCallout;
extern void *retCallout;

extern uint32_t ia32DecodeTable[]; /* see below */

/* instrumentation target */

extern int user_prog(void *a);

void StartProfiling(void *func);

void StopProfiling(void);

void ia32Decode(uint8_t *ptr, IA32Instr *instr);

void *callTarget;

#define DECODE_CHECK(_a, _b) (ia32DecodeTable[(_a)] & (_b))

/*********************************************************************
 *
 *  callout handlers
 *
 *   These get called by asm glue routines.
 *
 *********************************************************************/

void handleJccCallout(SaveRegs regs)
{
  NOT_IMPLEMENTED();
}

void handleJmpCallout(SaveRegs regs)
{
  NOT_IMPLEMENTED();
}

void handleCallCallout(SaveRegs regs)
{
  NOT_IMPLEMENTED();
}

void handleRetCallout(SaveRegs regs)
{
  NOT_IMPLEMENTED();
}

/*********************************************************************
 *
 *  ia32Decode
 *
 *   Decode an IA32 instruction.
 *
 *********************************************************************/
void ia32Decode(uint8_t *ptr, IA32Instr *instr)
{
  instr->len = 0;
  instr->isCflow = 0;
  int i = 0; // index variable

  // handle instruction prefixes
  while (DECODE_CHECK(ptr[i], IA32_PREFIX))
  {
    instr->len = instr->len + 1;
    i++;
  }

  // determine opcode length and set opcode
  if (DECODE_CHECK(ptr[i], IA32_2BYTE))
  {
    instr->len = instr->len + 2;
    instr->opcode = (uint16_t)ptr[i];
    i += 2;
  }
  else
  {
    instr->len = instr->len + 1;
    instr->opcode = (uint16_t)ptr[i];
    i++;
  }

  // check if the instruction is a control flow instruction
  if (DECODE_CHECK(instr->opcode, IA32_CFLOW))
  {
    instr->isCflow = 1;
  }

  // handle ModR/M byte if present
  if (DECODE_CHECK(instr->opcode, IA32_MODRM))
  {
    instr->len = instr->len + 1; // for the ModR/M byte
    uint8_t modrm = ptr[i];
    i++;

    if (BITS(modrm, 6, 7) != 0b11 && BITS(modrm, 0, 2) == 4) // check if SIB byte is present
    {
      instr->len = instr->len + 1;
      uint8_t sib = ptr[i];
      i++;

      if (BITS(modrm, 6, 7) == 0b00 && BITS(sib, 0, 2) == 5)
      {
        // disp32 follows
        instr->len = instr->len + 4;
        i += 4;
      }
      else if (BITS(modrm, 6, 7) == 0b01)
      {
        // disp8 follows
        instr->len = instr->len + 1;
        i++;
      }
      else if (BITS(modrm, 6, 7) == 0b10)
      {
        // disp32 follows
        instr->len = instr->len + 4;
        i += 4;
      }
    }
    else
    {
      switch (BITS(modrm, 6, 7))
      {
      case 0b00: // 0
        if (BITS(modrm, 0, 2) == 5)
        {
          instr->len = instr->len + 4;
          i += 4;
        }
        break;
      case 0b01: // 1
        instr->len = instr->len + 1;
        i++;
        break;
      case 0b10: // 2
        instr->len = instr->len + 4;
        i += 4;
        break;
      case 0b11: // 3
        // Register mode, no additional bytes
        break;
      }
    }
  }

  // handle immediate values if present
  if (DECODE_CHECK(instr->opcode, IA32_IMM8))
  {
    instr->len = instr->len + 1;
  }
  else if (DECODE_CHECK(instr->opcode, IA32_IMM32))
  {
    instr->len = instr->len + 4;
  }

  return;
}

/*********************************************************************
 *
 *  StartProfiling, StopProfiling
 *
 *   Profiling hooks. This is your place to inspect and modify the profiled
 *   function.
 *
 *********************************************************************/
void StartProfiling(void *func)
{
  int i = 0;

  while (1)
  {

    IA32Instr *inst = malloc(sizeof(IA32Instr));
    ia32Decode((func + i), inst);

    printf("addr: %p, opcode: %x, len: %d, isCFlow: %s\n", func + i, inst->opcode, inst->len, inst->isCflow ? "true" : "false");

    if (inst->opcode == 0xc3)
    {
      free(inst);
      break;
    }
    i += inst->len;
    free(inst);
  }
}

void StopProfiling(void)
{
}

int main(int argc, char *argv[])
{
  int value;
  char *end;

  char buf[16];

  if (argc != 1)
  {
    fprintf(stderr, "usage: %s\n", argv[0]);
    exit(1);
  }

#ifdef __FIB__
  printf("running fib()\n");
#endif

#ifdef __FIBP__
  printf("running fibp()\n");
#endif

#ifdef __PRIME__
  printf("running isPrime()\n");
#endif

  printf("input number: ");
  scanf("%15s", buf);

  value = strtol(buf, &end, 10);

  if (((errno == ERANGE) && ((value == LONG_MAX) || (value == LONG_MIN))) ||
      ((errno != 0) && (value == 0)))
  {
    perror("strtol");
    exit(1);
  }

  if (end == buf)
  {
    fprintf(stderr, "error: %s is not an integer\n", buf);
    exit(1);
  }

  if (*end != '\0')
  {
    fprintf(stderr, "error: junk at end of parameter: %s\n", end);
    exit(1);
  }

  StartProfiling(user_prog);

#if defined(__FIB__) || defined(__PRIME__)
  value = user_prog((void *)value);
#else
  value = user_prog(&value);
#endif

  StopProfiling();

  printf("%d\n", value);
  exit(0);
}
