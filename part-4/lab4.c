#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "macros.h"
#include "ia32_context.h"
#include "ia32_disas.h"

/* addresses of asm callout glue code */

extern void *jccCallout;
extern void *jmpCallout;
extern void *callCallout;
extern void *retCallout;

extern uint32_t ia32DecodeTable[]; /* see below */

typedef struct BasicBlock
{
  void *startAddr;
  void *endAddr;
  int instrCount;
  int execCount;
  struct BasicBlock *next;
} BasicBlock;

BasicBlock *basic_blocks = NULL; // Head of the linked list

void *patchedAddr;
uint8_t control_buff_bytes[5];
void *nextPatchedAddr;
/* instrumentation target */

extern int user_prog(void *a);

void patchNextInstruction(void *addr);

void StartProfiling(void *func);

void StopProfiling(void);

void ia32Decode(uint8_t *ptr, IA32Instr *instr);

void *callTarget;

#define DECODE_CHECK(_a, _b) (ia32DecodeTable[(_a)] & (_b))

void RecordBasicBlock(void *startAddress, void *endAddress, uint32_t instruction_count)
{
  BasicBlock *current = basic_blocks;
  BasicBlock *prev = NULL;

  // Search for existing basic block
  while (current != NULL)
  {
    if (current->startAddr == startAddress)
    {
      current->execCount++;
      return;
    }
    prev = current;
    current = current->next;
  }

  // Create a new basic block
  BasicBlock *new_block = (BasicBlock *)malloc(sizeof(BasicBlock));
  if (new_block == NULL)
  {
    perror("Failed to allocate memory for new basic block");
    exit(EXIT_FAILURE);
  }
  new_block->startAddr = startAddress;
  new_block->endAddr = endAddress;
  new_block->instrCount = instruction_count;
  new_block->execCount = 1;
  new_block->next = NULL;

  // Insert into the linked list
  if (prev == NULL)
  {
    // Inserting the first element
    basic_blocks = new_block;
  }
  else
  {
    prev->next = new_block;
  }
}

void PrintBasicBlockStatistics(void)
{
  uint32_t totalBlocks = 0;
  uint64_t totalExecutions = 0;
  BasicBlock *current = basic_blocks;

  int count = 1;
  printf("\n--- Basic Block Statistics ---\n");
  while (current != NULL)
  {
    printf("Basic block #     %d: %p ~ %p contains    %u Instruction(s) and executed   %u times\n",
           count++,
           current->startAddr,
           current->endAddr,
           current->instrCount,
           current->execCount);
    totalBlocks++;
    totalExecutions += (current->execCount * current->instrCount);
    current = current->next;
  }

  // Print totals
  printf("\nTotal Basic Blocks: %u\n", totalBlocks);
  printf("Total Instructions Executed: %llu\n\n", totalExecutions);
}

// Don't mind me -- I was here to just try out malloc and relloc for arrays -- linkedlist rules
// void initializeBasicBlocks()
// {
//   basicBlocks = malloc(basicBlockArraySize * sizeof(BasicBlock));
//   if (basicBlocks == NULL)
//   {
//     perror("Failed to allocate memory for basic blocks");
//     exit(1);
//   }
// }
/*********************************************************************
 *
 *  callout handlers
 *
 *   These get called by asm glue routines.
 *
 *********************************************************************/

void handleJccCallout(SaveRegs regs)
{
  // Debugging
  // printf("Inside JCC Callout\n");
  memcpy(patchedAddr, control_buff_bytes, 5);

  int8_t jccOpcode = *(int8_t *)patchedAddr;
  int32_t eflags = regs.eflags;
  int jump_taken = 0;

  switch (jccOpcode)
  {
  case 0x70: // JO - Overflow
    jump_taken = (eflags & (1 << 11)) != 0;
    break;
  case 0x71: // JNO - Not Overflow
    jump_taken = (eflags & (1 << 11)) == 0;
    break;
  case 0x72: // JB - Below (CF=1)
    jump_taken = (eflags & (1 << 0)) != 0;
    break;
  case 0x73: // JAE - Above or Equal (CF=0)
    jump_taken = (eflags & (1 << 0)) == 0;
    break;
  case 0x74: // JE - Equal (ZF=1)
    jump_taken = (eflags & (1 << 6)) != 0;
    break;
  case 0x75: // JNE - Not Equal (ZF=0)
    jump_taken = (eflags & (1 << 6)) == 0;
    break;
  case 0x76: // JBE - Below or Equal (CF=1 or ZF=1)
    jump_taken = (eflags & (1 << 0)) || (eflags & (1 << 6));
    break;
  case 0x77: // JA - Above (CF=0 and ZF=0)
    jump_taken = !((eflags & (1 << 0)) || (eflags & (1 << 6)));
    break;
  case 0x78: // JS - Sign (SF=1)
    jump_taken = (eflags & (1 << 7)) != 0;
    break;
  case 0x79: // JNS - Not Sign (SF=0)
    jump_taken = (eflags & (1 << 7)) == 0;
    break;
  case 0x7A: // JP - Parity (PF=1)
    jump_taken = (eflags & (1 << 2)) != 0;
    break;
  case 0x7B: // JNP - Not Parity (PF=0)
    jump_taken = (eflags & (1 << 2)) == 0;
    break;
  case 0x7C: // JL - Less (SF!=OF)
    jump_taken = ((eflags & (1 << 7)) != 0) != ((eflags & (1 << 11)) != 0);
    break;
  case 0x7D: // JGE - Greater or Equal (SF=OF)
    jump_taken = ((eflags & (1 << 7)) != 0) == ((eflags & (1 << 11)) != 0);
    break;
  case 0x7E: // JLE - Less or Equal (ZF=1 or SF!=OF)
    jump_taken = (eflags & (1 << 6)) || (((eflags & (1 << 7)) != 0) != ((eflags & (1 << 11)) != 0));
    break;
  case 0x7F: // JG - Greater (ZF=0 and SF=OF)
    jump_taken = !(eflags & (1 << 6)) && (((eflags & (1 << 7)) != 0) == ((eflags & (1 << 11)) != 0));
    break;
  }

  int8_t offset = *(int8_t *)(patchedAddr + 1);
  void *jump_target = nextPatchedAddr + offset;

  if (jump_taken)
  {
    regs.pc = jump_target;
  }
  else
  {
    regs.pc = nextPatchedAddr;
  }
  patchNextInstruction(regs.pc);
  // NOT_IMPLEMENTED();
}

void handleJmpCallout(SaveRegs regs)
{
  // printf("Inside JMP callout\n");
  memcpy(patchedAddr, control_buff_bytes, 5);

  // uint8_t *jmpOpcode = (uint8_t *)patchedAddr;
  // int32_t displacement = 0;
  // printf("%x", jmpOpcode[0]);
  void *nextInstr = nextPatchedAddr + control_buff_bytes[1];
  regs.pc = nextInstr;
  patchNextInstruction(nextInstr);
  // NOT_IMPLEMENTED();
}

void handleCallCallout(SaveRegs regs)
{
  // printf("Inside Call CALLOUT\n");
  memcpy(patchedAddr, control_buff_bytes, 5);

  int8_t *callOpcode = (int8_t *)patchedAddr;
  int32_t offset = 0;

  memcpy(&offset, control_buff_bytes + 1, 4);
  callTarget = nextPatchedAddr + offset;

  // if (callTarget != &StopProfiling)
  // {
  //   patchNextInstruction(callTarget);
  // }

  patchNextInstruction(callTarget);
  // NOT_IMPLEMENTED();
}

void handleRetCallout(SaveRegs regs)
{
  // printf("Inside RET Callout\n");
  memcpy(patchedAddr, control_buff_bytes, 5);

  if ((int32_t)user_prog >= (int32_t)regs.retPC)
  {
    // printf("RET inside StopProfiling\n");
    return;
  }
  patchNextInstruction(regs.retPC);
  // NOT_IMPLEMENTED();
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

void patchNextInstruction(void *func)
{
  int i = 0;
  int flag = 0;
  int instr_count = 0;
  while (!flag)
  {
    IA32Instr *inst = malloc(sizeof(IA32Instr));
    ia32Decode((func + i), inst);
    ++instr_count;

    // Part of part - 3
    //  printf("addr: %p, opcode: %x, len: %d, isCFlow: %s\n", func + i, inst->opcode, inst->len, inst->isCflow ? "true" : "false");
    if (inst->isCflow == 1)
    {
      RecordBasicBlock((func), (func + i), instr_count);
      patchedAddr = (func + i);
      nextPatchedAddr = (func + i) + inst->len;
      memcpy(control_buff_bytes, patchedAddr, 5);
      int32_t *callptr;

      if (DECODE_CHECK(inst->opcode, IA32_RET))
      {

        callptr = (int32_t *)&retCallout;
      }
      else if (DECODE_CHECK(inst->opcode, IA32_JCC))
      {

        callptr = (int32_t *)&jccCallout;
      }
      else if (DECODE_CHECK(inst->opcode, IA32_JMP))
      {

        callptr = (int32_t *)&jmpCallout;
      }
      else if (DECODE_CHECK(inst->opcode, IA32_CALL))
      {

        callptr = (int32_t *)&callCallout;
      }

      int32_t offset = (int32_t)callptr - ((int32_t)patchedAddr + 5);
      // printf("offset: %x\n", offset);

      // printf("%p\n", (func + i));
      uint8_t *current = (func + i);

      current[0] = 0xe8;
      // printf("%p\n", callptr);
      memcpy(current + 1, &offset, 4);

      // Debugging
      // printf("%x %x %x %x %x\n", current[0], current[1], current[2], current[3], current[4]);
      // if (inst->opcode == 0xc3)
      // {
      //   free(inst);
      //   break;
      // }
      flag = 1;
    }
    i += inst->len;
    free(inst);
  }
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
  // initializeBasicBlocks();
  patchNextInstruction(func);
}

void StopProfiling(void)
{
  PrintBasicBlockStatistics();
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
