# Unit - 4

## Lab - 4 - 3 : IA32 Instruction Decode (40 pt)

### Problem Statement

The goal of this step is to use StartProfiling() to decode a block of instructions. You need only to decode enough of each instruction to determine its length. By doing this you should be able to decode a block of instructions of arbitrary length. StartProfiling() should print the address, opcode, and length of instructions for user_prog() until it encounters ret (0xc9) instruction.

Due to the complexity of Intel ISA being CISC, the core challenge for the part is to get the right length for each instruction. On the other hand, it is simpler than ARM architecture as it has limited set of instructions which make control (branch) operations. To help you on this, we provide IA32 opcode map in ia32DecodeTable. Use it as you see fit. Or if you find any instruction not covered by the map, please feel free to update the table.

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

### Write - up

- First I changed the struct `IA32Instr` to accomodate `isCflow` variable which would store integer 0 or 1 with which we would identify if the instruction is control flow instruction or not.
- `ia32Decode` function decodes each instruction based on the guidelines and structure of the instruction and increases the instruction length accordingly.
- `ia32DecodeTable` helps with getting which instruction will have how many bytes.

## Lab - 4 - 4: Control flow following and instruction counting (60 pt)

### Problem Statement

In this part, you should now have the tools to identify the control (or branch) instructions and follow the control flow of IA32 architecture. With this, you will extend lab4.c to implement the same binary patching / unpatching operations you did for the previous lab.
Again, decode the instructions to be executed until you hit a control flow instruction. Binary patch the instruction to call out to handler routines instead of doing the control flow. You can then return to the guest code knowing that you will be called before execution passes that point (i.e., patched control flow instruction). When your handler is called, unpatch the patched instruction, emulate the control behavior, and binary patch the control instruction at the end of the following basic block. For each basic block you encounter, record the instructions in that block. You should stop this process when you hit the StopProfiling() function. Create a data structure to capture the start address of each basic block executed, the instruction count, and the number of basic block execution. Run target program (user_prog()) with different inputs and check the number of instructions (and basic blocks) executions. At the end of the execution, you will output the list of (executed) basic blocks and their instruction count and number of executions.

### Write - up

- Use code of `Part-3` and modify it to patch the control flow instruction to its proper callout. After that unpatch it to its previous state and follow the control flow path.
- Each callout is handled differently. JccCallout would need to check if there would be jmp taken or not while for callCallout we would need to figure the offset to where we would call to.
- Take note of the basic blocks and print it in stop profiling.

## (Bonus) lab4-5: Control Flow Graph (CFG) generation (40 pt)

### Problem Statement

In this part, you will extend lab4-4 to trace the control flow transition among basic blocks. To achieve, you need to modify the internal data structure to trace to control flow among different basic blocks. The exemplary entry would look like the following.

`"(0x8049426 ~ 0x8049437)" -> "(0x8049402 ~ 0x804940b)" [label=88]`

Here, control flow transition happened from basic block (0x8049426 ~ 0x8049437) to (0x8049402 ~ 0x804940b) for 88 times during the execution. Your output should be formatted in DOT language format so that it can be visualized using graph-easy or dot command.

### Write - up

- Simply created a few global variables to store the src and dst block's start and end addresses.
  - I tried to store the direct basicblock inside the linkedlist node of a transition as srcBlock and destBlock but when i tried to print the addresses of those blocks it was giving me wacky address and I was a bit too sleepy to debug that. So used above method.
- Used linkedlist structure (similar to basic block) and created methods to create and update transitions
- In StopProfiling just added a code to record those transitions in cfg.dot file.
- Sample Output:

```
digraph G {
  "(nil) ~ (nil)" -> "0x8049609 ~ 0x8049612" [label=1];
  "0x8049609 ~ 0x8049612" -> "0x8049620 ~ 0x804962a" [label=2];
  "0x8049620 ~ 0x804962a" -> "0x8049609 ~ 0x8049612" [label=2];
  "0x8049609 ~ 0x8049612" -> "0x8049614 ~ 0x804961e" [label=3];
  "0x8049614 ~ 0x804961e" -> "0x804964c ~ 0x8049652" [label=3];
  "0x804964c ~ 0x8049652" -> "0x804962f ~ 0x8049640" [label=1];
  "0x804962f ~ 0x8049640" -> "0x8049609 ~ 0x8049612" [label=2];
  "0x804964c ~ 0x8049652" -> "0x8049645 ~ 0x8049652" [label=2];
  "0x8049645 ~ 0x8049652" -> "0x804962f ~ 0x8049640" [label=1];
}
```
