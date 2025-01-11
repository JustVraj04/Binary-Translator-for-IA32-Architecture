# Unit - 4

## Lab - 4 - 1

### Problem Statement

The first step will get you warmed up and comfortable with code patching. Look at the bottom of main(). Just before main calls user_prog(), it calls StartProfiling() which is your hook. It allows you to inspect and/or modify the target function, user_prog() in this case before it starts executing. Your objective is to use StartProfiling() to binary patch user_prog() to return immediately. It gives you and opportunity to inspect and/or modify the target function, user_prog() in this case, before it starts executing. Your objective in Part1 is to use StartProfiling() to binary patch user_prog() to immediately return.

### Write - up

`StartProfiling` function changes the first instruction of the `user_prog` to `ret`(`0xc3`) instruction. Which makes the function return immediately without running its course.

## Lab - 4 - 2

### Problem Statement

In this step you should accomplish the same thing as lab4-1, but this time using a callout that emulates function return. The trickiness is that they need to save all the registers (EAX, EBX …) and condition registers (EFLAGS for Intel) because the code was not expecting a callout. The normal gcc calling conventions are not sufficient. Find the glue code in ia32_callout.S.

You should patch the first instruction on user_prog() with a callout. The callout should emulate the behavior of the function return behavior by returning not to the calling site of the callout (which is the normal behavior) but directly to the return PC on the stack. Please make sure store the original instructions before you overwrite.

[Hint] You will patch user_prog() to call glue code. What is the format for call instruction?

Try to add and run an arbitrary code from the callout context by replacing NOT_IMPLEMENTED() inside handleRetCallout() with something else. Finally, you need to ensure to call to visit StopProfiling() where you restore the original instruction of user_prog().

### Write - up

`StartProfiling` function changes the first instruction of the `user_prog` to `call retCallout`. Which makes the function call retCallOut which gets handled by handlRetCallout. I also store the original instructions in `StartProfiling` and put them back in `StopProfiling`.
