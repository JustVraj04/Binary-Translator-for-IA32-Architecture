# CS6332: Lab 4 - IA32 Binary Translator (160 + 40 points)

**NOTE:** You will connect to and use `ctf-vm2.utdallas.edu` for this assignment.

In this homework you will implement a basic form of [binary translator] for Intel architecture that would instrument [single][isPrime.c] [argument][fib.c] [functions][fibp.c]. The main focus is in three folds. We want to learn *(1)* basic principles of software-based virtualization and instrumentation techniques, *(2)* CISC instruction decoding and disassembly techniques Intel architecture (i386/AMD64), and (3) profiling and monitoring the guest code behavior by instrumenting [inline monitors] on-the-fly.

The unit provides [skeleton codes] to implement a minimal binary translator for Intel architecture. In summary, your implementation will (1) Intel instruction decoder decode instruction, (2) patch the branch (or control) instruction to switch into callout context, (3) profile program execution at runtime, and (4) apply an optimization.

Although the assignment is tested and verified for GCC compilers in ctf-vm2 (version 5.4 and 7.3), the other environmental factors may affect compilers to generate different machine instruction sequences, resulting in runtime failures. Please feel free to consult the instructor or TA regarding such issues.

## Preliminaries

Again, you will begin the lab by building and running the provided skeleton code. Then, you will incrementally extend it in the course of completing each part. Two recursive functions with no [side effects] ([Fibonacci] and [isPrime][isPrime.c]), written in plain C, are provided to test your binary translator. Fibonacci comes in with two different implementations, recursive ([fib.c]) and iterative ([fibp.c]) versions.

Both functions have signatures that take a single integer argument, return an integer with no side effects. Inside the main driver code ([lab4.c]), all functions are alias as `int64_t user_prog(void *)`. While all functions take a single argument, we need to support two different ways to pass the argument(s). [fib.c] takes an integer argument and returns an integer value. [fibp.c] takes input in pointer type to support variable-length arguments. We use the `void*` type to indicate generic input types.

To test each function, first, copy the template code located in `/home/labs/unit4.zip` from your ctf-vm2 and run the following.

```bash
# Fibonacci functions.
$ make fib
$ ./lab4_fib

$ make fibp
$ ./lab4_fibp

$ make prime
$ ./lab4_prime
```

### Submission guideline.

This assignment will have two deadlines. `lab4-1` and `lab4-2` be first due at 11:59 PM, Nov 7 and `lab4-3` and the rest `lab4-4` and `lab4-5` will be due at 11:59 PM, Nov 19. To submit the assignment, you will extend the provided skeleton codes to implement a solution for each part. Please do not forget to write a README.md document to explain your code and
solution.

* Unit4-Part1
```
<netid>-lab04/
├── lab4-1/
├── lab4-2/
└── README.md
```

* Unit4-Part2
```
<netid>-lab04/
├── lab4-3/
├── lab4-4/
├── lab4-5/
└── README.md
```

<!--

### Academic honesty

The submission and code should be your own. Although strongly encouraging discussions among students and with the instructor or TA, the class will not tolerate students' failure to comply with [the Student code of conduct] or any unethical behaviors. The submitted code should be of your own. We will run code similarity measurement tools against all student submissions.

-->

----
## lab4-1: Patching binary to return (30 pt)

The first step will get you warmed up and comfortable with code patching. Look at the bottom of `main()`. Just before main calls `user_prog()`, it calls `StartProfiling()` which is your hook. It allows you to inspect and/or modify the target function, `user_prog()` in this case before it starts executing. Your objective is to use `StartProfiling()` to binary patch `user_prog()` to return immediately.  It gives you and opportunity to inspect and/or modify the target function, `user_prog()` in this case, before it starts  executing. Your objective in Part1 is to use [StartProfiling()] to binary patch `user_prog()` to *immediately return*.


## lab4-2: Callout and return (30 pt)

In this step you should accomplish the same thing as lab4-1, but this time using [a callout][glue-IA32] that emulates function return. The trickiness is that they need to save all the registers (`EAX`, `EBX` ...) and condition registers (`EFLAGS` for Intel) because the code was not expecting a callout. The normal `gcc` calling conventions are not sufficient. Find the glue code in [ia32_callout.S][glue-IA32].

You should *patch* the first instruction on `user_prog()` with a callout. The callout should emulate the behavior of the function return behavior by returning not to the calling site of the callout (which is the normal behavior) but directly to the return `PC` on the stack.

:::info
**[Hint]** You will patch *user_prog()* to call [glue code][glue-IA32]. What is the format for `call` instruction?
:::

Try to add and run an arbitrary code from the callout context by replacing *NOT_IMPLEMENTED()* inside *[handleRetCallout()]* with something else.

----
## lab4-3: IA32 Instruction Decode (40 pt)

The goal of this step is to use *[StartProfiling()]* to decode a block of instructions. You need only to decode enough of each instruction to determine its length. By doing this you should be able to decode a block of instructions of arbitrary length. *[StartProfiling()]* should print the address, opcode, and length of instructions for *[user_prog()]* until it encounters `ret` (0xc9) instruction.

Due to the complexity of Intel ISA being CISC, the core challenge for the part is to get the right length for each instruction. On the other hand, it is simpler than ARM architecture as it has limited set of instructions which make control (branch) operations. To help you on this, we provide IA32 opcode map in *[ia32DecodeTable]*. Use it as you see fit. Or if you find any instruction not covered by the map, please feel free to update the table.

The following is the sample output.

```
input number: 10
addr 0x8049310, opcode: 55, len: 1, isCFlow: false
addr 0x8049311, opcode: 89, len: 2, isCFlow: false
addr 0x8049313, opcode: 57, len: 1, isCFlow: false
addr 0x8049314, opcode: 56, len: 1, isCFlow: false
addr 0x8049315, opcode: 53, len: 1, isCFlow: false
addr 0x8049316, opcode: 51, len: 1, isCFlow: false
addr 0x8049317, opcode: 8d, len: 3, isCFlow: false
addr 0x804931a, opcode: 89, len: 2, isCFlow: false
addr 0x804931c, opcode: 83, len: 3, isCFlow: false
addr 0x804931f, opcode: 77, len: 2, isCFlow: true
addr 0x8049321, opcode: 8b, len: 2, isCFlow: false
addr 0x8049323, opcode: ba, len: 5, isCFlow: false
addr 0x8049328, opcode: eb, len: 2, isCFlow: true
addr 0x804932a, opcode: 8b, len: 2, isCFlow: false
addr 0x804932c, opcode: 83, len: 3, isCFlow: false
addr 0x804932f, opcode: 50, len: 1, isCFlow: false
addr 0x8049330, opcode: e8, len: 5, isCFlow: true
addr 0x8049335, opcode: 83, len: 3, isCFlow: false
addr 0x8049338, opcode: 89, len: 2, isCFlow: false
addr 0x804933a, opcode: 89, len: 2, isCFlow: false
addr 0x804933c, opcode: 8b, len: 2, isCFlow: false
addr 0x804933e, opcode: 83, len: 3, isCFlow: false
addr 0x8049341, opcode: 50, len: 1, isCFlow: false
addr 0x8049342, opcode: e8, len: 5, isCFlow: true
addr 0x8049347, opcode: 83, len: 3, isCFlow: false
addr 0x804934a, opcode: 1, len: 2, isCFlow: false
addr 0x804934c, opcode: 11, len: 2, isCFlow: false
addr 0x804934e, opcode: 8d, len: 3, isCFlow: false
addr 0x8049351, opcode: 59, len: 1, isCFlow: false
addr 0x8049352, opcode: 5b, len: 1, isCFlow: false
addr 0x8049353, opcode: 5e, len: 1, isCFlow: false
addr 0x8049354, opcode: 5f, len: 1, isCFlow: false
addr 0x8049355, opcode: 5d, len: 1, isCFlow: false
addr 0x8049356, opcode: c3, len: 1, isCFlow: true
```

## lab4-4: Control flow following and instruction counting (60 pt)

In this part, you should now have the tools to identify the control (or branch) instructions and follow the control flow of IA32 architecture. With this, you will extend [lab4.c] to implement the same binary patching / unpatching operations you did for the previous lab.
Again, decode the instructions to be executed until you hit a control flow instruction. Binary patch the instruction to call out to handler routines instead of doing the control flow. You can then return to the guest code knowing that you will be called before execution passes that point (i.e., patched control flow instruction). When your handler is called, unpatch the patched instruction, emulate the control behavior, and binary patch the control instruction at the end of the following basic block. For each basic block you encounter, record the instructions in that block. You should stop this process when you hit the [StopProfiling()] function. Create a data structure to capture the start address of each basic block executed, the instruction count, and the number of basic block execution. Run target program  (`user_prog()`) with different inputs and check the number of instructions (and basic blocks) executions. At the end of the execution, you will output the list of (executed) basic blocks and their instruction count and number of executions.

## (Bonus) lab4-5: Control Flow Graph (CFG) generation (40 pt)

In this part, you will extend lab4-4 to trace the control flow transition among basic blocks. To achieve, you need to modify the internal data structure to trace to control flow among different basic blocks. The exemplary entry would look like the following.

```
"(0x8049426 ~ 0x8049437)" -> "(0x8049402 ~ 0x804940b)" [label=88]
```

Here, control flow transition happened from basic block (0x8049426 ~ 0x8049437) to (0x8049402 ~ 0x804940b) for 88 times during the execution. Your output should be formatted in [DOT language] format so that it can be visualized using [graph-easy] or [dot] command.

## Resources

You may find this reference helpful for PC assembly language programming. You will need the [Intel IA32 manuals] for exact instruction formats and decoding rules. You can find them here:
* [Volume 1]
* [Volume 2]


[binary translator]:https://dl.acm.org/doi/10.1145/3321705.3329819
[skeleton codes]:https://files.syssec.org/lab4.zip
[side effects]:https://en.wikipedia.org/wiki/Side_effect_(computer_science)
[Fibonacci]:https://en.wikipedia.org/wiki/Fibonacci_number
[code]:https://theory.stanford.edu/~aiken/moss/
[similarity]:https://en.wikipedia.org/wiki/Content_similarity_detection
[measurement]:https://github.com/genchang1234/How-to-cheat-in-computer-science-101
[tools]:https://ieeexplore.ieee.org/document/5286623
[optimized]:https://www.sciencedirect.com/topics/computer-science/conditional-execution
[inline monitors]:https://files.syssec.org/0907-mm.pdf

[Intel IA32 manuals]:https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
[Volume 1]:https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-1-manual.pdf
[Volume 2]:https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf

[lab4.c]:lab4.c
[fibp.c]:fibp.c
[isPrime.c]:isPrime.c
[fib.c]:fib.c
[user_prog()]:lab4.c#L21
[StartProfiling()]:lab4.c#L66
[StopProfiling()]:lab4.c#L68
[ia32DecodeTable]:ia32_disas.c
[memoization]:https://en.wikipedia.org/wiki/Memoization
[handleRetCallout()]:ia32_callout.S#L30

[glue-IA32]:ia32_callout.S
[graphviz]:https://graphviz.org/
[DOT Language]:https://graphviz.org/doc/info/lang.html
[graph-easy]:https://github.com/ironcamel/Graph-Easy
[dot]:https://graphviz.org/doc/info/command.html
