#include <stdint.h>
#include <stdio.h>

uint64_t factorial(uint32_t n)
{
    if (n == 0)
    {
        return 1;
    }
    return n * factorial(n - 1);
}

uint64_t user_prog(int n) __attribute__((alias("factorial")));
