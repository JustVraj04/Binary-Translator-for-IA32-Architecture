#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "ia32_context.h"
#include "ia32_disas.h"
#include "macros.h"

/* addresses of asm callout glue code */

extern void *jccCallout;
extern void *jmpCallout;
extern void *callCallout;
extern void *retCallout;

extern uint32_t ia32DecodeTable[]; /* see below */

/* instrumentation target */

extern int user_prog(void *a);

void StartProfiling(void *func);

void StopProfiling(void *func);

void ia32Decode(uint8_t *ptr, IA32Instr *instr);

void *callTarget;

/*********************************************************************
 *
 *  callout handlers
 *
 *   These get called by asm glue routines.
 *
 *********************************************************************/

void handleJccCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

void handleJmpCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

void handleCallCallout(SaveRegs regs) { NOT_IMPLEMENTED(); }

void handleRetCallout(SaveRegs regs) { 
	printf("Be Aware!!! You are wondering in the depths of return callout!!\n");
	printf("EAX: %x\n", regs.eax);
	printf("EBX: %x\n", regs.ebx);
	printf("ECX: %x\n", regs.ecx);
}

/*********************************************************************
 *
 *  ia32Decode
 *
 *   Decode an IA32 instruction.
 *
 *********************************************************************/

void ia32Decode(uint8_t *ptr, IA32Instr *instr) { NOT_IMPLEMENTED(); }

/*********************************************************************
 *
 *  StartProfiling, StopProfiling
 *
 *   Profiling hooks. This is your place to inspect and modify the profiled
 *   function.
 *
 *********************************************************************/
uint8_t tmp[5];
void StartProfiling(void *func) {
	uint8_t *func_ptr = (uint8_t *)func;
	for (int i = 0; i < 5; i++){
		printf("%x ",func_ptr[i]);
		tmp[i] = func_ptr[i];
	}
	printf("\n");

	func_ptr[0] = 0xe8;
	uint32_t offset = (uint32_t)((uint8_t *)&retCallout - func_ptr - 5);
	*((uint32_t *)(func_ptr + 1)) = offset;
}

void StopProfiling(void *func) {
	printf("Running stop profilling.\n");
	uint8_t *func_ptr = (uint8_t *)func;
	for(int i = 0; i < 5; i++){
		func_ptr[i] = tmp[i];
		printf("%x ", func_ptr[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[]) {
  int value;
  char *end;

  char buf[16];

  if (argc != 1) {
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
      ((errno != 0) && (value == 0))) {
    perror("strtol");
    exit(1);
  }

  if (end == buf) {
    fprintf(stderr, "error: %s is not an integer\n", buf);
    exit(1);
  }

  if (*end != '\0') {
    fprintf(stderr, "error: junk at end of parameter: %s\n", end);
    exit(1);
  }

  StartProfiling(user_prog);

#if defined(__FIB__) || defined(__PRIME__)
  value = user_prog((void *)value);
#else
  value = user_prog(&value);
#endif

  StopProfiling(user_prog);

  printf("%d\n", value);
  exit(0);
}
